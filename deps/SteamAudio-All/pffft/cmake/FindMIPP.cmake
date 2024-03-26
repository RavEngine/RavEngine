
if(MIPP_INCLUDE_DIRS)
  set(MIPP_FIND_QUIETLY TRUE)
endif()

find_path(MIPP_INCLUDE_DIRS NAMES mipp.h
    HINTS
        ${MIPP_ROOT}
        $ENV{HOME}/.local
    PATH_SUFFIXES include/mipp
)

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(MIPP DEFAULT_MSG MIPP_INCLUDE_DIRS)

if(MIPP_FOUND AND NOT TARGET MIPP)
    message(STATUS "MIPP_FOUND -> creating interface library MIPP at ${MIPP_INCLUDE_DIRS}")
    add_library(MIPP INTERFACE)
    target_compile_definitions(MIPP INTERFACE HAVE_MIPP=1)
    target_include_directories(MIPP INTERFACE ${MIPP_INCLUDE_DIRS})
    target_compile_features(MIPP INTERFACE cxx_std_11)
else()
    message(WARNING "MIPP not found.")
endif()

mark_as_advanced(MIPP_INCLUDE_DIRS)
