import os, sys

from distutils.core import setup, Extension
from distutils import sysconfig

cpp_args = ['-std=c++11', '-stdlib=libc++', '-mmacosx-version-min=10.7']

ext_modules = [
    Extension(
    'pybind11_wrapper',
        ['lib/funcs.cc', 'lib/pybind11_wrapper.cc'],
        include_dirs=['pybind11/include','include'],
    language='c++',
    extra_compile_args = cpp_args,
    ),
]

setup(
    name='pybind11_wrapper',
    version='0.0.1',
    author='Ertan Pnur',
    author_email='eronur@metu.edu.tr',
    description='pybind11_wrapper for liquid_usrp',
    ext_modules=ext_modules,
)