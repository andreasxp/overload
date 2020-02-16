from .typedinspect import signature
from .exception import NoMatchingOverloadError, AmbiguousOverloadError

class OverloadedFunction:
    def __init__(self, name):
        self.overloads = []
        self.name = name

        def function(*args, **kwargs):
            candidates = []
            candidate_bargs = []

            fail_reasons = []

            for func in self.overloads:
                try:
                    ba = signature(func).bind(*args, **kwargs)
                except TypeError as error:
                    fail_reasons.append(str(error))
                else:
                    candidates.append(func)
                    candidate_bargs.append(ba)
            
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
            ba = candidate_bargs[0]
            return func(*ba.args, **ba.kwargs)
    
        self.function = function

