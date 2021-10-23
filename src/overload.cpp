#define PY_SSIZE_T_CLEAN
#include <Python.h>
#include <iostream>
#include <vector>
#include "exceptions.hpp"
#include "hello.hpp"
#include "util/ref.hpp"
#include "util/macros.hpp"

namespace {
extern "C" {

// Module Declaration ==================================================================================================
PyMethodDef defModuleMethods[] = {
    {"hello", (PyCFunction)methodHello, METH_VARARGS | METH_KEYWORDS, "Prints hello"},
    {nullptr, nullptr, 0, nullptr}
};

PyModuleDef defModule = {PyModuleDef_HEAD_INIT, "overload", "Overloading functions module", -1, defModuleMethods};

PyMODINIT_FUNC PyInit_overload() {
    uref module {PyModule_Create(&defModule)};
    if (!module) return nullptr;

    uref moduleInspect {PyImport_ImportModule("inspect")};
    uref methodInspectSignature {PyObject_GetAttrString(&*moduleInspect, "signature")};

    // Exceptions ======================================================================================================
    // OverloadError ---------------------------------------------------------------------------------------------------
    static const char* docOverloadError =
        "An exception that is raised when there was an error during overload resolution.\n"
        "This exception is not raised - it serves as a base class for AmbiguousOverloadError and"
        "NoMatchingOverloadError.\n";
    uref dictOverloadError {PyDict_New()};
    uref OverloadError {PyErr_NewExceptionWithDoc(
        "overload.OverloadError", docOverloadError, PyExc_TypeError, &*dictOverloadError
    )};

    for (PyMethodDef* iDefMethod = defOverloadErrorMethods; iDefMethod->ml_name != nullptr; iDefMethod++) {
        uref func {PyCFunction_New(iDefMethod, nullptr)};
        uref method {PyMethod_New(&*func, &*OverloadError)};
        PyObject_SetAttrString(&*OverloadError, iDefMethod->ml_name, &*method);
    }

    // AmbiguousOverloadError ------------------------------------------------------------------------------------------
    static const char* docAmbiguousOverloadError =
        "An exception that is raised when arguments passed to a function match more that one overload.\n";
    uref dictAmbiguousOverloadError {PyDict_New()};

    uref AmbiguousOverloadError {PyErr_NewExceptionWithDoc(
        "overload.AmbiguousOverloadError", docAmbiguousOverloadError, &*OverloadError, &*dictAmbiguousOverloadError
    )};

    for (PyMethodDef* iDefMethod = defAmbiguousOverloadErrorMethods; iDefMethod->ml_name != nullptr; iDefMethod++) {
        uref func {PyCFunction_New(iDefMethod, nullptr)};
        uref method {PyMethod_New(&*func, &*AmbiguousOverloadError)};
        PyObject_SetAttrString(&*AmbiguousOverloadError, iDefMethod->ml_name, &*method);
    }

    // NoMatchingOverloadError ------------------------------------------------------------------------------------------
    static const char* docNoMatchingOverloadError =
        "An exception that is raised when arguments passed to a function match none of the overloads.\n";
    uref dictNoMatchingOverloadError {PyDict_New()};

    uref NoMatchingOverloadError {PyErr_NewExceptionWithDoc(
        "overload.NoMatchingOverloadError", docNoMatchingOverloadError, &*OverloadError, &*dictNoMatchingOverloadError
    )};

    for (PyMethodDef* iDefMethod = defNoMatchingOverloadErrorMethods; iDefMethod->ml_name != nullptr; iDefMethod++) {
        uref func {PyCFunction_New(iDefMethod, nullptr)};
        uref method {PyMethod_New(&*func, &*NoMatchingOverloadError)};
        PyObject_SetAttrString(&*NoMatchingOverloadError, iDefMethod->ml_name, &*method);
    }

    PyModule_AddObject(&*module, "OverloadError", OverloadError.release());
    PyModule_AddObject(&*module, "AmbiguousOverloadError", AmbiguousOverloadError.release());
    PyModule_AddObject(&*module, "NoMatchingOverloadError", NoMatchingOverloadError.release());

    return module.release();
}


} // extern "C"
} // namespace
