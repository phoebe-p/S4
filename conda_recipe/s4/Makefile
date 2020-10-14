CONDA_INC = $(CONDA_PREFIX)/lib/
CONDA_LIB = $(CONDA_PREFIX)/lib/

# BLAS_LIB = -L$(CONDA_LIB) -lmkl_core_dll -lmkl_rt
# LAPACK_LIB = -L$(CONDA_LIB) -lmkl_core_dll -lmkl_rt

BLAS_LIB = -L$(CONDA_LIB) -lopenblas
LAPACK_LIB = -L$(CONDA_LIB) -lopenblas

# FFTW3_INC = -I$(CONDA_INC)/include/
# FFTW3_LIB = -L$(CONDA_LIB) -lfftw3

PTHREAD_INC = -DHAVE_UNISTD_H
# PTHREAD_LIB = -lpthread

CHOLMOD_INC = -I$(CONDA_INC)
CHOLMOD_LIB = -L$(CONDA_LIB) -lcholmod -lamd -lcolamd -lcamd -lccolamd

BOOST_INC = -I$(CONDA_PREFIX)/include
BOOST_LIBS = -L$(CONDA_LIB) -lboost_serialization

# Specify custom compilers if needed
CXX = g++
CC  = gcc

#CFLAGS += -O3 -fPIC
CFLAGS = -Wall -O3 -m64 -msse3 -msse2 -msse -fPIC

OPTFLAGS = -O3

OBJDIR = ./build
S4_BINNAME = $(OBJDIR)/S4
S4_LIBNAME = $(OBJDIR)/libS4.a
S4r_LIBNAME = $(OBJDIR)/libS4r.a

CPPFLAGS = -Wall -I. -IS4 -IS4/RNP -IS4/kiss_fft 
 
ifdef BOOST_INC
	CPPFLAGS += $(BOOST_INC) $(BOOST_LIBS)
endif

ifdef BLAS_LIB
CPPFLAGS += -DHAVE_BLAS
endif

ifdef LAPACK_LIB
CPPFLAGS += -DHAVE_LAPACK
endif

ifdef FFTW3_LIB
CPPFLAGS += -DHAVE_FFTW3 $(FFTW3_INC)
endif

ifdef PTHREAD_LIB
CPPFLAGS += -DHAVE_LIBPTHREAD $(PTHREAD_INC)
endif

ifdef CHOLMOD_LIB
CPPFLAGS += -DHAVE_LIBCHOLMOD $(CHOLMOD_INC)
endif

ifdef MPI_LIB
CPPFLAGS += -DHAVE_MPI $(MPI_INC)
endif

LIBS = $(BLAS_LIB) $(LAPACK_LIB) $(FFTW3_LIB) $(PTHREAD_LIB) $(CHOLMOD_LIB) $(MPI_LIB) $(BOOST_LIBS)

#### Compilation targets

all: $(S4_LIBNAME)

objdir:
	mkdir -p $(OBJDIR)
	mkdir -p $(OBJDIR)/S4k
	mkdir -p $(OBJDIR)/S4r
	mkdir -p $(OBJDIR)/modules
	
S4_LIBOBJS = \
	$(OBJDIR)/S4k/S4.o \
	$(OBJDIR)/S4k/rcwa.o \
	$(OBJDIR)/S4k/fmm_common.o \
	$(OBJDIR)/S4k/fmm_FFT.o \
	$(OBJDIR)/S4k/fmm_kottke.o \
	$(OBJDIR)/S4k/fmm_closed.o \
	$(OBJDIR)/S4k/fmm_PolBasisNV.o \
	$(OBJDIR)/S4k/fmm_PolBasisVL.o \
	$(OBJDIR)/S4k/fmm_PolBasisJones.o \
	$(OBJDIR)/S4k/fmm_experimental.o \
	$(OBJDIR)/S4k/fft_iface.o \
	$(OBJDIR)/S4k/pattern.o \
	$(OBJDIR)/S4k/intersection.o \
	$(OBJDIR)/S4k/predicates.o \
	$(OBJDIR)/S4k/numalloc.o \
	$(OBJDIR)/S4k/gsel.o \
	$(OBJDIR)/S4k/sort.o \
	$(OBJDIR)/S4k/kiss_fft.o \
	$(OBJDIR)/S4k/kiss_fftnd.o \
	$(OBJDIR)/S4k/SpectrumSampler.o \
	$(OBJDIR)/S4k/cubature.o \
	$(OBJDIR)/S4k/Interpolator.o \
	$(OBJDIR)/S4k/convert.o

S4r_LIBOBJS = \
	$(OBJDIR)/S4r/Material.o \
	$(OBJDIR)/S4r/LatticeGridRect.o \
	$(OBJDIR)/S4r/LatticeGridArb.o \
	$(OBJDIR)/S4r/POFF2Mesh.o \
	$(OBJDIR)/S4r/PeriodicMesh.o \
	$(OBJDIR)/S4r/Shape.o \
	$(OBJDIR)/S4r/Simulation.o \
	$(OBJDIR)/S4r/Layer.o \
	$(OBJDIR)/S4r/Pseudoinverse.o \
	$(OBJDIR)/S4r/Eigensystems.o \
	$(OBJDIR)/S4r/IRA.o \
	$(OBJDIR)/S4r/intersection.o \
	$(OBJDIR)/S4r/predicates.o \
	$(OBJDIR)/S4r/periodic_off2.o

