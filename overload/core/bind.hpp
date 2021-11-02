#pragma once
#include <vector>
#include "base.hpp"
#include "signature.hpp"

// per-argument bind functions =========================================================================================
using arg_bind_t = bool(*)(ref value, ref annotation);

inline bool arg_bind_strict(ref value, ref annotation) {
	return PyObject_IsInstance(value, annotation);
}

// function_bind ===========================================================================================================
int POSITIONAL_ONLY = 0;
int VAR_POSITIONAL = 0;
int KEYWORD_ONLY = 0;
int VAR_KEYWORD = 0;

uref object = nullptr;

/**
 * @brief Data structure returned from function_bind
 * When binding was successful, returns bind_result with `status` set to `success`.
 * On errors, `status` is set to one of the error codes and param_name and arg_type may
 * hold relevant information:
 *
 * | Status Code           | param_name | arg_type  |
 * | :-------------------- | :--------: | :-------: |
 * | success               | ❌         | ❌       |
 * | unexpected_type       | ✅         | ✅       |
 * | missing_value         | ✅         | ❌       |
 * | multiple_values       | ✅         | ❌       |
 * | too_many_positional   | ❌         | ❌       |
 * | unexpected_keyword    | ✅         | ❌       |
 * | positional_as_keyword | ✅         | ❌       |
 *
 */
struct bind_result {
	enum status_t {
		success,
		unexpected_type,
		missing_value,
		multiple_values,
		too_many_positional,
		unexpected_keyword,
		positional_as_keyword,
	};
	status_t status = success;

	/**
	 * @brief Parameter name, if applicable.
	 * The signature object holds a reference to param name until module destruction, so inreasing reference count is
	 * not required.
	 */
	ref param_name = nullptr;

	/**
	 * @brief Argument type, for unexpected_type.
	 * bind_result is not intended to live longer than the arguments, so increasing reference count is not required.
	 */
	PyTypeObject* arg_type = nullptr;

	uref message() const {
		switch(status) {
			case success:
				return uref {PyUnicode_FromString("success")};
			case unexpected_type:
				return uref {PyUnicode_FromFormat("argument %R has unexpected type %R", &*param_name, &*arg_type)};
			case missing_value:
				return uref {PyUnicode_FromFormat("missing a required argument: %R", &*param_name)};
			case multiple_values:
				return uref {PyUnicode_FromFormat("multiple values for argument: %R", &*param_name)};
			case too_many_positional:
				return uref {PyUnicode_FromString("too many positional arguments")};
			case unexpected_keyword:
				return uref {PyUnicode_FromFormat("got an unexpected keyword argument %R", &*param_name)};
			case positional_as_keyword:
				return uref {PyUnicode_FromFormat(
					"%R parameter is positional only, but was passed as a keyword", &*param_name
				)};
		}
	}
};

/**
 * @brief Bind a set of arguments to a function signature.
 * This function performs the core of overload resolution, by checking if a function signature can accept the set of
 * args and kwargs passed in. This check is performed by checking that:
 *
 * 1. Passed arguments match the amount of parameters, including positional-only and keyword-only parameters;
 * 2. Arguments have a type that is compatible with the corresponding parameter, as specified in the param. annotation.
 *
 * The type compatibility is checked using the `bind` parameter - a `arg_bind_t` function that has an interface similar
 * to python's `isinstance`.
 *
 * @param sig Function signature
 * @param bind Bind function to perform binding per parameter
 * @param args Positional arguments
 * @param kwargs Keyword arguments
 * @return An instance of bind_result, possibly containing information about an error.
 */
