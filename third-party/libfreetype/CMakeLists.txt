cmake_minimum_required(VERSION 2.8)
project(libfreetype)

find_package(redwood_catkin REQUIRED)
add_definitions(-DFT2_BUILD_LIBRARY)

catkin_simple()

cs_add_library(freetype
  src/base/ftbbox.c
  src/base/ftbitmap.c
  src/base/ftglyph.c
  src/base/ftstroke.c
  src/base/ftxf86.c
  src/base/ftbase.c
  src/base/ftsystem.c
  src/base/ftinit.c
  src/base/ftgasp.c
  src/raster/raster.c
  src/sfnt/sfnt.c
  src/smooth/smooth.c
  src/autofit/autofit.c
  src/truetype/truetype.c
  src/cff/cff.c
  src/psnames/psnames.c
  src/pshinter/pshinter.c
  )

cs_install()
cs_export()
redwood_generate_ndk_import_module(${PROJECT_NAME})
