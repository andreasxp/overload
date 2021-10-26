#pragma once
#define PY_SSIZE_T_CLEAN
#include <Python.h>
#include "ref.hpp"

uref getattr(ref object, ref attrname) {
	return uref{PyObject_GetAttr(object, attrname)};
}

uref getattr(const uref& object, ref attrname) {
	return uref{PyObject_GetAttr(object.get(), attrname)};
}

uref getattr(ref object, const char* attrname) {
	return uref{PyObject_GetAttrString(object, attrname)};
}

uref getattr(const uref& object, const char* attrname) {
	return uref{PyObject_GetAttrString(object.get(), attrname)};
}

uref import(const char* modulename) {
	return uref{PyImport_ImportModule(modulename)};
}

uref import_from(const char* modulename, const char* name) {
	uref mod = import(modulename);
	return getattr(mod, name);
}