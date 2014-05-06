import os
import sys
from os import unlink, symlink, popen, uname, environ

def set_options(opt):
  opt.tool_options("compiler_cxx")

def configure(conf):
  conf.check_tool("compiler_cxx")
  conf.check_tool("node_addon")

  linkflags = popen("gdal-config --libs").readline().strip().split(' ')
  cflags = popen("gdal-config --cflags").readline().strip().split(' ')

  cflags.append("-g")
  cflags.append("-D_FILE_OFFSET_BITS=64")
  cflags.append("-D_LARGEFILE_SOURCE")
  cflags.append("-Wall")

  conf.env.append_value("LINKFLAGS", linkflags)
  conf.env.append_value("CFLAGS", cflags)
  conf.env.append_value("CXXFLAGS", cflags)

def build(bld):
  obj = bld.new_task_gen("cxx", "shlib", "node_addon")
  obj.target = "gdal"
  obj.source = [
    "src/gdal_driver.cpp",
    "src/gdal_dataset.cpp",
    "src/gdal_rasterband.cpp",
    "src/node_gdal.cpp"
  ]
