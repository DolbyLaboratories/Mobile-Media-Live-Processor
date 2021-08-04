project(gst_mmb)
cmake_minimum_required (VERSION 3.9)
set(CMAKE_CXX_STANDARD 17)

set(CMAKE_C_FLAGS_RELEASE "${CMAKE_C_FLAGS_RELEASE} -s")
set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -s")

find_package(PkgConfig REQUIRED)
pkg_check_modules(GSTREAMER REQUIRED IMPORTED_TARGET gstreamer-1.0 gstreamer-base-1.0 gstreamer-video-1.0)
find_package(Threads REQUIRED)

set(IDIRS ${GST_INCLUDE_DIRS} include shims)
set(LDIRS ${GST_LIBRARY_DIRS} )

link_directories( ${LDIRS} )
include_directories( ${IDIRS})

add_subdirectory(shims)

add_library( gstdlbrpumeta SHARED src/gstmetavisionrpu.c src/gstmetavisionrpu.h )
target_include_directories( gstdlbrpumeta PRIVATE ${IDIRS})
target_link_libraries( gstdlbrpumeta PUBLIC PkgConfig::GSTREAMER )

add_library( gstdlbmapper SHARED src/gstdlbmapper.c src/gstdlbmapper.h src/gstmetavisionrpu.h )
target_include_directories(gstdlbmapper PRIVATE src/shim_linker)
target_link_libraries( gstdlbmapper PUBLIC PkgConfig::GSTREAMER gstdlbrpumeta slbc_linker)

add_library( gstdlbrpux SHARED src/gstdlbrpuextract.c src/gstdlbrpuextract.h src/gstmetavisionrpu.h )
target_include_directories( gstdlbrpux PRIVATE ${IDIRS} src/shim_linker )
target_link_libraries( gstdlbrpux PUBLIC PkgConfig::GSTREAMER gstdlbrpumeta lp_linker stdc++ )

add_library( gstdlbmux SHARED src/gstdlbmux.c src/gstdlbmux.h src/gstmetavisionrpu.h)
target_include_directories( gstdlbmux PRIVATE ${IDIRS} src/shim_linker )
target_link_libraries( gstdlbmux PUBLIC PkgConfig::GSTREAMER vesmux_linker gstdlbrpumeta)

