#pragma once
#define PY_SSIZE_T_CLEAN
#include <Python.h>
#include <unordered_map>
#include "util.hpp"

struct parameter {
	long int kind;
	uref name;
	uref annotation;
	bool hasdefault;

	parameter() = delete;
	parameter(ref py_parameter) {
		name = getattr(py_parameter, "name");
		kind = PyLong_AsLong(&*getattr(py_parameter, "kind"));
		annotation = getattr(py_parameter, "annotation");

		uref empty = import_from("inspect", "_empty");
		hasdefault = py_parameter != &*empty;
	}
};

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


struct py_unicode_hash {
	size_t operator()(ref k) {
		return PyObject_Hash(k);
	}
};

struct py_unicode_equal {
	bool operator()(ref lhs, ref rhs) {
		return PyUnicode_RichCompare(lhs, rhs, Py_EQ) == Py_True;
	}
};

std::unordered_map<ref, signature, py_unicode_hash, py_unicode_equal> signatures;