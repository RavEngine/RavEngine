project(AMD_QBS)

set(AMD_QBS_INCLUDE_DIR "${AMD_QBS_SOURCE_DIR}/include" CACHE PATH "")
mark_as_advanced(AMD_QBS_INCLUDE_DIR)

if (OGREDEPS_INSTALL_DEV)
  install(DIRECTORY include/ DESTINATION include)
endif ()
