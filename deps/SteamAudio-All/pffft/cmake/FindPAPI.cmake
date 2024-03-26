# Find PAPI libraries
# Once done this will define
#  PAPI_FOUND - System has PAPI
#  PAPI_INCLUDE_DIRS - The PAPI include directories
#  PAPI_LIBRARIES - The libraries needed to use PAPI

if(PAPI_INCLUDE_DIRS AND PAPI_LIBRARIES)
  set(PAPI_FIND_QUIETLY TRUE)
endif()

find_path(PAPI_INCLUDE_DIRS NAMES papi.h HINTS ${PAPI_ROOT} PATH_SUFFIXES include)
find_library(PAPI_LIBRARIES NAMES papi HINTS ${PAPI_ROOT} PATH_SUFFIXES lib lib64)

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(PAPI DEFAULT_MSG PAPI_LIBRARIES PAPI_INCLUDE_DIRS)
if(PAPI_FOUND AND NOT TARGET PAPI::PAPI)
    set(PAPI_LIBRARIES ${PAPI_LIBRARIES} rt)

    add_library(PAPI::PAPI SHARED IMPORTED)
    set_target_properties(PAPI::PAPI PROPERTIES
        INTERFACE_INCLUDE_DIRECTORIES "${PAPI_INCLUDE_DIRS}"
        IMPORTED_LOCATION "${PAPI_LIBRARIES}")
endif()

mark_as_advanced(PAPI_INCLUDE_DIRS PAPI_LIBRARIES)
