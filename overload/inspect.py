import inspect


class Signature(inspect.Signature):
    """Signature class, derived from inspect.Signature. Overrides the `bind` method to compare args to parameter
    annotations.  
    When constructing this signature (using the function `signature`) you can specify a special binder object - a 
    callable `binder(arg, annotation) -> bool` that returns True if the arguments matches the parameter annotation.
    The simplest case of such a binder is `isinstance`. Parameters that do not have annotations will use `object` as an
    annotation instead.
    """
    def bind(self, binder, *args, **kwargs):
        result = super().bind(*args, **kwargs)

        for name in result.arguments:
            value = result.arguments[name]

            expected_type = self.parameters[name].annotation
            if expected_type == self.empty:
                expected_type = object

            if not binder(value, expected_type):
                raise TypeError("argument '{}' has unexpected type '{}'".format(name, type(value).__qualname__))
        
        return result


def signature(callable, *, follow_wrapped=True):
    """Returns a Signature object from a callable, optionally providing a special binder object."""
    return Signature.from_callable(callable, follow_wrapped=follow_wrapped)
