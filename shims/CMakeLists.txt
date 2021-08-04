
add_library(slbc_linker SHARED slbc_mmb_linker.c slbc_mmb_linker.h linker.c linker.h)
add_library(vesmux_linker SHARED vesmux_linker.c vesmux_linker.h linker.c linker.h)
add_library(lp_linker SHARED lp_linker.c lp_linker.h linker.c linker.h)

target_link_libraries( slbc_linker PUBLIC dl)
target_link_libraries( vesmux_linker PUBLIC dl)
target_link_libraries( lp_linker PUBLIC dl)
target_compile_definitions( lp_linker PUBLIC SHARED_LP_LIB)

