import sys
from overload import overload_strict, NoMatchingOverloadError

overload = overload_strict


@overload
def position_str(x, y):
    print("({}, {})".format(x, y))

@overload
def position_str(pos: tuple):
    print("({}, {})".format(pos[0], pos[1]))

def main():
    print("position_str(1, 2):")
    print(position_str(1, 2))
    
    print("position_str(some_tuple):")
    print(position_str((1, 2)))

    try:
        print("position_str(\"apple\"):")
        print(position_str("apple"))
    except NoMatchingOverloadError as err:
        print(err)

if __name__ == "__main__":
    sys.excepthook = sys.__excepthook__
    main()