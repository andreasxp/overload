from setuptools import setup

install_requires = []
extras_require = {}

setup(
    name="overload",
    version="0.1",
    description="Overload function decorator",
    author="Andrey Zhukov",
    author_email="andres.zhukov@gmail.com",
    license="MIT",
    install_requires=install_requires,
    package_data={},
    extras_require=extras_require,
    packages=["overload"],
)
