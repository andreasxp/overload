"""Create overloaded functions using simple function decorators."""

__all__ = [
    # Decorators
    "overload",
    "overload_strict",

    # Decorator factory
    "decorator",

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


from .overload import overload, overload_strict, decorator