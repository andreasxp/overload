#distutils: language = c++
#cython: profile=True
#cython: infer_types=True
#cython: boundscheck=False
#cython: wraparound=False
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

cdef bind_with(Signature sig, object bind_func, tuple args, dict kwargs)