#pragma once
#include "flat_hash_map.hpp"
#include "base.hpp"

/// A simplifier parameter object, created from inspect.Parameter.
struct parameter {
	long int kind;
	uref name;
	uref annotation;
	bool has_default;

	parameter() = delete;
	parameter(ref py_parameter) {
		name = getattr(py_parameter, "name");
		kind = PyLong_AsLong(&*getattr(py_parameter, "kind"));
		annotation = getattr(py_parameter, "annotation");

		has_default = getattr(py_parameter, "default") != import_from("inspect", "_empty");
	}
};

/// A simplifier signature object, created from inspect.Signature.
struct signature {
	std::vector<parameter> parameters;

	signature(ref function) {
		uref methodInspectSignature = import_from("inspect", "signature");
		uref py_signature {PyObject_CallFunctionObjArgs(&*methodInspectSignature, function, nullptr)};

		uref py_parameters_mapping = getattr(py_signature, "parameters");
		uref py_parameters {PyMapping_Values(&*py_parameters_mapping)};
		ssize n = PyList_Size(&*py_parameters);

		parameters.reserve(n);
		for (ssize i = 0; i < n; i++) {
			parameters.emplace_back(PyList_GetItem(&*py_parameters, i));
		}
	}
};

ska::flat_hash_map<ref, signature> signatures;

AT_SUBMODULE_INIT(ref module) {
	// Pre-load inspect module
	import("inspect");
};