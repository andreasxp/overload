from inspect import signature


class OverloadError(Exception):
    """An exception that is raised when there was an error during overload resolution.  
    This exception is not raised - it serves as a base class for AmbiguousOverloadError and NoMatchingOverloadError.
    """
    def __init__(self, ovl_func, arguments):
        self.ovl_func = ovl_func
        self.call_args = arguments[0]
        self.call_kwargs = arguments[1]

    def __str__(self):
        name = self.ovl_func.__module__ + self.ovl_func.__qualname__

        return "overload error during call to {}".format(name)


class AmbiguousOverloadError(OverloadError):
    """An exception that is raised when arguments passed to a function match more that one overload."""
    def __init__(self, ovl_func, arguments, candidates):
        super().__init__(ovl_func, arguments)
                
        self.candidates = candidates
        """A list of candidates that matched the arguments."""
    
    def __str__(self):
        name = self.ovl_func.__module__ + self.ovl_func.__qualname__
        shortname = self.ovl_func.__name__


        title = "ambiguous overloaded call to {}\nPossible candidates:\n".format(name)
        reasons = "\n".join("  {}{}".format(shortname, signature(candidate)) for candidate in self.candidates)
        
        return title + reasons


class NoMatchingOverloadError(OverloadError):
    """An exception that is raised when arguments passed to a function match none of the overloads."""
    def __init__(self, ovl_func, arguments, fail_reasons):
        super().__init__(ovl_func, arguments)
                
        self.fail_reasons = fail_reasons
        """A list of reasons why each function did not match the overload."""
    
    def __str__(self):
        name = self.ovl_func.__module__ + self.ovl_func.__qualname__
        shortname = self.ovl_func.__name__

        title = "no matching overload found for {}\nReason:\n".format(name)
        reasons = "\n".join("  {}{}: {}".format(shortname, signature(candidate), reason)
            for candidate, reason in zip(self.ovl_func.overloads, self.fail_reasons)
        )

        return title + reasons
