import sys
from overload import overload, NoMatchingOverloadError


class Text:
    @overload
    def __init__(self, val: int):
        """Initialize text from an integer."""
        self.text = str(val)

    @overload
    def __init__(self, val: bool):
        """Initialize text from a boolean."""
        self.text = "yes" if val else "no"

@overload
def pos(x, y):
    return f"({x}, {y})"

@overload
def pos(pos: tuple):
    return f"({pos[0]}, {pos[1]})"

def main():
    # ------------------------------------------------------------------------------------------------------------------
    print("When calling an overloaded function, an appropriate overload is picked:")
    print("pos(1, 2):", pos(1, 2))

    p = (1, 2)
    print("pos(some_tuple):", pos(p))

    # ------------------------------------------------------------------------------------------------------------------
    print("\n\nWhen no appropriate overload exists, an exception is raised:")
    print("position_str(\"apple\"):")
    try:
        print(pos("apple"))
    except NoMatchingOverloadError as err:
        print(err)

    # ------------------------------------------------------------------------------------------------------------------
    print("\n\nOverloaded functions document their overloads:")
    help(pos)

if __name__ == "__main__":
    main()

# def func1(a, b=1):
#     return "func1"

# def func2(x, y, /):
#     return "func2"

# if __name__ == "__main__":
#     import overload
#     overload.core.hello("andreasxp")

#     print("===========================================================================================================")
#     x = overload.OverloadError("module", "function", (1, 2), {})
#     print("OverloadError:", x)

#     print("===========================================================================================================")
#     y = overload.AmbiguousOverloadError(
#         "module", "function",
#         (1, 2), {},
#         [func1, func2]
#     )
#     print("AmbiguousOverloadError:", y)

#     print("===========================================================================================================")
#     z = overload.NoMatchingOverloadError(
#         "module", "function",
#         (1, 2), {},
#         {
#             func1: "bad args",
#             func2: "bad kwargs"
#         }
#     )
#     print("NoMatchingOverloadError:", z)

#     overload.prepare(func1)
#     overload.prepare(func2)

#     print("===========================================================================================================")
#     try:
#         print("Resolved:", overload.por((1, 2), {}, [func1, func2], "module", "qualname"))
#     except overload.OverloadError as e:
#         print("Error:", e)

#     print("===========================================================================================================")
#     try:
#         print("Resolved:", overload.por(tuple(), {"a": 2, "b": 2}, [func1, func2], "module", "qualname"))
#     except overload.OverloadError as e:
#         print("Error:", e)

#     print("===========================================================================================================")
#     try:
#         print("Resolved:", overload.por(tuple(), {"x": 2}, [func1, func2], "module", "qualname"))
#     except overload.OverloadError as e:
#         print("Error:", e)

#     print("===========================================================================================================")
#     try:
#         print("Called:", overload.call(tuple(), {"a": 2, "b": 2}, [func1, func2], "module", "qualname"))
#     except overload.OverloadError as e:
#         print("Error:", e)

#     print("Done")
