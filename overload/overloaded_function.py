from .inspect import signature
from .exception import NoMatchingOverloadError, AmbiguousOverloadError


class OverloadedFunction:
    """Container class for an overloaded function.  
    Contains a list of callable overloads, a function name, and a method `function` that, when invoked, will try to bind
    arguments passed to it to one of the overloads. If one overload matches the passed arguments, it is invoked and
    result is returned. If none match, NoMatchingOverloadError is raised. If more than one match, AmbiguousOverloadError
    is raised.
    """

    def __init__(self):
        self.overloads = []
        """A list of tuples, containing tuples of (function, binder object).
        The function is one of the overloads, and the corresponding binder object is a callable that is used to
        check if the passed argument can be bound to the type annotation. It is picked depending on which overloading
        engine you used when creating overloaded functions.
        Although the package permits using different overload engines in one overload functions, this is not recommended
        because it can become difficult to reason about which overload will be picked.
        """

        # Names that are inherited from the source functions
        self.__name__ = None
        self.__qualname__ = None
        self.__module__ = None

    def __call__(self, *args, **kwargs):
        candidates = []
        fail_reasons = []

        for func, binder in self.overloads:
            try:
                signature(func, binder).bind(*args, **kwargs)
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

    def __repr__(self):
        return "<overloaded function {}.{} at {}>".format(self.__module__, self.__qualname__, id(self))