from overload.core import call, prepare, hello, resolve, OverloadError, AmbiguousOverloadError, NoMatchingOverloadError

__all__ = [
    "make_overloaded",
    "hello",
    "call",
    "prepare",
    "resolve",

    # Decorators
    "overload",

    # Exceptions
    "OverloadError",
    "AmbiguousOverloadError",
    "NoMatchingOverloadError",
]

_registry = {}
"""A registry of all overloaded functions.
This is how decorated functions communicate with each other to merge several functions into one.
Not for manual editing.
"""

def make_overloaded(func):
    """Make a function `func` overloaded.

    Add this to all functions with the same name in one scope. When calling a function with
    this name, an appropriate overload will be picked based on the arguments you provide.
    Argument checking is performed by calling `bind_func(argument, annotation)`. If this call returns true, this
    argument is considered matching.
    """
    __module__ = func.__module__
    __qualname__ = func.__qualname__
    functions = []

    if (func.__module__, func.__qualname__) not in _registry:
        def overloaded_function(*args, **kwargs):
            return call(args, kwargs, functions, __module__, __qualname__)

        overloaded_function.__module__ = __module__
        overloaded_function.__qualname__ = __qualname__
        overloaded_function.functions = functions

        _registry[func.__module__, func.__qualname__] = overloaded_function
    else:
        overloaded_function = _registry[func.__module__, func.__qualname__]

    prepare(func)
    overloaded_function.functions.append(func)

    return overloaded_function


def overload(func):
    """Decorator that makes a function with this name overloaded.

    To create an overload set, create several functions with the same name in one scope and mark them with this
    decorator. This decorator uses `isinstance` to match arguments to annotations, and does not support the `typing`
    module.
    """
    return make_overloaded(func)
