#!/usr/bin/env python3
"""
Setuptools-based build configuration for S4 Python extension.

This is a transitional wrapper around the existing Make-based native build.
The native S4 library (libS4.a) must be built first using the Makefile.
"""

from pathlib import Path
import os
import shlex
from setuptools import Extension, setup
import numpy as np


def parse_linker_flags(flags_str: str):
    """
    Parse linker flags from Makefile LIBS variable.
    
    Converts flags like '-lfoo -L/path' into appropriate setuptools parameters.
    Preserves order for linker-sensitive flags.
    """
    libraries = []
    library_dirs = []
    extra_link_args = []
    
    for token in shlex.split(flags_str):
        if token.startswith('-l'):
            libraries.append(token[2:])
        elif token.startswith('-L'):
            library_dirs.append(token[2:])
        else:
            extra_link_args.append(token)
    
    return libraries, library_dirs, extra_link_args


def main():
    # Get build configuration from environment variables (set by Makefile)
    objdir = os.environ.get('S4_OBJDIR', './build')
    libfile = os.environ.get('S4_LIBFILE', './build/libS4.a')
    link_flags = os.environ.get('S4_LINK_FLAGS', '')
    boost_prefix = os.environ.get('BOOST_PREFIX', f'{Path(__file__).parent.resolve()}/S4')
    
    # Parse linker flags
    libs, lib_dirs, extra_link_args = parse_linker_flags(link_flags)
    
    # Base libraries required for S4 extension
    libs.extend(['S4', 'stdc++'])
    lib_dirs.extend([objdir, f'{boost_prefix}/lib'])
    
    # Include directories
    include_dirs = [
        f'{boost_prefix}/include',
        np.get_include(),
    ]
    
    # Extension configuration
    S4module = Extension(
        'S4',
        sources=['S4/main_python.c'],
        libraries=libs,
        library_dirs=lib_dirs,
        include_dirs=include_dirs,
        extra_objects=[libfile],
        runtime_library_dirs=[f'{boost_prefix}/lib'],
        extra_compile_args=['-std=gnu99'],
    )
    
    # Setup configuration (metadata comes from pyproject.toml)
    setup(
        ext_modules=[S4module],
    )


if __name__ == '__main__':
    main()