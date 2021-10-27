#pragma once
#include <unordered_set>
#include "base.hpp"
#include "signature.hpp"

int POSITIONAL_ONLY = 0;
int VAR_POSITIONAL = 0;
int KEYWORD_ONLY = 0;
int VAR_KEYWORD = 0;

using bind_function = bool(*)(ref value, ref annotation);

struct bind_result {
	enum status_t {
		success,
		unexpected_type,
		missing_value,
		multiple_values,
		too_many_positional,
		too_many_keyword,
		positional_as_keyword,
	};
	status_t status = success;

	uref param_name = nullptr;
	uref arg_type = nullptr;

	bind_result(status_t status, ref param_name = nullptr, ref arg_type = nullptr) :
		status(status),	param_name(param_name), arg_type(arg_type) {
		Py_XINCREF(param_name);
		Py_XINCREF(arg_type);
	}
};

bind_result bind_with(const signature& sig, bind_function bind, ref args, ref kwargs) {
	uref empty = import_from("inspect", "_empty");
	uref object = import_from("builtins", "object");

	ssize i_args = 0;
	ssize len_args = PyTuple_Size(args);
	int i_params = 0;

	while (true) {
		// Let's iterate through the positional arguments and corresponding
		// parameters
		if (i_args < len_args) {
			ref arg_val = PyTuple_GetItem(args, i_args);
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
					return bind_result{bind_result::too_many_positional};
					// TypeError('too many positional arguments')
				}

				if (param.kind == VAR_POSITIONAL) break;

				if (PyDict_Contains(kwargs, &*param.name) and param.kind != POSITIONAL_ONLY) {
					return bind_result{bind_result::multiple_values, &*param.name};
					// TypeError(f'multiple values for argument {<object> param.name!r}')
				}

				if (not bind(arg_val, annotation)) {
					uref type_arg_val {PyObject_Type(&*arg_val)};
					uref type_qualname = getattr(type_arg_val, "__qualname__");
					return bind_result{bind_result::unexpected_type, &*param.name, &*type_qualname};
					// TypeError(f"argument '{<object> param.name!r}' has unexpected type '{type(arg_val).__qualname__}'")
				}
			}
			else {
				return bind_result{bind_result::too_many_positional};
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
						return bind_result{bind_result::positional_as_keyword, &*param.name};
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
					return bind_result{bind_result::missing_value, &*param.name};
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

	std::unordered_set<ref, py_unicode_hash, py_unicode_equal> kwarg_keys;

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
		if (not haveArgVal) {
			// We have no value for this parameter.  It's fine though,
			// if it has a default value, or it is an '*args'-like
			// parameter, left alone by the processing of positional
			// arguments.
			if (param.kind != VAR_POSITIONAL and not param.has_default) {
				return bind_result{bind_result::missing_value, &*param.name};
				// TypeError(f'missing a required argument: {<object> param.name!r}')
			}
		}
		else {
			if (param.kind == POSITIONAL_ONLY) {
				// This should never happen in case of a properly built
				// Signature object (but let's have this check here
				// to ensure correct behaviour just in case)
				return bind_result{bind_result::positional_as_keyword, &*param.name};
				// TypeError(f'{<object> param.name!r} parameter is positional only, but was passed as a keyword')
			}

			ref arg_val = PyDict_GetItem(kwargs, &*param.name);

			ref annotation = &*param.annotation;
			if (annotation == &*empty) annotation = &*object;

			if (not bind(arg_val, annotation)) {
				uref type_arg_val {PyObject_Type(&*arg_val)};
				uref type_qualname = getattr(type_arg_val, "__qualname__");
				return bind_result{bind_result::unexpected_type, &*param.name, &*type_qualname};
				// TypeError(f"argument '{<object> param.name!r}' has unexpected type '{type(arg_val).__qualname__}'")
			}
		}
	}

	if (not kwarg_keys.empty() and not have_kwargs_param) {
		return bind_result{bind_result::too_many_keyword, *kwarg_keys.begin()};
		// TypeError(f'got an unexpected keyword argument {next(iter(kwargs_))!r}')
	}

	return bind_result{bind_result::success};
}

AT_SUBMODULE_INIT(ref module) {
	// Pre-load inspect module
	uref moduleInspect = import("inspect");
	POSITIONAL_ONLY = PyLong_AsLong(&*getattr(moduleInspect, "_POSITIONAL_ONLY"));
	VAR_POSITIONAL = PyLong_AsLong(&*getattr(moduleInspect, "_VAR_POSITIONAL"));
	KEYWORD_ONLY = PyLong_AsLong(&*getattr(moduleInspect, "_KEYWORD_ONLY"));
	VAR_KEYWORD = PyLong_AsLong(&*getattr(moduleInspect, "_VAR_KEYWORD"));
};