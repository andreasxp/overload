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
        Extension("overload.bind_with", ["overload/bind_with.cpp"]),
        Extension("overload.bind", ["overload/bind.cpp"]),
        Extension("overload.signature", ["overload/signature.cpp"]),
        Extension("overload.overload", ["overload/overload.cpp"]),
    ]
)