ifndef LAPACK_LIB
  S4_LIBOBJS += $(OBJDIR)/S4k/Eigensystems.o
endif

$(S4_LIBNAME): objdir $(S4_LIBOBJS)
	$(AR) crvs $@ $(S4_LIBOBJS)

$(OBJDIR)/S4k/S4.o: S4/S4.cpp
	$(CXX) -c $(CFLAGS) $(CPPFLAGS) $< -o $@
$(OBJDIR)/S4k/rcwa.o: S4/rcwa.cpp
	$(CXX) -c $(CFLAGS) $(CPPFLAGS) $< -o $@
$(OBJDIR)/S4k/fmm_common.o: S4/fmm/fmm_common.cpp
	$(CXX) -c $(CFLAGS) $(CPPFLAGS) $< -o $@
$(OBJDIR)/S4k/fmm_FFT.o: S4/fmm/fmm_FFT.cpp
	$(CXX) -c $(CFLAGS) $(CPPFLAGS) $< -o $@
$(OBJDIR)/S4k/fmm_kottke.o: S4/fmm/fmm_kottke.cpp
	$(CXX) -c $(CFLAGS) $(CPPFLAGS) $< -o $@
$(OBJDIR)/S4k/fmm_closed.o: S4/fmm/fmm_closed.cpp
	$(CXX) -c $(CFLAGS) $(CPPFLAGS) $< -o $@
$(OBJDIR)/S4k/fmm_PolBasisNV.o: S4/fmm/fmm_PolBasisNV.cpp
	$(CXX) -c $(CFLAGS) $(CPPFLAGS) $< -o $@
$(OBJDIR)/S4k/fmm_PolBasisVL.o: S4/fmm/fmm_PolBasisVL.cpp
	$(CXX) -c $(CFLAGS) $(CPPFLAGS) $< -o $@
$(OBJDIR)/S4k/fmm_PolBasisJones.o: S4/fmm/fmm_PolBasisJones.cpp
	$(CXX) -c $(CFLAGS) $(CPPFLAGS) $< -o $@
$(OBJDIR)/S4k/fmm_experimental.o: S4/fmm/fmm_experimental.cpp
	$(CXX) -c $(CFLAGS) $(CPPFLAGS) $< -o $@
$(OBJDIR)/S4k/fft_iface.o: S4/fmm/fft_iface.cpp
	$(CXX) -c $(CFLAGS) $(CPPFLAGS) $< -o $@
$(OBJDIR)/S4k/pattern.o: S4/pattern/pattern.c
	$(CC) -c $(CFLAGS) $(CPPFLAGS) $< -o $@
$(OBJDIR)/S4k/intersection.o: S4/pattern/intersection.c
	$(CC) -c $(CFLAGS) $(CPPFLAGS) $< -o $@
$(OBJDIR)/S4k/predicates.o: S4/pattern/predicates.c
	$(CC) -c $(CFLAGS) $(CPPFLAGS) $< -o $@
$(OBJDIR)/S4k/numalloc.o: S4/numalloc.c
	$(CC) -c $(CFLAGS) $(CPPFLAGS) $< -o $@
$(OBJDIR)/S4k/gsel.o: S4/gsel.c
	$(CC) -c $(CFLAGS) $(CPPFLAGS) $< -o $@
$(OBJDIR)/S4k/sort.o: S4/sort.c
	$(CC) -c $(CFLAGS) $(CPPFLAGS) $< -o $@
$(OBJDIR)/S4k/kiss_fft.o: S4/kiss_fft/kiss_fft.c
	$(CC) -c $(CFLAGS) $(CPPFLAGS) $< -o $@
$(OBJDIR)/S4k/kiss_fftnd.o: S4/kiss_fft/tools/kiss_fftnd.c
	$(CC) -c $(CFLAGS) $(CPPFLAGS) $< -o $@
$(OBJDIR)/S4k/SpectrumSampler.o: S4/SpectrumSampler.c
	$(CC) -c $(CFLAGS) $(CPPFLAGS) $< -o $@
$(OBJDIR)/S4k/cubature.o: S4/cubature.c
	$(CC) -c $(CFLAGS) $(CPPFLAGS) $< -o $@
$(OBJDIR)/S4k/Interpolator.o: S4/Interpolator.c
	$(CC) -c $(CFLAGS) $(CPPFLAGS) $< -o $@
$(OBJDIR)/S4k/convert.o: S4/convert.c
	$(CC) -c $(CFLAGS) $(CPPFLAGS) $< -o $@
$(OBJDIR)/S4k/Eigensystems.o: S4/RNP/Eigensystems.cpp
	$(CXX) -c $(CFLAGS) $(CPPFLAGS) $< -o $@

#### Python extension

S4_pyext: objdir $(S4_LIBNAME)
	sh conda_recipe/s4/gensetup.py.sh $(OBJDIR) $(S4_LIBNAME) "$(LIBS)" $(CONDA_LIB)
	$(PYTHON) setup.py install

clean:
	rm -rf $(OBJDIR)
