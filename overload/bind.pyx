#distutils: language = c++
#cython: language_level = 3
from typing import _GenericAlias, Union, Any
from .bind_with cimport bind_with


_signature_cache = {}

def bind_strict(func, args, kwargs):
    bind_with(_signature_cache[func], isinstance, args, kwargs)

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

    bind_with(_signature_cache[func], matchesannotation, args, kwargs)