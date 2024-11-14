project(libDDS)

add_library(libDDS dds.c include/dds/dds.h)
target_compile_features(libDDS PRIVATE c_std_99)
target_include_directories(libDDS PUBLIC "${CMAKE_CURRENT_LIST_DIR}/include")