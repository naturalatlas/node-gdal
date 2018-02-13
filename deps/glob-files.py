# -*- coding: utf-8 -*-
"""
collect files for gyp
workaround as "find" that comes with git on Windows seems to walk the whole directory
tree of the current drive (just sometimes)

argument "search pattern" according to
https://docs.python.org/2/library/glob.html

e.g.
"../src/*.cpp" => just look in "src" directory
"../src/*/*.cpp" => look in "src" directory and subdirectories
"""

import sys
import glob

if len(sys.argv) < 2:
  print('no argument')
  sys.exit(1)

for f in glob.glob(sys.argv[1]):
  # gyp needs either double backslashes (\\) or forward slashes (/) as path separator
  # go with forward slashes to be platform independent
  # as "print" outputs just single backslashes (\) on Windows
  f = f.replace('\\', '/')
  # use line feed as delimiter between file names to prevent problems with whitespaces in file names
  print(f)
