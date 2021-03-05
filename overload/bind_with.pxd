#distutils: language = c++
#cython: profile=True
#cython: infer_types=True
#cython: boundscheck=False
#cython: wraparound=False
#cython: language_level = 3
from .signature cimport Signature

cdef bind_with(Signature sig, object bind_func, tuple args, dict kwargs)