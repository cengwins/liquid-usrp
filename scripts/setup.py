import os, sys

from distutils.core import setup, Extension
from distutils import sysconfig
# '-lliquidusrp',
cpp_args = ['-w', '-std=c++11', '-g']
link_args = ['-v', '-L.', '-L/usr/local/lib', '-lfec', '-lboost_system', '-lpthread', '-luhd', '-lliquid', '-lm', '-lc']
ext_modules = [
    Extension(
    'liquid_usrp_pybind11_wrapper',
        [ 'lib/liquid_usrp_pybind11_wrapper.cc',  'lib/multichannelrx.cc','lib/multichanneltx.cc', 'lib/multichanneltxrx.cc', 'lib/ofdmtxrx.cc', 'lib/timer.cc'],
        include_dirs=['pybind11/include','include'],
    language='c++',
    extra_compile_args = cpp_args,
    extra_link_args = link_args,
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