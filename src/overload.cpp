#define PY_SSIZE_T_CLEAN
#include <Python.h>
#include <iostream>
#include <vector>
#include "exceptions.hpp"
#include "hello.hpp"
#include "signature.hpp"
#include "util.hpp"

namespace {
extern "C" {

// Module Declaration ==================================================================================================
PyMethodDef defModuleMethods[] = {
    {"hello", (PyCFunction)methodHello, METH_VARARGS | METH_KEYWORDS, "Prints hello"},
    {nullptr, nullptr, 0, nullptr}
};

PyModuleDef defModule = {PyModuleDef_HEAD_INIT, "overload", "Overloading functions module", -1, defModuleMethods};

PyMODINIT_FUNC PyInit_overload() {
    // Developer note:
    // To simplify project structure, all functions and exceptions are compiled to a single python module, "overload".
    // To make code more manageable, this module is split into so-called "submodules" - hpp files.
    // Each hpp submodule is wrapped in an unnamed namespace, and contains a statement like this:
    // AT_SUBMODULE_INIT(ref module) { ... };
    // The code in AT_SUBMODULE_INIT is similar in purpose to top-level code in python files. It performs initialization
    // for the functions and other things contained in that (sub)module.
    // All this code is executed here by calling submodules::init(&*module);

    uref module {PyModule_Create(&defModule)};
    if (!module) return nullptr;

    submodules::init(&*module);
    return module.release();
}


} // extern "C"
} // namespace
