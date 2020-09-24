import inspect
from pytypes import is_of_type


class Signature(inspect.Signature):    
    def bind(self, *args, **kwargs):
        result = super().bind(*args, **kwargs)

        for name in result.arguments:
            value = result.arguments[name]

            expected_type = self.parameters[name].annotation
            if expected_type == self.empty:
                expected_type = object

            if not is_of_type(value, expected_type):
                raise TypeError("argument '" + name + "' has unexpected type '" + type(value).__name__ + "'")
        
        return result


def signature(callable, *, follow_wrapped=True):
    return Signature.from_callable(callable, follow_wrapped=follow_wrapped)
