"""Create overloaded functions using simple function decorators."""
from itertools import count
from inspect import signature
from .bind import bind_strict, bind_annotated, _signature_cache
from .inspect_bind import Signature as OptimizedSignature

__all__ = [
    # Decorators
    "overload",
    "overload_strict",

    # Decorator factory
    "make_overload",

    # Exceptions
    "OverloadError",
    "AmbiguousOverloadError",
    "NoMatchingOverloadError",
]


# Exceptions ===========================================================================================================
class OverloadError(TypeError):
    """An exception that is raised when there was an error during overload resolution.  
    This exception is not raised - it serves as a base class for AmbiguousOverloadError and NoMatchingOverloadError.
    """
    def __init__(self, module, qualname, arguments):
        self.module = module
        self.qualname = qualname
        self.call_args = arguments[0]
        self.call_kwargs = arguments[1]

    def __str__(self):
        return f"overload error during call to {self.module}.{self.qualname}"


class AmbiguousOverloadError(OverloadError):
    """An exception that is raised when arguments passed to a function match more that one overload."""
    def __init__(self, module, qualname, arguments, candidates):
        super().__init__(module, qualname, arguments)
                
        self.candidates = candidates
        """A list of candidates that matched the arguments."""
    
    def __str__(self):
        shortname = self.qualname[self.qualname.rfind(".")+1:]

        title = f"ambiguous overloaded call to {self.module}.{self.qualname}\nPossible candidates:\n"
        reasons = "\n".join(f"  {shortname}{signature(candidate)}" for candidate in self.candidates)
        
        return title + reasons


class NoMatchingOverloadError(OverloadError):
    """An exception that is raised when arguments passed to a function match none of the overloads."""
    def __init__(self, module, qualname, arguments, overloads, fail_reasons):
        super().__init__(module, qualname, arguments)
        
        self.overloads = overloads
        self.fail_reasons = fail_reasons
        """A list of reasons (TypeErrors) why each function did not match the overload."""
    
    def __str__(self):
        shortname = self.qualname[self.qualname.rfind(".")+1:]

        title = f"no matching overload found for {self.module}.{self.qualname}\nReason:\n"

        def generate_reasons():
            for func, reason in zip((func for func, _ in self.overloads), self.fail_reasons):
                yield f"  {shortname}{signature(func)}: {reason}"
        
        reasons = "\n".join(generate_reasons())

        return title + reasons


# Overload decorators ==================================================================================================
_registry = {}
"""A registry of all overloaded functions.  
This is how decorated functions communicate with each other to merge several functions into one.
Not for manual editing.
"""


def make_overload(bind):
    def overload(func):
        """Decorator that makes the function overloaded.
        Add this to all functions with the same name in one scope (global or in class). When calling a function with 
        this name, an appropriate overload will be picked based on the arguments you provide.
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
                    try:
                        bind(func, args, kwargs)
                    except TypeError as error:
                        fail_reasons.append(error)
                    else:
                        candidates.append(func)
                
                if len(candidates) == 0:
                    raise NoMatchingOverloadError(__module__, __qualname__, (args, kwargs), overloads, fail_reasons)
                if len(candidates) > 1:
                    raise AmbiguousOverloadError(__module__, __qualname__, (args, kwargs), candidates)
            
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

        ovl.overloads.append((func, bind))

        # Build documentation ------------------------------------------------------------------------------------------
        doc_decl = []
        doc_desc = []
        for i, func in zip(count(1), (func for func, _ in ovl.overloads)):
            doc_decl.append(f"{i}: {ovl.__name__}{signature(func)}")

            if func.__doc__ is not None:
                doc_desc.append(f"({i}) {func.__doc__}")
        
        ovl.__doc__ = "Overloaded function:\n" + "\n".join(doc_decl) + "\n\n" + "\n".join(doc_desc)
        # --------------------------------------------------------------------------------------------------------------

        # Cache function signature to speed up binding
        _signature_cache[func] = OptimizedSignature(signature(func))

        return ovl
    return overload


overload = make_overload(bind_annotated)
overload_strict = make_overload(bind_strict)
