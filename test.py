from typing import Union
from overload import overload, OverloadError


class Test:
    def __init__(self):
        self.calls = 0
        
    @overload
    def setPos(self, x: float, y: float = 1):
        self.calls += 1
        print("Set1 pos to {} and {}", x, y)

    @overload
    def setPos(self, x: Union[float, None], y: int = 1):
        self.calls += 1
        print("Set2 pos to {} and {}", x, y)

    @overload
    def setPos(self, pos):
        self.calls += 1
        print("Set pos to ", pos)

@overload
def setPos(x: float, y: float = 1):
    print("Set1 pos to {} and {}", x, y)

@overload
def setPos(x: float, y: int = 1):
    print("Set2 pos to {} and {}", x, y)

@overload
def setPos(pos: tuple):
    print("Set pos to ", pos)	

def main():
    try:
        t = Test()
        t.setPos(1, 1)
        #setPos(1, 1)
    except OverloadError as e:
        print(str(e))

if __name__ == "__main__":
    main()
