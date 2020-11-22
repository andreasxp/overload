from setuptools import setup, Extension

setup(
    name="overload",
    version="0.1",
    description="Overload function decorator",
    author="Andrey Zhukov",
    author_email="andres.zhukov@gmail.com",
    license="MIT",
    packages=["overload"],
    ext_modules=[
        Extension("overload.inspect_bind", ["overload/inspect_bind.cpp"])
    ]
)
