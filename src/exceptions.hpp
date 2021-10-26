#pragma once
#define PY_SSIZE_T_CLEAN
#include <Python.h>
#include <iostream>
#include <vector>
#include "util.hpp"

namespace {

uref nameFromQualname(ref qualname) {
	ssize qualname_size = PyUnicode_GET_LENGTH(qualname);
	ssize last_dot = PyUnicode_FindChar(qualname, '.', 0, qualname_size, -1);

	if (last_dot == -1) {
		Py_INCREF(qualname);
		return uref {qualname};
	}
	return uref {PyUnicode_Substring(qualname, last_dot, qualname_size)};
}

void implMethodOverloadErrorInit(ref self, ref module, ref qualname, ref args, ref kwargs) {
	setattr(self, "module", module);
	setattr(self, "qualname", qualname);
	setattr(self, "args", args);
	setattr(self, "kwargs", kwargs);
}

extern "C" {

// Exceptions ==========================================================================================================
// OverloadError -------------------------------------------------------------------------------------------------------
ref methodOverloadErrorInit(ref, ref _a, ref _kw) {
	PARSEARGS(self, module, qualname, args, kwargs);

	implMethodOverloadErrorInit(self, module, qualname, args, kwargs);

	Py_RETURN_NONE;
}

ref methodOverloadErrorStr(ref, ref _a, ref _kw) {
	PARSEARGS(self);

	uref module = getattr(self, "module");
	uref qualname = getattr(self, "qualname");

	return PyUnicode_FromFormat("overload error during call to %U.%U", &*module, &*qualname);
}

PyMethodDef defOverloadErrorMethods[] = {
	{"__init__", (PyCFunction)methodOverloadErrorInit, METH_VARARGS | METH_KEYWORDS, "TODO"},
	{"__str__", (PyCFunction)methodOverloadErrorStr, METH_VARARGS | METH_KEYWORDS, "TODO"},
	{nullptr, nullptr, 0, nullptr}
};

// AmbiguousOverloadError ----------------------------------------------------------------------------------------------
ref methodAmbiguousOverloadErrorInit(ref, ref _a, ref _kw) {
	PARSEARGS(self, module, qualname, args, kwargs, candidates);

	implMethodOverloadErrorInit(self, module, qualname, args, kwargs);
	setattr(self, "candidates", candidates);

	Py_RETURN_NONE;
}

ref methodAmbiguousOverloadErrorStr(ref, ref _a, ref _kw) {
	PARSEARGS(self);

	uref module = getattr(self, "module");
	uref qualname = getattr(self, "qualname");
	uref candidates = getattr(self, "candidates");
	uref name {nameFromQualname(&*qualname)};

	uref methodInspectSignature = import_from("inspect", "signature");

	ssize candidates_size = PyList_Size(&*candidates);
	std::vector<uref> candidates_str_vec;
	candidates_str_vec.reserve(candidates_size);
	for (ssize i = 0; i < candidates_size; i++) {
		ref candidate = PyList_GetItem(&*candidates, i);
		uref candidate_sig_args {PyTuple_Pack(1, candidate)};
		uref candidate_sig {PyObject_Call(&*methodInspectSignature, &*candidate_sig_args, nullptr)};

		candidates_str_vec.emplace_back(PyUnicode_FromFormat("  %U%S", &*name, &*candidate_sig));
	}

	uref newline {PyUnicode_FromString("\n")};

	static_assert(sizeof(ref) == sizeof(uref), "safe reinterpret_cast of array data impossible");
	uref candidates_str {_PyUnicode_JoinArray(
		&*newline,
		reinterpret_cast<ref*>(candidates_str_vec.data()), candidates_str_vec.size()
	)};

	uref title_str {PyUnicode_FromFormat(
		"ambiguous overloaded call to %U.%U\nPossible candidates:\n", &*module, &*qualname
	)};

	return PyUnicode_Concat(&*title_str, &*candidates_str);
}

PyMethodDef defAmbiguousOverloadErrorMethods[] = {
	{"__init__", (PyCFunction)methodAmbiguousOverloadErrorInit, METH_VARARGS | METH_KEYWORDS, "TODO"},
	{"__str__", (PyCFunction)methodAmbiguousOverloadErrorStr, METH_VARARGS | METH_KEYWORDS, "TODO"},
	{nullptr, nullptr, 0, nullptr}
};

// NoMatchingOverloadError ---------------------------------------------------------------------------------------------
ref methodNoMatchingOverloadErrorInit(ref, ref _a, ref _kw) {
	PARSEARGS(self, module, qualname, args, kwargs, candidates, fail_reasons);

	implMethodOverloadErrorInit(self, module, qualname, args, kwargs);
	setattr(self, "candidates", candidates);
	setattr(self, "fail_reasons", fail_reasons);

	Py_RETURN_NONE;
}

ref methodNoMatchingOverloadErrorStr(ref, ref _a, ref _kw) {
	PARSEARGS(self);

	uref module = getattr(self, "module");
	uref qualname = getattr(self, "qualname");
	uref candidates = getattr(self, "candidates");
	uref fail_reasons = getattr(self, "fail_reasons");
	uref name = nameFromQualname(&*qualname);

	uref methodInspectSignature = import_from("inspect", "signature");

	ssize size = PyList_Size(&*candidates);
	std::vector<uref> reasons_str_vec;
	reasons_str_vec.reserve(size);
	for (ssize i = 0; i < size; i++) {
		ref candidate = PyList_GetItem(&*candidates, i);
		ref reason = PyList_GetItem(&*fail_reasons, i);

		uref candidate_sig_args {PyTuple_Pack(1, candidate)};
		uref candidate_sig {PyObject_Call(&*methodInspectSignature, &*candidate_sig_args, nullptr)};

		reasons_str_vec.emplace_back(PyUnicode_FromFormat("  %U%S: %S", &*name, &*candidate_sig, reason));
	}

	uref newline {PyUnicode_FromString("\n")};

	static_assert(sizeof(ref) == sizeof(uref), "safe reinterpret_cast of array data impossible");
	uref reasons_str {_PyUnicode_JoinArray(&*newline,
		reinterpret_cast<ref*>(reasons_str_vec.data()), reasons_str_vec.size()
	)};

	uref title_str {PyUnicode_FromFormat(
		"no matching overload found for %U.%U\nReason:\n", &*module, &*qualname
	)};

	return PyUnicode_Concat(&*title_str, &*reasons_str);
}

PyMethodDef defNoMatchingOverloadErrorMethods[] = {
	{"__init__", (PyCFunction)methodNoMatchingOverloadErrorInit, METH_VARARGS | METH_KEYWORDS, "TODO"},
	{"__str__", (PyCFunction)methodNoMatchingOverloadErrorStr, METH_VARARGS | METH_KEYWORDS, "TODO"},
	{nullptr, nullptr, 0, nullptr}
};

} // extern "C"

AT_SUBMODULE_INIT(ref module) {
	// Pre-load inspect module
	uref moduleInspect = import("inspect");

	// OverloadError ---------------------------------------------------------------------------------------------------
	static const char* docOverloadError =
		"An exception that is raised when there was an error during overload resolution.\n"
		"This exception is not raised - it serves as a base class for AmbiguousOverloadError and"
		"NoMatchingOverloadError.\n";
	uref OverloadError {PyErr_NewExceptionWithDoc(
		"overload.OverloadError", docOverloadError, PyExc_TypeError, nullptr
	)};

	for (PyMethodDef* iDefMethod = defOverloadErrorMethods; iDefMethod->ml_name != nullptr; iDefMethod++) {
		uref func {PyCFunction_New(iDefMethod, nullptr)};
		uref method {PyMethod_New(&*func, &*OverloadError)};
		PyObject_SetAttrString(&*OverloadError, iDefMethod->ml_name, &*method);
	}

	PyModule_AddObjectRef(module, "OverloadError", &*OverloadError);

	// AmbiguousOverloadError ------------------------------------------------------------------------------------------
	static const char* docAmbiguousOverloadError =
		"An exception that is raised when arguments passed to a function match more that one overload.\n";

	uref AmbiguousOverloadError {PyErr_NewExceptionWithDoc(
		"overload.AmbiguousOverloadError", docAmbiguousOverloadError, &*OverloadError, nullptr
	)};

	for (PyMethodDef* iDefMethod = defAmbiguousOverloadErrorMethods; iDefMethod->ml_name != nullptr; iDefMethod++) {
		uref func {PyCFunction_New(iDefMethod, nullptr)};
		uref method {PyMethod_New(&*func, &*AmbiguousOverloadError)};
		PyObject_SetAttrString(&*AmbiguousOverloadError, iDefMethod->ml_name, &*method);
	}

	PyModule_AddObjectRef(module, "AmbiguousOverloadError", &*AmbiguousOverloadError);

	// NoMatchingOverloadError -----------------------------------------------------------------------------------------
	static const char* docNoMatchingOverloadError =
		"An exception that is raised when arguments passed to a function match none of the overloads.\n";

	uref NoMatchingOverloadError {PyErr_NewExceptionWithDoc(
		"overload.NoMatchingOverloadError", docNoMatchingOverloadError, &*OverloadError, nullptr
	)};

	for (PyMethodDef* iDefMethod = defNoMatchingOverloadErrorMethods; iDefMethod->ml_name != nullptr; iDefMethod++) {
		uref func {PyCFunction_New(iDefMethod, nullptr)};
		uref method {PyMethod_New(&*func, &*NoMatchingOverloadError)};
		PyObject_SetAttrString(&*NoMatchingOverloadError, iDefMethod->ml_name, &*method);
	}

	PyModule_AddObjectRef(module, "NoMatchingOverloadError", &*NoMatchingOverloadError);
};

} // namespace
