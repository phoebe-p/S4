import setuptools
from setuptools.command.build_ext import build_ext
from distutils.command.clean import clean
from distutils.errors import DistutilsSetupError
from distutils import log as distutils_logger
import os, subprocess
import platform

if platform.system() == 'Linux':
    libs = ['S4', 'stdc++']
    libs.extend([lib[2::] for lib in '-lopenblas -lcholmod -lamd -lcolamd -lcamd -lccolamd'.split()])

    extra_link_args = ['./build/libS4.a']
    Makefile='Makefile' 

elif platform.system() =='Darwin':
    if int(platform.mac_ver()[0].split('.')[1]) >= 14:
        libs = ['S4', 'c++']
    else:
        ## Need for Mojave
        libs = ['S4', 'stdc++']

    extra_link_args = ['./build/libS4.a', '-Wl,-framework', '-Wl,Accelerate']

    Makefile='Makefile.osx' 
else:
    print(platform.system())
    raise NotImplementedError

S4module = setuptools.extension.Extension('S4B', 
                      sources = [ 'S4/main_python.c' ],
                      libraries = libs,
                      library_dirs = ['./build'],
                      # extra_link_args = ['./build/libS4.a'],
                      extra_link_args = extra_link_args,
                      extra_compile_args = ['-std=gnu99', '-O0'],)

S4module.Makefile=Makefile


class alt_build_ext(build_ext, object):
    """Alternate builder for  S4.
    It creates the .a using the pertinent Makefile,
    and afterwards the .so lib using setuptools
    """
    special_extension = S4module.name

    def build_extension(self, ext):

        if ext.name != self.special_extension:
            super(alt_build_ext, self).build_extension(ext)
        else:
            sources = ext.sources
            if sources is None or not isinstance(sources, (list, tuple)):
                raise DistutilsSetupError(
                       "in 'ext_modules' option (extension '%s'), "
                       "'sources' must be present and must be "
                       "a list of source filenames" % ext.name)
            sources = list(sources)

            if len(sources)>1:
                sources_path = os.path.commonpath(sources)
            else:
                sources_path = os.path.dirname(sources[0])

            sources_path = os.path.realpath(sources_path)

            if not sources_path.endswith(os.path.sep):
                sources_path+= os.path.sep

            if not os.path.exists(sources_path) or not os.path.isdir(sources_path):
                raise DistutilsSetupError("in 'extensions' option (extension '%s'), "
                       "the supplied 'sources' base dir "
                       "must exist" % ext.name)

            output_dir = os.path.realpath(os.path.join(sources_path,'..','lib'))

            if not os.path.exists(output_dir):
                os.makedirs(output_dir)

            output_lib = './build/libS4.a'

            distutils_logger.info('Will execute the following command' 
                                  'in with subprocess.Popen: \n{0}'
                                  .format('make -f Makefile.os '.format(output_lib,
                                            os.path.join(output_dir, output_lib))))
            
            os.system('make -f {0}'.format(S4module.Makefile))

            super(alt_build_ext, self).build_extension(ext)

class MakefileClean(clean):
        def run(self):
            super().run()
            os.system('make clean')


setuptools.setup(name = 'S4B',
	version = '1.1.2',
	description = "Fork Stanford Stratified Structure Solver (S4B): Fourier Modal Method",
        # install_requires=['fftw', 'openblas', 'suitesparse'],
        cmdclass = {'build_ext': alt_build_ext, 'clean':MakefileClean},
	ext_modules = [S4module]
)
