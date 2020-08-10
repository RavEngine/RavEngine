# -*- cmake -*- -----------------------------------------------------------
# @@COPYRIGHT@@
#*-------------------------------------------------------------------------

project(CMLTestMain)

add_library(cml_test_main STATIC catch_main.cpp catch.hpp catch_runner.h)
target_include_directories(cml_test_main
  PUBLIC ${CMAKE_CURRENT_LIST_DIR}
  PRIVATE ${CML_ROOT}
  )
set_target_properties(cml_test_main PROPERTIES FOLDER "CML-Tests")

# --------------------------------------------------------------------------
# vim:ft=cmake
