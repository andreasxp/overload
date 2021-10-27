#pragma once
#include <iostream>
#include <vector>
#include "base.hpp"

namespace {
extern "C" {

ref methodHello(ref, ref _a, ref _kw) {
    PARSEARGS(name);

	uref methodPrint = import_from("builtins", "print");
	uref result {PyObject_CallFunction(&*methodPrint, "sO", "Hello", name)};

    Py_RETURN_NONE;
}

} // extern "C"
} // namespace