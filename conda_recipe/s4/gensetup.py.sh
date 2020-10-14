#!/bin/bash

OBJDIR="$1"
LIBFILE="$2"
LIBS="$3"
BOOST_LIBS="$4"

echo "LIBFILE: $LIBFILE"

cat <<SETUPPY > setup.py
from distutils.core import setup, Extension
import numpy as np
#import os
#os.environ["CC"] = "g++"
#os.environ["CXX"] = "g++"

libs = ['S4', 'stdc++']
lib_dirs = ['$OBJDIR', '$BOOST_LIBS']
libs.extend([lib[2::] for lib in '$LIBS'.split()])
include_dirs = ['$BOOST_PREFIX/include', np.get_include()]
extra_link_args = ['$LIBFILE']

S4module = Extension('S4',
	sources = ['S4/main_python.c'],
	libraries = libs,
	library_dirs = lib_dirs,
    include_dirs = include_dirs,
    extra_objects = ['$LIBFILE'],
	# extra_link_args = extra_link_args,
    runtime_library_dirs=['$BOOST_PREFIX/lib'],
	extra_compile_args=['-std=gnu99'] 
)

setup(name = 'S4',
	version = '1.1',
	description = 'Stanford Stratified Structure Solver (S4): Fourier Modal Method',
	ext_modules = [S4module]
)
SETUPPY