inline bind_result function_bind(const signature& sig, ref args, ref kwargs, arg_bind_t bind) {
	//std::cout << "Enter\n";
	ssize i_args = 0;
	ssize len_args = PyTuple_GET_SIZE(args);
	int i_params = 0;

	while (true) {
		// Let's iterate through the positional arguments and corresponding
		// parameters
		if (i_args < len_args) {
			ref arg_val = PyTuple_GET_ITEM(args, i_args);
			i_args++;

			// We have a positional argument to process
			if (i_params < sig.parameters.size()) {
				const parameter& param = sig.parameters[i_params];
				i_params++;

				ref annotation = &*param.annotation;
				if (annotation == &*empty) annotation = &*object;

				if (param.kind == VAR_KEYWORD or param.kind == KEYWORD_ONLY) {
					// Looks like we have no parameter for this positional
					// argument
					//std::cout << "Leave 1\n";
					return {bind_result::too_many_positional};
					// TypeError('too many positional arguments')
				}

				if (param.kind == VAR_POSITIONAL) break;

				if (PyDict_Contains(kwargs, &*param.name) and param.kind != POSITIONAL_ONLY) {
					//std::cout << "Leave 2\n";
					return {bind_result::multiple_values, &*param.name};
					// TypeError(f'multiple values for argument {<object> param.name!r}')
				}

				if (not bind(arg_val, annotation)) {
					//std::cout << "Leave 3\n";
					return {bind_result::unexpected_type, &*param.name, arg_val->ob_type};
					// TypeError(f"argument '{<object> param.name!r}' has unexpected type '{type(arg_val).__qualname__}'")
				}
			}
			else {
				//std::cout << "Leave 4\n";
				return {bind_result::too_many_positional};
				// TypeError('too many positional arguments')
			}
		}
		else {
			// No more positional arguments
			if (i_params < sig.parameters.size()) {
				const parameter& param = sig.parameters[i_params];
				i_params++;

				ref annotation = &*param.annotation;
				if (annotation == &*empty) annotation = &*object;

				if (param.kind == VAR_POSITIONAL) {
					// That's OK, just empty *args.  Let's start parsing
					// kwargs
					break;
				}
				else if (PyDict_Contains(kwargs, &*param.name)) {
					if (param.kind == POSITIONAL_ONLY) {
						//std::cout << "Leave 5\n";
						return {bind_result::positional_as_keyword, &*param.name};
						// TypeError(f'{<object> param.name!r} parameter is positional only, but was passed as a keyword')
					}
					i_params--;
					break;
				}
				else if (param.kind == VAR_KEYWORD or param.has_default) {
					// That's fine too - we have a default value for this
					// parameter.  So, lets start parsing `kwargs`, starting
					// with the current parameter
					i_params--;
					break;
				}
				else {
					// No default, not VAR_KEYWORD, not VAR_POSITIONAL,
					// not in `kwargs`
					//std::cout << "Leave 6\n";
					return {bind_result::missing_value, &*param.name};
					// TypeError(f'missing a required argument: {<object> param.name!r}')
				}
			}
			else break;
		}
	}

	// Create a set of kwarg keys --------------------------------------------------------------------------------------
	ref key = nullptr;
	ref value = nullptr;
	ssize pos = 0;

	flat_hash_set<ref, py_unicode_hash, py_unicode_equal> kwarg_keys;

	while (PyDict_Next(kwargs, &pos, &key, &value)) {
		kwarg_keys.insert(key);
	}
	// -----------------------------------------------------------------------------------------------------------------

	bool have_kwargs_param = false;
	while (i_params < sig.parameters.size()) {
		const parameter& param = sig.parameters[i_params];
		i_params++;

		if (param.kind == VAR_KEYWORD) {
			have_kwargs_param = true;
			continue;
		}

		if (param.kind == VAR_POSITIONAL) {
			// Named arguments don't refer to '*args'-like parameters.
			// We only arrive here if the positional arguments ended
			// before reaching the last parameter before *args.
			continue;
		}

		bool haveArgVal = kwarg_keys.erase(&*param.name);
		//print(&*param.name);
		//std::cout << "here: " << haveArgVal << "\n";
		if (not haveArgVal) {
			// We have no value for this parameter.  It's fine though,
			// if it has a default value, or it is an '*args'-like
			// parameter, left alone by the processing of positional
			// arguments.
			//std::cout << "Data: " << (param.kind == VAR_POSITIONAL) << " and " << (param.has_default) << "\n";
			if (param.kind != VAR_POSITIONAL and not param.has_default) {
				//std::cout << "Leave 7\n";
				return {bind_result::missing_value, &*param.name};
				// TypeError(f'missing a required argument: {<object> param.name!r}')
			}
		}
		else {
			if (param.kind == POSITIONAL_ONLY) {
				// This should never happen in case of a properly built
				// Signature object (but let's have this check here
				// to ensure correct behaviour just in case)
				//std::cout << "Leave 8\n";
				return {bind_result::positional_as_keyword, &*param.name};
				// TypeError(f'{<object> param.name!r} parameter is positional only, but was passed as a keyword')
			}

			ref arg_val = PyDict_GetItem(kwargs, &*param.name);

			ref annotation = &*param.annotation;
			if (annotation == &*empty) annotation = &*object;

			if (not bind(arg_val, annotation)) {
				//std::cout << "Leave 9\n";
				return {bind_result::unexpected_type, &*param.name, arg_val->ob_type};
				// TypeError(f"argument '{<object> param.name!r}' has unexpected type '{type(arg_val).__qualname__}'")
			}
		}
	}

	if (not kwarg_keys.empty() and not have_kwargs_param) {
		//std::cout << "Leave 10\n";
		return {bind_result::unexpected_keyword, *kwarg_keys.begin()};
		// TypeError(f'got an unexpected keyword argument {next(iter(kwargs_))!r}')
	}

	//std::cout << "Leave 11\n";
	return {bind_result::success};
}

AT_SUBMODULE_INIT(ref module) {
	// Pre-load inspect module
	uref moduleInspect = import("inspect");
	object = std::move(import_from("builtins", "object"));

	POSITIONAL_ONLY = PyLong_AsLong(&*getattr(moduleInspect, "_POSITIONAL_ONLY"));
	VAR_POSITIONAL = PyLong_AsLong(&*getattr(moduleInspect, "_VAR_POSITIONAL"));
	KEYWORD_ONLY = PyLong_AsLong(&*getattr(moduleInspect, "_KEYWORD_ONLY"));
	VAR_KEYWORD = PyLong_AsLong(&*getattr(moduleInspect, "_VAR_KEYWORD"));
};