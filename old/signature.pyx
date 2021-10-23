#distutils: language = c++
#cython: profile=True
#cython: infer_types=True
#cython: boundscheck=False
#cython: wraparound=False
#cython: language_level = 3
from inspect import _empty
from cpython cimport PyObject, Py_INCREF, Py_DECREF
from libcpp.vector cimport vector


signature_cache = {}


cdef Parameter createParameter(object py_param):
	"""Create a Parameter object from a python parameter from the inspect module"""
	cdef Parameter param

	param.name = <PyObject*> py_param.name
	param.kind = py_param.kind
	param.annotation = <PyObject*> py_param.annotation
	param.has_default = py_param.default is not _empty

	Py_INCREF(py_param.name)
	Py_INCREF(py_param.annotation)

	return param


cdef void destroyParameter(Parameter param):
	"""Destroy a Parameter object."""
	Py_DECREF(<object> param.name)
	Py_DECREF(<object> param.annotation)


cdef Signature createSignature(object py_sig):
	cdef Signature sig = Signature()
	sig.parameters.reserve(len(py_sig.parameters))

	for py_param in py_sig.parameters.values():
		sig.parameters.push_back(createParameter(py_param))
	
	return sig


cdef void destroySignature(Signature sig):
	cdef Parameter param
	for param in sig.parameters:
		destroyParameter(param)
