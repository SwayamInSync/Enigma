from setuptools import setup, Extension
import sys
import os


python_include = sys.executable.replace(
    'bin/python', 'include/python{}.{}'.format(sys.version_info.major, sys.version_info.minor))

ext_modules = [
    Extension(
        'enigma.core.enigma',
        ['enigma/core/tensor.cpp', 'enigma/core/enigma_module.cpp'],
        include_dirs=[python_include, 'enigma/core'],
        language='c++',
        extra_compile_args=['-std=c++20', '-O2']
    ),
]

setup(
    name='enigma',
    version='0.0.1',
    author='Your Name',
    author_email='your.email@example.com',
    description='A custom tensor library',
    ext_modules=ext_modules,
    packages=['enigma', 'enigma.core'],
)
