# Installation instructions:
In principle you should be able to install this on Windows and OS X using conda and 'conda install -c marcus-o s4'.
Install in a new environment to avoid incompatabilities.
I don't have a large test base, so I would appreciate if you let me know if it works (marcus.ossiander at gmail) or not, then I'll try to help).

# Detailed installation instructions (64-bit Ubuntu 16 or 18):
## Key steps:

```
git clone https://github.com/phoebe-p/S4
cd S4
make boost
make S4_pyext
```

## Installing relevant libraries etc.:

```sudo apt-get update
sudo apt install make
sudo apt install git
sudo apt install python3-pip
sudo pip3 install numpy matplotlib scipy
sudo apt-get install python3-tk
sudo apt-get install libopenblas-dev
sudo apt-get install libfftw3-dev
sudo apt-get install libsuitesparse-dev
```

- libopenblas installs OpenBLAS, to satisfy the LAPACK and BLAS requirements
- lib fftw3 install FFTW3, to satisfy the FFTW requirements
- libsuitesparse install libraries to satisdy CHOLMOD & related requirements

*If you have multiple Python versions, you make need to modify the S4_pyext part of the Makefile:*

````
pip3 install --upgrade ./
````

to e.g.:
```
[path of target python or virtual environment] setup.py install
```

-------------------------------------

S4: Stanford Stratified Structure Solver (http://fan.group.stanford.edu/S4/)

A program for computing electromagnetic fields in periodic, layered
structures, developed by Victor Liu (victorliu@alumni.stanford.edu) of the
Fan group in the Stanford Electrical Engineering Department.

See the S4 manual, in doc/index.html, for a complete
description of the package and its user interface, as well as
installation instructions, the license and copyright, contact
addresses, and other important information.

---------------------------------------

sajidmc: The MakefileHPC is created to compile the S4 in HPC environment without
root access. Also added an example for python extension testing: 

Check Wiki for details: https://github.com/sajidmc/S4/wiki
Example result: https://raw.githubusercontent.com/sajidmc/S4/master/examples/bontempi_et_al_Nanoscale_2017.png

