#pragma once
#include "base.hpp"
#include "bind.hpp"

// perform_overload_resolution =========================================================================================
inline uref perform_overload_resolution(ref args, ref kwargs, std::vector<ref>& functions, ref module, ref qualname) {
	std::vector<ref> candidates;
	std::vector<bind_result> fail_reasons;

	for (ref func : functions) {
		//std::cout << "Binding ";
		//print(func);
		signature& sig = signatures.find(func)->second;
		bind_result result = function_bind(sig, args, kwargs, arg_bind_strict);

		if (result.status == bind_result::success) {
			//std::cout << "  ...success!\n";
			candidates.push_back(func);
		}
		else {
			//std::cout << "  ...failed\n";
			fail_reasons.push_back(std::move(result));
		}
	}

	if (candidates.empty()) {
		uref reasons {PyDict_New()};
		for (size_t i = 0; i < functions.size(); i++) {
			PyDict_SetItem(&*reasons, functions[i], &*fail_reasons[i].message());
		}

		uref NoMatchingOverloadError = import_from("overload.core", "NoMatchingOverloadError");
		uref exc {PyObject_CallFunctionObjArgs(
			&*NoMatchingOverloadError, module, qualname, args, kwargs, &*reasons, nullptr
		)};
		PyErr_SetObject(&*NoMatchingOverloadError, &*exc);
		return nullptr;
		// raise ovl_module.NoMatchingOverloadError(
        //     module, qualname, (args, kwargs), functions, fail_reasons
        // )
	}

	if (candidates.size() > 1) {
		uref AmbiguousOverloadError = import_from("overload.core", "AmbiguousOverloadError");
		uref exc {PyObject_CallFunctionObjArgs(
			&*AmbiguousOverloadError, module, qualname, args, kwargs, &*to_list(candidates), nullptr
		)};
		PyErr_SetObject(&*AmbiguousOverloadError, &*exc);
		return nullptr;
		// raise ovl_module.AmbiguousOverloadError(module, qualname, (args, kwargs), candidates)
	}

	Py_INCREF(candidates[0]);
	return uref{candidates[0]};
}

ref methodPor(ref, ref _a, ref _kw) {
    PARSEARGS(args, kwargs, functions, module, qualname);
	// std::vector<ref> functions_vec = from_list(functions);

	// // Releasing reference because this is the gateway to external python code
	// return perform_overload_resolution(args, kwargs, functions_vec, module, qualname).release();
	Py_RETURN_NONE;
}

/// Perform overload resoulution and call the selected function.
ref methodCall(ref, ref _a, ref _kw) {
    PARSEARGS(args, kwargs, functions, module, qualname);
	std::vector<ref> functions_vec = from_list(functions);

	uref func = perform_overload_resolution(args, kwargs, functions_vec, module, qualname);
	if (!func) return nullptr;
	return PyObject_Call(&*func, args, kwargs);
}