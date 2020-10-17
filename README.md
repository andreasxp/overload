# overload
A python package that makes it possible to overload functions.  
An overloaded function has several signatures and definitions, one of which is picked depending on the arguments:
```python
@overload
def position(x, y):
  print(f"({x}, {y})")

@overload
def position(t: tuple):
  print(f"({t[0]}, {y[1]})")

@overload
def position(pos: QPoint):
  print(f"{pos.x()}, {pos.y()}")


position(1, 2)  # Picks overload 1
position((1, 2))  # Picks overload 2
position(QPoint())  # Picks overload 3
```

## Installation
The recommended method of installation is through pip:
```
pip install https://github.com/andreasxp/overload/archive/master.zip
```
Since this package is not yet published on PyPI, installing from Github is the only option.

## Usage
Import `overload` from the package:
```python
from overload import overload
```
Use it to overload functions or methods:
```python
class Texture:
  @overload
  def __init__(color: int):
    # create texture from a color code
  
  @overload
  def __init__(image_path: str):
    # create texture from an image
```
```python
@overload
def print_sequence(*items):
  print(", ".join(items))

@overload
def print_sequence(seq: list):
  print_sequence(*seq)
```
