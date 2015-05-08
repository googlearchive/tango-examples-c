cmake_minimum_required(VERSION 2.8)
project(libpng)

find_package(redwood_catkin REQUIRED)

catkin_simple()

cs_add_library(png
  png.c
  pngerror.c
  pngget.c
  pngmem.c
  pngpread.c
  pngread.c
  pngrio.c
  pngrtran.c
  pngrutil.c
  pngset.c
  pngtrans.c
  pngwio.c
  pngwrite.c
  pngwtran.c
  pngwutil.c
  )

file(GLOB LIBPNG_HEADERS "include/*.h")
file(COPY ${LIBPNG_HEADERS} DESTINATION ${CATKIN_DEVEL_PREFIX}/include)
message("Copying headers to ${CATKIN_DEVEL_PREFIX}/include")

cs_install()
cs_export()

# Install NDK import module, we need to populate our Android.mk template and
# place it in the correct location.
configure_file(
  ${CMAKE_CURRENT_SOURCE_DIR}/templates/Android.mk.in
  ${CMAKE_CURRENT_BINARY_DIR}/Android.mk @ONLY)

file(INSTALL ${CMAKE_CURRENT_BINARY_DIR}/Android.mk
     DESTINATION ${CATKIN_DEVEL_PREFIX}/${REDWOOD_PACKAGE_NDK_DESTINATION})
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/Android.mk
     DESTINATION ${REDWOOD_PACKAGE_NDK_DESTINATION})
