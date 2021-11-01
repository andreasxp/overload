#include <iostream>
#include <vector>
#include "base.hpp"
#include "exceptions.hpp"
#include "hello.hpp"
#include "signature.hpp"
#include "bind.hpp"
#include "resolve.hpp"

namespace {
extern "C" {

ref methodResolve(ref, ref _a, ref _kw) {
    PARSEARGS(args, kwargs, functions, module, qualname);
	span<ref> functions_vec(_PyList_CAST(functions)->ob_item, PyList_GET_SIZE(functions));

	// Releasing reference because this is the gateway to external python code
	return resolve(args, kwargs, functions_vec, module, qualname).release();
}

/// Perform overload resoulution and call the selected function.
ref methodCall(ref self, const ref args[], ssize nargs) {
    // PARSEARGS(args, kwargs, functions, module, qualname);
    span<ref> functions(_PyList_CAST(args[2])->ob_item, PyList_GET_SIZE(args[2]));

	uref func = resolve(args[0], args[1], functions, args[3], args[4]);
	if (!func) return nullptr;
	return PyObject_Call(&*func, args[0], args[1]);
}

/**
 * @brief Prepare `function` for overload resolution. If `function` is already prepared, does nothing.
 * At the moment, just creates a `signature` instance in the cache.
 */
ref methodPrepare(ref, ref _a, ref _kw) {
    PARSEARGS(function);

	signatures.emplace(function, signature{function});

    Py_RETURN_NONE;
}

// Module Declaration ==================================================================================================
PyMethodDef defModuleMethods[] = {
    {"hello", (PyCFunction)methodHello, METH_VARARGS | METH_KEYWORDS, "Prints hello"},
    {"resolve", (PyCFunction)methodResolve, METH_VARARGS | METH_KEYWORDS, "Perform overload resolution"},
    {"call", (PyCFunction)methodCall, METH_FASTCALL, "Perform overload resolution"},
    {"prepare", (PyCFunction)methodPrepare, METH_VARARGS | METH_KEYWORDS, "Prepare `function` for overload resolution"},
    {nullptr, nullptr, 0, nullptr}
};

PyModuleDef defModule = {
    PyModuleDef_HEAD_INIT,
    "overload.core",
    "Overloading functions module",
    -1,
    defModuleMethods
};

PyMODINIT_FUNC PyInit_core() {
    // Developer note:
    // To simplify project structure, all functions and exceptions are compiled to a single python module,
    // "overload.core". To make code more manageable, this module is split into so-called "submodules" - hpp files.
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
