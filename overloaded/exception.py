from .typedinspect import signature

class OverloadError(Exception):
    def __init__(self, ovl_function, arguments):
        self.ovl_function = ovl_function
        self.call_args = arguments[0]
        self.call_kwargs = arguments[1]

    def __str__(self):
        return "overload error during call to " + self.ovl_function.name

class AmbiguousOverloadError(OverloadError):
    def __init__(self, ovl_function, arguments, candidates):
        super().__init__(ovl_function, arguments)
                
        self.candidates = candidates
    
    def __str__(self):
        shortname = self.ovl_function.name.split(".")[-1]

        message = "ambiguous overloaded call to " + self.ovl_function.name + "\n" + "Possible candidates:\n"
        
        for candidate in self.candidates:
            message = message + "- " + shortname + str(signature(candidate)) + "\n"
        
        return message


class NoMatchingOverloadError(OverloadError):
    def __init__(self, ovl_function, arguments, fail_reasons):
        super().__init__(ovl_function, arguments)
                
        self.fail_reasons = fail_reasons
    
    def __str__(self):
        shortname = self.ovl_function.name.split(".")[-1]

        message = "no matching overload found for " + self.ovl_function.name + "\n" + "Reason:\n"

        for candidate, reason in zip(self.ovl_function.overloads, self.fail_reasons):
            message = message + "- " + shortname + str(signature(candidate)) + ": " + reason + "\n"
        
        return message
