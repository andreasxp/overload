#distutils: language = c++
#cython: profile=True
#cython: infer_types=True
#cython: boundscheck=False
#cython: wraparound=False
#cython: language_level = 3
from inspect import signature
from itertools import count
from cpython cimport PyObject
from libcpp.vector cimport vector
import overload as ovl_module
from .bind_with cimport bind_with
from .signature import signature_cache
from .signature cimport createSignature

_registry = {}
"""A registry of all overloaded functions.  
This is how decorated functions communicate with each other to merge several functions into one.
Not for manual editing.
"""

cdef perform_overload_resolution(tuple args, dict kwargs, list functions, str module, str qualname):
    cdef list candidates = []
    cdef list fail_reasons = []

    for func in functions:
        error = bind_with(signature_cache[func], isinstance, args, kwargs)
        if error is not None:
            fail_reasons.append(error)
        else:
            candidates.append(func)
    
    if len(candidates) == 0:
        raise ovl_module.NoMatchingOverloadError(
            module, qualname, (args, kwargs), functions, fail_reasons
        )
    if len(candidates) > 1:
        raise ovl_module.AmbiguousOverloadError(module, qualname, (args, kwargs), candidates)

    func = candidates[0]
    return func(*args, **kwargs)


cdef make_overloaded(func):
    """Make a function `func` overloaded.
    
    Add this to all functions with the same name in one scope. When calling a function with 
    this name, an appropriate overload will be picked based on the arguments you provide.
    Argument checking is performed by calling `bind_func(argument, annotation)`. If this call returns true, this
    argument is considered matching.
    """
    cdef str __module__ = func.__module__
    cdef str __qualname__ = func.__qualname__
    cdef list functions = []

    if (func.__module__, func.__qualname__) not in _registry:
        def overloaded_function(*args, **kwargs):
            return perform_overload_resolution(
                args, kwargs, 
                functions, 
                __module__, __qualname__
            )
        
        overloaded_function.__module__ = __module__
        overloaded_function.__qualname__ = __qualname__
        overloaded_function.functions = functions

        _registry[func.__module__, func.__qualname__] = overloaded_function
    else:
        overloaded_function = _registry[func.__module__, func.__qualname__]

    overloaded_function.functions.append(func)

    return overloaded_function


def overload(func):
    """Decorator that makes a function with this name overloaded.

    To create an overload set, create several functions with the same name in one scope and mark them with this
    decorator. This decorator supports annotations from the `typing` module.
    """
    # Cache function signature (this will be used during overload resolution)
    signature_cache[func] = createSignature(signature(func))

    return None
    #return make_overloaded(func, bind_annotated)


def overload_strict(func):
    """Decorator that makes a function with this name overloaded.
    
    To create an overload set, create several functions with the same name in one scope and mark them with this
    decorator. This decorator uses `isinstance` to match arguments to annotations, and does not support the `typing`
    module.
    """
    # Cache function signature (this will be used during overload resolution)
    signature_cache[func] = createSignature(signature(func))

    return make_overloaded(func)