from typing import _GenericAlias, Union, Any
from .inspect_bind import bind


_signature_cache = {}

def bind_strict(func, args, kwargs):
    bind(_signature_cache[func], args, kwargs, binder=isinstance)

def bind_annotated(func, args, kwargs):
    def matchesannotation(obj, ann):
        """Return True if the object matches the annotation, including annotations from typing module."""
        if isinstance(ann, _GenericAlias):
            # typing annotations
            if ann.__origin__ is Union:
                return any(matchesannotation(obj, x) for x in ann.__args__)
            print("unknown typing annotation")
            return False
        elif ann is Any:
            return True
        else:
            return isinstance(obj, ann)

    bind(_signature_cache[func], args, kwargs, binder=matchesannotation)