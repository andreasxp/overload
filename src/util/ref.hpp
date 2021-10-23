#pragma once
#define PY_SSIZE_T_CLEAN
#include <Python.h>
#include <memory>

using ssize = Py_ssize_t;
using ref = PyObject*;

namespace impl {

struct Deleter {
	void operator()(ref ptr) const noexcept {
		Py_DECREF(ptr);
	}
};

}

using uref = std::unique_ptr<PyObject, impl::Deleter>;
static_assert(sizeof(uref) == sizeof(ref), "Overload C++ code performs reinterpret_cast from uref** to ref**");