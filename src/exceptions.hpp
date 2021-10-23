#pragma once
#define PY_SSIZE_T_CLEAN
#include <Python.h>
#include <iostream>
#include <vector>
#include "util/ref.hpp"
#include "util/macros.hpp"

namespace {

uref nameFromQualname(ref qualname) {
    ssize qualname_size = PyUnicode_GET_LENGTH(qualname);
    ssize last_dot = PyUnicode_FindChar(qualname, '.', 0, qualname_size, -1);

    ref name = nullptr;
    if (last_dot == -1) {
        Py_INCREF(qualname);
        return uref {qualname};
    }
    return uref {PyUnicode_Substring(qualname, last_dot, qualname_size)};
}

extern "C" {

uref moduleInspect;
uref methodInspectSignature;

// Exceptions ==========================================================================================================
// OverloadError -------------------------------------------------------------------------------------------------------
uref OverloadError;

void implMethodOverloadErrorInit(ref self, ref module, ref qualname, ref args, ref kwargs) {
    PyObject_SetAttrString(self, "module", module);
    PyObject_SetAttrString(self, "qualname", qualname);
    PyObject_SetAttrString(self, "args", args);
    PyObject_SetAttrString(self, "kwargs", kwargs);
}

ref methodOverloadErrorInit(ref, ref _a, ref _kw) {
    PARSEARGS(self, module, qualname, args, kwargs);

    implMethodOverloadErrorInit(&*self, &*module, &*qualname, &*args, &*kwargs);
    
    Py_RETURN_NONE;
}

ref methodOverloadErrorStr(ref, ref _a, ref _kw) {
    PARSEARGS(self);

    uref module {PyObject_GetAttrString(&*self, "module")};
    uref qualname {PyObject_GetAttrString(&*self, "qualname")};

    return PyUnicode_FromFormat("overload error during call to %U.%U", &*module, &*qualname);
}

PyMethodDef defOverloadErrorMethods[] = {
    {"__init__", (PyCFunction)methodOverloadErrorInit, METH_VARARGS | METH_KEYWORDS, "TODO"},
    {"__str__", (PyCFunction)methodOverloadErrorStr, METH_VARARGS | METH_KEYWORDS, "TODO"},
    {nullptr, nullptr, 0, nullptr}
};

// AmbiguousOverloadError ----------------------------------------------------------------------------------------------
uref AmbiguousOverloadError;

ref methodAmbiguousOverloadErrorInit(ref, ref _a, ref _kw) {
    PARSEARGS(self, module, qualname, args, kwargs, candidates);

    implMethodOverloadErrorInit(&*self, &*module, &*qualname, &*args, &*kwargs);
    PyObject_SetAttrString(&*self, "candidates", &*candidates);

    Py_RETURN_NONE;
}

ref methodAmbiguousOverloadErrorStr(ref, ref _a, ref _kw) {
    PARSEARGS(self);

    uref module {PyObject_GetAttrString(&*self, "module")};
    uref qualname {PyObject_GetAttrString(&*self, "qualname")};
    uref candidates {PyObject_GetAttrString(&*self, "candidates")};
    uref name {nameFromQualname(&*qualname)};
    
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
    uref candidates_str {_PyUnicode_JoinArray(&*newline, (ref*)candidates_str_vec.data(), candidates_str_vec.size())};

    uref title_str {PyUnicode_FromFormat(
        "ambiguous overloaded call to %U.%U\nPossible candidates:\n", &*module, &*qualname)};
    
    return PyUnicode_Concat(&*title_str, &*candidates_str);
}

PyMethodDef defAmbiguousOverloadErrorMethods[] = {
    {"__init__", (PyCFunction)methodAmbiguousOverloadErrorInit, METH_VARARGS | METH_KEYWORDS, "TODO"},
    {"__str__", (PyCFunction)methodAmbiguousOverloadErrorStr, METH_VARARGS | METH_KEYWORDS, "TODO"},
    {nullptr, nullptr, 0, nullptr}
};

// NoMatchingOverloadError ----------------------------------------------------------------------------------------------
uref NoMatchingOverloadError;

ref methodNoMatchingOverloadErrorInit(ref, ref _a, ref _kw) {
    PARSEARGS(self, module, qualname, args, kwargs, candidates, fail_reasons);

    implMethodOverloadErrorInit(&*self, &*module, &*qualname, &*args, &*kwargs);
    PyObject_SetAttrString(&*self, "candidates", &*candidates);
    PyObject_SetAttrString(&*self, "fail_reasons", &*fail_reasons);

    Py_RETURN_NONE;
}

ref methodNoMatchingOverloadErrorStr(ref, ref _a, ref _kw) {
    PARSEARGS(self);

    uref module {PyObject_GetAttrString(&*self, "module")};
    uref qualname {PyObject_GetAttrString(&*self, "qualname")};
    uref candidates {PyObject_GetAttrString(&*self, "candidates")};
    uref fail_reasons {PyObject_GetAttrString(&*self, "fail_reasons")};
    uref name = nameFromQualname(&*qualname);
    
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
    uref reasons_str {_PyUnicode_JoinArray(&*newline, (ref*)reasons_str_vec.data(), reasons_str_vec.size())};

    uref title_str {PyUnicode_FromFormat(
        "no matching overload found for %U.%U\nReason:\n", &*module, &*qualname)};
    
    return PyUnicode_Concat(&*title_str, &*reasons_str);
}

PyMethodDef defNoMatchingOverloadErrorMethods[] = {
    {"__init__", (PyCFunction)methodNoMatchingOverloadErrorInit, METH_VARARGS | METH_KEYWORDS, "TODO"},
    {"__str__", (PyCFunction)methodNoMatchingOverloadErrorStr, METH_VARARGS | METH_KEYWORDS, "TODO"},
    {nullptr, nullptr, 0, nullptr}
};

} // extern "C"
} // namespace
