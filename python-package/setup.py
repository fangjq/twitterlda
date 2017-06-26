# -*- coding: utf-8 -*- #
# Author: Jiaquan Fang

from __future__ import absolute_import
import sys
import os
from setuptools import setup, find_packages
sys.path.insert(0, '.')

CURRENT_DIR = os.path.dirname(__file__)

libpath_py = os.path.join(CURRENT_DIR, 'twitterlda/libpath.py')
libpath = {'__file__': libpath_py}

exec(compile(open(libpath_py, "rb").read(), libpath_py, 'exec'), libpath, libpath)

LIB_PATH = [os.path.relpath(libfile, CURRENT_DIR) for libfile in libpath['find_lib_path']()]

print "Install twitterlda from: %s" % LIB_PATH

setup(name='twitterlda',
      version=open(os.path.join(CURRENT_DIR, 'twitterlda/VERSION')).read().strip(),
      description="twitterlda",
      long_description=open(os.path.join(CURRENT_DIR, 'README.rst')).read(),
      install_requires=[
          'numpy',
      ],
      maintainer='Jiaquan Fang',
      maintainer_email='jiaquanfang@gmail.com',
      packages=find_packages(),
      include_package_data=True,
      data_files=[('twitterlda', LIB_PATH)],
      license="Apache-2.0"
)
