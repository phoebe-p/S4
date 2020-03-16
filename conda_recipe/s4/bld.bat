@echo off

setlocal EnableDelayedExpansion

::if "%PY_VER%"=="2.7" (
::    set MSVC_VER=9.0
::    set LIB_VER=90
::) else if "%PY_VER%"=="3.4" (
::    set MSVC_VER=10.0
::    set LIB_VER=100
::) else (
::    set MSVC_VER=14.0
::    set LIB_VER=140
::)

CALL "C:\Program Files (x86)\Microsoft Visual Studio 14.0\VC\vcvarsall.bat" amd64

set "BLAS_PATH=%CONDA_PREFIX%/Library/lib/"
set "BLAS_LIB=mkl_core_dll mkl_rt"
set "LAPACK_PATH=%CONDA_PREFIX%/Library/lib/"
set "LAPACK_LIB=mkl_core_dll mkl_rt"
::set "BLAS_PATH=%CONDA_PREFIX%/Library/lib/"
::set "BLAS_LIB=openblas"
::set "LAPACK_PATH=%CONDA_PREFIX%/Library/lib/"
::set "LAPACK_LIB=openblas"

::set "FFTW3_INC=-I%CONDA_PREFIX%/include/"
::set "FFTW3_PATH=%CONDA_PREFIX%/Library/lib/"
::set "FFTW3_LIB=fftw3"

::set "PTHREAD_PATH=%CONDA_PREFIX%/Library/lib/"
::set "PTHREAD_LIB=pthread"

set "CHOLMOD_INC=-I%CONDA_PREFIX%\Library\include\suitesparse"
set "CHOLMOD_PATH=%CONDA_PREFIX%\Library\lib\"
set "CHOLMOD_LIB=cholmod amd colamd camd ccolamd suitesparseconfig metis"

set "BOOST_INC=-I%CONDA_PREFIX%\Library\include\"
set "BOOST_PATH=%CONDA_PREFIX%\Library\lib\"
set "BOOST_LIB="

:: Specify custom compilers if needed
set "CXX=cl /O2"
set "CC=cl /O2"

set "OBJDIR=build"
set "S4_BINNAME=%OBJDIR%/S4"
set "S4_LIBNAME=%OBJDIR%/S4.lib"

set "CPPFLAGS=-EHsc -MD -I. -IS4 -IS4/RNP -IS4/kiss_fft"
set "CPPFLAGS=%CPPFLAGS% %BOOST_INC%"
set "CPPFLAGS=%CPPFLAGS% -DHAVE_BLAS"
set "CPPFLAGS=%CPPFLAGS% -DHAVE_LAPACK"
::set "CPPFLAGS=%CPPFLAGS% -DHAVE_FFTW3 %FFTW3_INC%"
:: set "CPPFLAGS=%CPPFLAGS% -DHAVE_LIBPTHREAD %PTHREAD_INC%"
set "CPPFLAGS=%CPPFLAGS% -DHAVE_LIBCHOLMOD %CHOLMOD_INC%"

set "LIBS=%BLAS_LIB% %LAPACK_LIB% %FFTW3_LIB% %PTHREAD_LIB% %CHOLMOD_LIB% %BOOST_LIB%"
set "LIBPATHS=%BLAS_PATH% %LAPACK_PATH% %FFTW3_PATH% %PTHREAD_PATH% %CHOLMOD_PATH% %BOOST_PATH%"
set "S4_LIBOBJS=%OBJDIR%/S4k/S4.obj %OBJDIR%/S4k/rcwa.obj %OBJDIR%/S4k/fmm_common.obj %OBJDIR%/S4k/fmm_FFT.obj %OBJDIR%/S4k/fmm_kottke.obj %OBJDIR%/S4k/fmm_closed.obj %OBJDIR%/S4k/fmm_PolBasisNV.obj %OBJDIR%/S4k/fmm_PolBasisVL.obj %OBJDIR%/S4k/fmm_PolBasisJones.obj %OBJDIR%/S4k/fmm_experimental.obj %OBJDIR%/S4k/fft_iface.obj %OBJDIR%/S4k/pattern.obj %OBJDIR%/S4k/intersection.obj %OBJDIR%/S4k/predicates.obj %OBJDIR%/S4k/numalloc.obj %OBJDIR%/S4k/gsel.obj %OBJDIR%/S4k/sort.obj %OBJDIR%/S4k/kiss_fft.obj %OBJDIR%/S4k/kiss_fftnd.obj %OBJDIR%/S4k/SpectrumSampler.obj %OBJDIR%/S4k/cubature.obj %OBJDIR%/S4k/Interpolator.obj %OBJDIR%/S4k/convert.obj"

