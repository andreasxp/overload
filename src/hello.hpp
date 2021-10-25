#pragma once
#define PY_SSIZE_T_CLEAN
#include <Python.h>
#include <iostream>
#include <vector>
#include "util/ref.hpp"
#include "util/macros.hpp"

namespace {
extern "C" {

ref methodHello(ref, ref _a, ref _kw) {
    PARSEARGS(name);

	uref moduleBuiltin {PyImport_ImportModule("builtins")};
	uref methodPrint {PyObject_GetAttrString(&*moduleBuiltin, "print")};

	uref result {PyObject_CallFunction(&*methodPrint, "sO", "Hello", name)};

    Py_RETURN_NONE;
}

} // extern "C"
} // namespace