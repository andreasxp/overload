from .overloaded_function import OverloadedFunction

registry = {}
"""A registry of all overloaded functions.  
This is how decorated functions communicate with each other to merge several functions into one.
Not for manual editing.
"""

def _fullname(obj, /):
    """Full name of the function, including the module it comes from."""
    module = obj.__module__
    if module is None:
        return obj.__qualname__
    return module + "." + obj.__qualname__

def overload(func):
    """Decorator that makes the function overloaded.
    Add this to all functions with the same name in one scope (global or in class). When calling a function with this
    name, an appropriate overload will be picked based on the arguments you provide.
    """
    fullname = _fullname(func)

    if fullname not in registry:
        registry[fullname] = OverloadedFunction(fullname)
    
    ovlfunc = registry[fullname]
    ovlfunc.overloads.append(func)

    return ovlfunc.function
