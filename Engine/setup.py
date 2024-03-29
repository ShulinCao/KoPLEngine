from distutils.core import setup, Extension

# define the extension module
cos_module = Extension('cos_module', sources=['cos_module.cpp'])

# run the setup
setup(ext_modules=[cos_module])