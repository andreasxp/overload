from .overloaded_function import OverloadedFunction

registry = {}
"""A registry of all overloaded functions.  
This is how decorated functions communicate with each other to merge several functions into one.
Not for manual editing.
"""

def make_overload(binder):
    def overload(func):
        """Decorator that makes the function overloaded.
        Add this to all functions with the same name in one scope (global or in class). When calling a function with this
        name, an appropriate overload will be picked based on the arguments you provide.
        """
        if (func.__module__, func.__qualname__) not in registry:
            ovl_func = OverloadedFunction()
            ovl_func.__name__ = func.__name__
            ovl_func.__qualname__ = func.__qualname__
            ovl_func.__module__ = func.__module__

            registry[func.__module__, func.__qualname__] = ovl_func
        else:
            ovl_func = registry[func.__module__, func.__qualname__]

        ovl_func.overloads.append((func, binder))

        return ovl_func
    return overload

overload_strict = make_overload(isinstance)