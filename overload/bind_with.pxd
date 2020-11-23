#distutils: language = c++
#cython: language_level = 3
from cpython cimport PyObject
from libcpp cimport bool
from libcpp.vector cimport vector

cdef struct Parameter:
	int kind
	PyObject* annotation
	PyObject* name
	bool has_default

cdef class Signature:
	cdef vector[Parameter] parameters

cpdef bind_with(Signature sig, bind_func, args, kwargs)