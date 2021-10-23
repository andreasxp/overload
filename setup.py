from setuptools import setup, Extension

setup(
    name="overload",
    version="0.2-dev",
    description="Overload function decorator",
    author="Andrey Zhukov",
    author_email="andres.zhukov@gmail.com",
    license="MIT",
    ext_modules=[Extension("overload", ["src/overload.cpp"], language="c++")]
)
