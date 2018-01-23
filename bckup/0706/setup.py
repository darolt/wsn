#!/usr/bin/env python

"""setup that compiles C++ wrapper to be used as python module.
"""

from distutils.core import setup, Extension
import os

os.environ["CC"] = "g++-5 -std=c++11"

pso = Extension('_pso',
                sources=['pso_wrap.cxx', 'pso.cpp', 'regions.cpp'])

setup (name        = 'pso',
       version     = '0.1',
       author      = "JRO",
       description = """C++ wrappers for performance optimization.""",
       ext_modules = [pso],
       py_modules  = ["pso"],
       )
