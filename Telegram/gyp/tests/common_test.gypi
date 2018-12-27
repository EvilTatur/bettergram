# This file is part of Bettergram.
#
# For license and copyright information please follow this link:
# https://github.com/bettergram/bettergram/blob/master/LEGAL

{
  'includes': [
    '../common_executable.gypi',
    '../qt.gypi',
  ],
  'include_dirs': [
    '<(src_loc)',
    '<(submodules_loc)/GSL/include',
    '<(submodules_loc)/variant/include',
    '<(submodules_loc)/Catch/include',
    '<(submodules_loc)/crl/src',
    '<(libs_loc)/range-v3/include',
  ],
  'sources': [
    '<(src_loc)/base/tests_main.cpp',
  ],
}
