#!/usr/bin/env python

# Copyright (C) 2008 Brian Quinlan
#
# This library is free software: you can redistribute it and/or
# modify it under the terms of the GNU Lesser General Public License
# as published by the Free Software Foundation, either version 3 of
# the License, or (at your option) any later version.
#
# This library is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
# Lesser General Public License for more details.
#
# You should have received a copy of the GNU Lesser General Public
# License along with this library.  If not, see
# <http://www.gnu.org/licenses/>.

"""Installs the Python bindings for libopenraw using distutils.

Run:
    python setup.py install
"""

__author__ = 'Brian Quinlan <brian@sweetapp.com>'

from distutils.core import setup
from distutils.extension import Extension

# TODO(bquinlan): Remove this include directive when libopenraw installs it's
# C++ headers to a system include directory.
include_directories = ['../include/']

libraries = ['boost_python', 'openraw']
c_source_files = ['src/_openraw.cpp', 'src/pythonrawfile.cpp']

setup(name='openraw',
      version='0.05',
      description='Python bindings for libopenraw',
      author='Brian Quinlan',
      author_email='brian@sweetapp.com',
      url='http://libopenraw.freedesktop.org/wiki/',
      license='LGPL 3',
      platforms='Python 2.2+',
      packages=['openraw'],
      ext_modules = [
          Extension('_openraw',
                    c_source_files,
                    include_dirs=include_directories,
                    libraries=libraries)]) 
