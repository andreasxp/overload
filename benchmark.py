import sys
from overload import overload, hello, call, prepare
from random import randint
import timeit

def func_ovl1(x, y):
    pass

def func_ovl2(val: tuple):
    pass

def func_ovl3(val: str):
    pass

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
    hello("")
    N = 10000000  # Number of tests

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

    prepare(func_ovl1)
    prepare(func_ovl2)
    prepare(func_ovl3)
    kwargs = {}
    funcs = [func_ovl1, func_ovl2, func_ovl3]
    def run_ovlraw():
        for arg in args:
            call(arg, kwargs, funcs, "module", "qualname")

    def run_normal():
        for arg in args:
            func_normal(*arg)

    print("Running benchmark...")

    time_ovl = timeit.timeit(run_ovl, number=1) / N
    time_ovlraw = timeit.timeit(run_ovlraw, number=1) / N
    time_normal = timeit.timeit(run_normal, number=1) / N

    try:
        from cProfile import Profile
        from pyprof2calltree import convert, visualize
        profiler = Profile()
        profiler.runctx("run_ovl()", globals(), locals())
        convert(profiler.getstats(), "C:/Users/andreasxp/Desktop/callgrind.profile")
    except ImportError:
        pass

    print(f"Average over {N} runs:")
    print(f"Overloaded function:     {time_ovl * 1000000:.2f} mcs ({time_ovl / time_normal:.2f}x)")
    print(f"Overloaded function raw: {time_ovlraw * 1000000:.2f} mcs ({time_ovlraw / time_normal:.2f}x)")
    print(f"Non-overloaded function: {time_normal * 1000000:.2f} mcs")


if __name__ == "__main__":
    main()
