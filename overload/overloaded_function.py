from .typedinspect import signature
from .exception import NoMatchingOverloadError, AmbiguousOverloadError

class OverloadedFunction:
    """Container class for an overloaded function.  
    Contains a list of callable overloads, a function name, and a method `function` that, when invoked, will try to bind
    arguments passed to it to one of the overloads. If one overload matches the passed arguments, it is invoked and
    result is returned. If none match, NoMatchingOverloadError is raised. If more than one match, AmbiguousOverloadError
    is raised.
    """

    def __init__(self, name):
        self.overloads = []
        """A list of callable overloads."""

        self.name = name
        """Function name."""

        def function(*args, **kwargs):
            candidates = []
            fail_reasons = []

            for func in self.overloads:
                try:
                    signature(func).bind(*args, **kwargs)
                except TypeError as error:
                    fail_reasons.append(str(error))
                else:
                    candidates.append(func)
            
            if len(candidates) == 0:
                raise NoMatchingOverloadError(
                    self, 
                    (args, kwargs), 
                    fail_reasons
                )
            if len(candidates) > 1:
                raise AmbiguousOverloadError( 
                    self, 
                    (args, kwargs), 
                    candidates
                )
        
            func = candidates[0]
            return func(*args, **kwargs)
    
        self.function = function
