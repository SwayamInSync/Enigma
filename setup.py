from setuptools import setup, Extension
from setuptools.command.build_ext import build_ext
import sys
import os

class BuildExt(build_ext):
    def build_extensions(self):
        if sys.platform == 'darwin':
            os.environ['LDFLAGS'] = '-undefined dynamic_lookup'
        super().build_extensions()

ext_modules = [
    Extension(
        'enigma.core.enigma',
        ['enigma/core/tensor.cpp', 'enigma/core/enigma_module.cpp'],
        include_dirs=['/usr/include/python3.12'],  # Adjust this path if necessary
        language='c++'
    ),
]

setup(
    name='enigma',
    version='0.0.1',
    author='Your Name',
    author_email='your.email@example.com',
    description='A custom tensor library',
    long_description='',
    ext_modules=ext_modules,
    cmdclass={'build_ext': BuildExt},
    packages=['enigma', 'enigma.core'],
    package_data={'enigma': ['core/*.so']},
)