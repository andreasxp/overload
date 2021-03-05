import sys
from overload import overload_strict as overload
from random import randint
import timeit
from cProfile import Profile
from pyprof2calltree import convert, visualize

@overload
def func_ovl(x, y):
    pass

@overload
def func_ovl(val: tuple):
    pass

@overload
def func_ovl(val: str):
    pass

def func_normal(*args):
    if len(args) == 2:
        return
    elif isinstance(args[0], tuple):
        return
    elif isinstance(args[0], str):
        return
    else:
        raise TypeError

def main():
    N = 100000  # Number of tests

    # Different argument types
    variants = [
        (1, 3),
        ((1, 3),),
        ("fox",),
    ]

    args = []

    for i in range(N):
        args.append(variants[randint(0, len(variants)-1)])
    
    def run_ovl():
        for arg in args:
            func_ovl(*arg)
    
    def run_normal():
        for arg in args:
            func_normal(*arg)
    
    print("Running benchmark...")

    time_ovl = timeit.timeit(run_ovl, number=1) / N
    time_normal = timeit.timeit(run_normal, number=1) / N

    profiler = Profile()
    profiler.runctx("run_ovl()", globals(), locals())
    convert(profiler.getstats(), "C:/Users/andreasxp/Desktop/callgrind.profile")

    print(f"Average over {N} runs:")
    print(f"Overloaded function:     {time_ovl * 1000000:.2f} mcs ({time_ovl / time_normal:.2f}x)")
    print(f"Non-overloaded function: {time_normal * 1000000:.2f} mcs")


if __name__ == "__main__":
    main()
