import inspect
from pytypes import is_of_type


class Signature(inspect.Signature):
    """Signature object, derived from inspect.Signature.
    Overloads the `bind` method to check the argument types using is_of_type function from pytypes.
    is_of_type is different from isinstance in that is correctly works with type hints from `typing`. 
    """
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
    """Returns a Signature object from a callable."""
    return Signature.from_callable(callable, follow_wrapped=follow_wrapped)
