from .overloaded_function import OverloadedFunction

registry = {}

def _fullname(obj, /):
    module = obj.__module__
    if module is None:
        return obj.__qualname__
    return module + "." + obj.__qualname__

def overloaded(func):
    fullname = _fullname(func)

    if fullname not in registry:
        registry[fullname] = OverloadedFunction(fullname)
    
    ovlfunc = registry[fullname]
    ovlfunc.overloads.append(func)

    return ovlfunc.function
