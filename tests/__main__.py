import unittest
from unittest import TestCase
from overload import overload_strict, AmbiguousOverloadError, NoMatchingOverloadError

overload = overload_strict


class TestBaseEncoder(TestCase):
    def test_simple(self):
        @overload
        def foo(x: int):
            return 0
        
        @overload
        def foo(x: str):
            return 1

        self.assertEqual(foo(1), 0)
        self.assertEqual(foo("apple"), 1)

    def test_simple_ambiguous(self):
        @overload
        def foo(x: int):
            return 0
        
        @overload
        def foo(x: int):
            return 1

        self.assertRaises(AmbiguousOverloadError, foo, 1)
    
    def test_simple_nomatching(self):
        @overload
        def foo(x: int):
            return 0
        
        @overload
        def foo(x: str):
            return 1

        self.assertRaises(NoMatchingOverloadError, foo, [])
    
    def test_positional(self):
        @overload
        def foo(x):
            return 0
        
        @overload
        def foo(x, y):
            return 1

        self.assertEqual(foo(1), 0)
        self.assertEqual(foo(1, 2), 1)
    
    def test_keyword(self):
        @overload
        def foo(x):
            return 0
        
        @overload
        def foo(y):
            return 1

        self.assertEqual(foo(x=1), 0)
        self.assertEqual(foo(y=1), 1)


if __name__ == "__main__":
    unittest.main()
