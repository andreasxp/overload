#pragma once
#include "base.hpp"
#include "bind.hpp"

// resolve =========================================================================================
inline uref resolve(ref args, ref kwargs, span<ref> functions, ref module, ref qualname) {
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