#!/usr/bin/env python

"""setup that compiles C++ wrapper to be used as python module.
"""

from distutils.core import setup, Extension
import os

os.environ["CC"] = "g++-5 -std=c++11"

regions= Extension('_regions',
                    sources=['regions_wrap.cxx', 'regions.cpp'])

setup (name = 'regions',
       version = '0.1',
       author      = "JRO",
       description = """C++ wrappers for performance optimization.""",
       ext_modules = [regions],
       py_modules = ["regions"],
       )
