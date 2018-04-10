#!/bin/bash

OBJDIR="$1"
LIBFILE="$2"
LIBS="$3"

cat <<SETUPPY > setup.py
from distutils.core import setup, Extension

libs = ['S4', 'stdc++']
libs.extend( [lib[2::] for lib in '$LIBS'.split()])

S4module = Extension('S4',
	sources = [
		'S4/main_python.c'
	],
	libraries = libs,
	library_dirs = ['$OBJDIR'],
	extra_link_args = [
		'$LIBFILE',
        '-Wl,-framework',
        '-Wl,Accelerate',
    
	],
	extra_compile_args=['-std=gnu99'] 
)

setup(name = 'S4',
	version = '1.1',
	description = 'Stanford Stratified Structure Solver (S4): Fourier Modal Method',
	ext_modules = [S4module]
)
SETUPPY