:: Make a build folder and change to it.
::cd S4
::cd S4
rmdir /Q /s %OBJDIR%
::
mkdir %OBJDIR%
cd %OBJDIR%
mkdir S4k
mkdir S4r
mkdir modules
cd ..
%CXX% -c %CPPFLAGS% S4/S4.cpp -Fo%OBJDIR%/S4k/S4.obj
%CXX% -c %CPPFLAGS% S4/rcwa.cpp -Fo%OBJDIR%/S4k/rcwa.obj
%CXX% -c %CPPFLAGS% S4/fmm/fmm_common.cpp -Fo%OBJDIR%/S4k/fmm_common.obj
%CXX% -c %CPPFLAGS% S4/fmm/fmm_FFT.cpp -Fo%OBJDIR%/S4k/fmm_FFT.obj
%CXX% -c %CPPFLAGS% S4/fmm/fmm_kottke.cpp -Fo%OBJDIR%/S4k/fmm_kottke.obj
%CXX% -c %CPPFLAGS% S4/fmm/fmm_closed.cpp -Fo%OBJDIR%/S4k/fmm_closed.obj
%CXX% -c %CPPFLAGS% S4/fmm/fmm_PolBasisNV.cpp -Fo%OBJDIR%/S4k/fmm_PolBasisNV.obj
%CXX% -c %CPPFLAGS% S4/fmm/fmm_PolBasisVL.cpp -Fo%OBJDIR%/S4k/fmm_PolBasisVL.obj
%CXX% -c %CPPFLAGS% S4/fmm/fmm_PolBasisJones.cpp -Fo%OBJDIR%/S4k/fmm_PolBasisJones.obj
%CXX% -c %CPPFLAGS% S4/fmm/fmm_experimental.cpp -Fo%OBJDIR%/S4k/fmm_experimental.obj
%CXX% -c %CPPFLAGS% S4/fmm/fft_iface.cpp -Fo%OBJDIR%/S4k/fft_iface.obj
%CC% -c %CPPFLAGS% S4/pattern/pattern.c -Fo%OBJDIR%/S4k/pattern.obj
%CC% -c %CPPFLAGS% S4/pattern/intersection.c -Fo%OBJDIR%/S4k/intersection.obj
%CC% -c %CPPFLAGS% S4/pattern/predicates.c -Fo%OBJDIR%/S4k/predicates.obj
%CC% -c %CPPFLAGS% S4/numalloc.c -Fo%OBJDIR%/S4k/numalloc.obj
%CC% -c %CPPFLAGS% S4/gsel.c -Fo%OBJDIR%/S4k/gsel.obj
%CC% -c %CPPFLAGS% S4/sort.c -Fo%OBJDIR%/S4k/sort.obj
%CC% -c %CPPFLAGS% S4/kiss_fft/kiss_fft.c -Fo%OBJDIR%/S4k/kiss_fft.obj
%CC% -c %CPPFLAGS% S4/kiss_fft/tools/kiss_fftnd.c -Fo%OBJDIR%/S4k/kiss_fftnd.obj
%CC% -c %CPPFLAGS% S4/SpectrumSampler.c -Fo%OBJDIR%/S4k/SpectrumSampler.obj
%CC% -c %CPPFLAGS% S4/cubature.c -Fo%OBJDIR%/S4k/cubature.obj
%CC% -c %CPPFLAGS% S4/Interpolator.c -Fo%OBJDIR%/S4k/Interpolator.obj
%CC% -c %CPPFLAGS% S4/convert.c -Fo%OBJDIR%/S4k/convert.obj
%CXX% -c %CPPFLAGS% S4/RNP/Eigensystems.cpp -Fo%OBJDIR%/S4k/Eigensystems.obj

@echo on
lib.exe %OBJDIR%/S4k/* /out:%S4_LIBNAME%



set "LIBS=%LIBS:\=/%"
set "LIBPATHS=%LIBPATHS:\=/%"
set "OBJDIR=%OBJDIR:\=/%"
set "S4_LIBNAME=%S4_LIBNAME:\=/%"

(echo:from distutils.core import setup, Extension
echo:import numpy as np
echo:
echo:lib_dirs = ['%OBJDIR%']
echo:lib_dirs.extend^([libpath for libpath in '%LIBPATHS%'.split^(^)]^)
echo:libs = ['S4']
echo:libs.extend^([lib for lib in '%LIBS%'.split^(^)]^)
echo:include_dirs = [np.get_include^(^)]
echo:extra_compile_args = ["-O2"]
echo:print^(lib_dirs^)
echo:print^(libs^)
echo:print^(include_dirs^)
echo:S4module = Extension^('S4',
echo:	sources = ['S4/main_python.c'],
echo:	libraries = libs,
echo:	library_dirs = lib_dirs,
echo:	include_dirs = include_dirs,
echo:	extra_objects = ['%S4_LIBNAME%'],
echo:   extra_compile_args = extra_compile_args
echo:^)
echo:
echo:setup^(name = 'S4',
echo:	version = '1.1',
echo:	description = 'Stanford Stratified Structure Solver ^(S4^): Fourier Modal Method',
echo:	ext_modules = [S4module]
echo:^))>setup.py

IF DEFINED PYTHON ("%PYTHON%" setup.py install) ELSE (python setup.py install)


