#distutils: language = c++
#cython: language_level = 3
from inspect import signature
from itertools import count
import overload as ovl_module
from .bind_with import Signature as OptimizedSignature
from .bind import bind_strict, bind_annotated, _signature_cache

_registry = {}
"""A registry of all overloaded functions.  
This is how decorated functions communicate with each other to merge several functions into one.
Not for manual editing.
"""

def make_overloaded(func, bind_func):
    """Make a function `func` overloaded.
    
    Add this to all functions with the same name in one scope. When calling a function with 
    this name, an appropriate overload will be picked based on the arguments you provide.
    Argument checking is performed by calling `bind_func(argument, annotation)`. If this call returns true, this
    argument is considered matching.
    """
    if (func.__module__, func.__qualname__) not in _registry:
        overloads = []

        __name__ = func.__name__
        __qualname__ = func.__qualname__
        __module__ = func.__module__

        # Create the function --------------------------------------------------------------------------------------
        def ovl(*args, **kwargs):
            candidates = []
            fail_reasons = []

            for func, bind in overloads:
                error = bind(func, args, kwargs)
                if error is not None:
                    fail_reasons.append(error)
                else:
                    candidates.append(func)
            
            if len(candidates) == 0:
                raise ovl_module.NoMatchingOverloadError(
                    __module__, __qualname__, (args, kwargs), overloads, fail_reasons
                )
            if len(candidates) > 1:
                raise ovl_module.AmbiguousOverloadError(__module__, __qualname__, (args, kwargs), candidates)
        
            func = candidates[0]
            return func(*args, **kwargs)

        # Customize function attributes ----------------------------------------------------------------------------
        # Add overloads as an attribute
        ovl.overloads = overloads

        # Overload special attributes to make this function as similar as possible to a normal function
        ovl.__name__ = __name__
        ovl.__qualname__ = __qualname__
        ovl.__module__ = __module__
        
        # repr does not work :(
        # def __repr__():
        #     return "<overloaded function {}.{} at {}>".format(__module__, __qualname__, id(ovl))
        # ovl.__repr__ = __repr__
        # ----------------------------------------------------------------------------------------------------------

        _registry[func.__module__, func.__qualname__] = ovl
    else:
        ovl = _registry[func.__module__, func.__qualname__]

    ovl.overloads.append((func, bind_func))

    # Build documentation ------------------------------------------------------------------------------------------
    doc_decl = []
    doc_desc = []
    for i, func in zip(count(1), (func for func, _ in ovl.overloads)):
        doc_decl.append(f"{i}: {ovl.__name__}{signature(func)}")

        if func.__doc__ is not None:
            doc_desc.append(f"({i}) {func.__doc__}")
    
    ovl.__doc__ = "Overloaded function:\n" + "\n".join(doc_decl) + "\n\n" + "\n".join(doc_desc)
    # --------------------------------------------------------------------------------------------------------------

    return ovl

def decorator(bind_func):
    """Create an overload decorator with a particular bind function."""
    def overload(func):
        """Decorator that makes a function with this name overloaded.
        
        To create an overload set, create several functions with the same name in one scope and mark them with this
        decorator.
        """
        return make_overloaded(func, bind_func)

    return overload


_overload = decorator(bind_annotated)
def overload(func):
    """Decorator that makes a function with this name overloaded.

    To create an overload set, create several functions with the same name in one scope and mark them with this
    decorator. This decorator supports annotations from the `typing` module.
    """
    # Cache function signature (this will be used during overload resolution)
    _signature_cache[func] = OptimizedSignature(signature(func))

    return _overload(func)

_overload_strict = decorator(bind_strict)
def overload_strict(func):
    """Decorator that makes a function with this name overloaded.
    
    To create an overload set, create several functions with the same name in one scope and mark them with this
    decorator. This decorator uses `isinstance` to match arguments to annotations, and does not support the `typing`
    module.
    """
    # Cache function signature (this will be used during overload resolution)
    _signature_cache[func] = OptimizedSignature(signature(func))

    return _overload_strict(func)