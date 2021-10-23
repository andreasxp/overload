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


cdef Parameter createParameter(object py_param)
cdef void destroyParameter(Parameter param)

cdef Signature createSignature(object py_sig)
cdef void destroySignature(Signature sig)