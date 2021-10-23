# import sys
# from overload import overload, NoMatchingOverloadError


# class Text:
#     @overload
#     def __init__(self, val: int):
#         """Initialize text from an integer."""
#         self.text = str(val)
    
#     @overload
#     def __init__(self, val: bool):
#         """Initialize text from a boolean."""
#         self.text = "yes" if val else "no"

# @overload
# def pos(x, y):
#     return f"({x}, {y})"

# @overload
# def pos(pos: tuple):
#     return f"({pos[0]}, {pos[1]})"

# def main():
#     # ------------------------------------------------------------------------------------------------------------------
#     print("When calling an overloaded function, an appropriate overload is picked:")
#     print("pos(1, 2):", pos(1, 2))
    
#     p = (1, 2)
#     print("pos(some_tuple):", pos(p))

#     # ------------------------------------------------------------------------------------------------------------------
#     print("\n\nWhen no appropriate overload exists, an exception is raised:")
#     print("position_str(\"apple\"):")
#     try:
#         print(pos("apple"))
#     except NoMatchingOverloadError as err:
#         print(err)
    
#     # ------------------------------------------------------------------------------------------------------------------
#     print("\n\nOverloaded functions document their overloads:")
#     help(pos)

# if __name__ == "__main__":
#     main()

def func1(a, b=1):
    pass

def func2(x, y, /):
    pass

if __name__ == "__main__":
    import overload
    overload.hello(1, 2, 3, "a", "b")

    x = overload.OverloadError("module", "function", (1, 2), {})
    y = overload.AmbiguousOverloadError("module", "function", (1, 2), {}, [func1, func2])
    z = overload.NoMatchingOverloadError("module", "function", (1, 2), {}, [func1, func2], ["bad args", "bad kwargs"])
    print(x)
    print(y)
    print(z)
