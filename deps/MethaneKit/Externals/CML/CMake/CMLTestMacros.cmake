# -*- cmake -*- -----------------------------------------------------------
# @@COPYRIGHT@@
#*-------------------------------------------------------------------------

# Macro to add a single-file test to the build, using ${_Name}.cpp as the
# test source.  The executable name and test title will be set to
# ${_Name}_test.
macro(CML_ADD_TEST
    _Name			# The test basename, e.g. my -> my_test
    )

  # Define the executable name:
  set(ExecName "${_Name}")

  # Define the test name:
  if(DEFINED CML_CURRENT_TEST_GROUP)
    set(TestName "CML:${CML_CURRENT_TEST_GROUP}:${_Name}")
  else()
    message(FATAL_ERROR "CML_CURRENT_TEST_GROUP must be defined")
  endif()

  # Setup the build target:
  add_executable(${ExecName} ${_Name}.cpp)
  set_target_properties(${ExecName} PROPERTIES
    FOLDER "CML-Tests/${CML_CURRENT_TEST_GROUP}")
  target_link_libraries(${ExecName} cml cml_test_main)

  # Setup the test:
  add_test(NAME ${TestName} COMMAND ${ExecName})
endmacro()

# --------------------------------------------------------------------------
# vim:ft=cmake
