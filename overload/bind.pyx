#distutils: language = c++
#cython: profile=True
#cython: infer_types=True
#cython: boundscheck=False
#cython: wraparound=False
#cython: language_level = 3
from typing import _GenericAlias, Union, Any

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
