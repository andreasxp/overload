import sys
from overload import overload_strict as overload
from random import randint
import timeit
from cProfile import Profile
from pyprof2calltree import convert, visualize

@overload
def func(x, y):
    pass

@overload
def func(val: tuple):
    pass

@overload
def func(val: str):
    pass

def func2(*args):
    if len(args) == 2:
        return
    elif isinstance(args[0], tuple):
        return
    elif isinstance(args[0], str):
        return
    else:
        raise TypeError

def main():
    N = 100

    variants = [
        (1, 3),
        ((1, 3),),
        ("fox",),
    ]

    args = []

    for i in range(N):
        args.append(variants[randint(0, len(variants)-1)])
    
    def run():
        for arg in args:
            func(*arg)
    
    print(timeit.timeit(run, number=1) * 1000000 / N, " mcs")
    profiler = Profile()
    profiler.runctx("run()", globals(), locals())
    convert(profiler.getstats(), "C:/Users/andreasxp/Desktop/callgrind.profile")


if __name__ == "__main__":
    main()