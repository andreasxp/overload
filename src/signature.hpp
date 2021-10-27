#pragma once
#include <unordered_map>
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

		uref empty = import_from("inspect", "_empty");
		has_default = py_parameter != &*empty;
	}
};

/// A simplifier signature object, created from inspect.Signature.
struct signature {
	std::vector<parameter> parameters;

	signature(ref py_signature) {
		uref py_parameters = getattr(py_signature, "parameters");
		parameters.reserve(PyDict_Size(&*py_parameters));

		ref key = nullptr;
		ref value = nullptr;
		ssize pos = 0;

		while (PyDict_Next(&*py_parameters, &pos, &key, &value)) {
			parameters.emplace_back(value);
		}
	}
};

std::unordered_map<ref, signature, py_unicode_hash, py_unicode_equal> signatures;

AT_SUBMODULE_INIT(ref module) {
	// Pre-load inspect module
	import("inspect");
};