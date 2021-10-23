#pragma once
#define PY_SSIZE_T_CLEAN
#include <Python.h>
#include <memory>

using ssize = Py_ssize_t;
using ref = PyObject*;

namespace impl {

/**
 * @brief Custom deleter for PyObject, for use in uref.
 * @see https://docs.python.org/3/c-api/refcounting.html#c.Py_DECREF
 */
struct Deleter {
	void operator()(ref ptr) const noexcept {
		Py_DECREF(ptr);
	}
};

} // namespace impl

/**
 * @brief A unique pointer to a PyObject, which is cleaned up automatically.
 */
using uref = std::unique_ptr<PyObject, impl::Deleter>;
static_assert(sizeof(uref) == sizeof(ref), "overload: C++ compiler failed to optimize the unique pointer");