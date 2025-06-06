# this cmake file is for compile tests.
# Not usable for your cmake app.
cmake_minimum_required(VERSION 3.5)

set(BUILD_TARGET "tinyexr")
set(SAMPLE_TARGET "test_tinyexr")

project(${BUILD_TARGET} CXX)

# options
option(TINYEXR_BUILD_SAMPLE "Build a sample" ON)
option(TINYEXR_USE_MINIZ "Use miniz" ON)

# cmake modules
list(APPEND CMAKE_MODULE_PATH ${PROJECT_SOURCE_DIR}/cmake)
list(APPEND CMAKE_MODULE_PATH ${PROJECT_SOURCE_DIR}/cmake/sanitizers)
find_package(Sanitizers) # Address sanitizer (-DSANITIZE_ADDRESS=ON)

# Require strict C++11 mode(e.g. `-std=c++11`)
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set_property(GLOBAL PROPERTY USE_FOLDERS ON)

set(TINYEXR_SOURCES
    ${PROJECT_SOURCE_DIR}/tinyexr.cc
    )

if(TINYEXR_USE_MINIZ)
  enable_language(C)
  add_library(miniz STATIC deps/miniz/miniz.c)
  target_include_directories(miniz PUBLIC deps/miniz)
  set_target_properties(miniz PROPERTIES FOLDER "deps")
  list(APPEND TINYEXR_EXT_LIBRARIES miniz)
endif()

add_library(${BUILD_TARGET} ${TINYEXR_SOURCES} ${TINYEXR_DEP_SOURCES})
add_sanitizers(${BUILD_TARGET})

target_include_directories(${BUILD_TARGET} PUBLIC ${PROJECT_SOURCE_DIR})
target_link_libraries(${BUILD_TARGET} ${TINYEXR_EXT_LIBRARIES} ${CMAKE_DL_LIBS})

# Increase warning level for clang.
if (CMAKE_CXX_COMPILER_ID MATCHES "Clang" AND NOT MSVC)
  set(CLANG_COMPILE_FLAGS "-Weverything -Werror -Wno-padded -Wno-c++98-compat-pedantic -Wno-documentation -Wno-unused-member-function")
  if (${CMAKE_CXX_COMPILER_VERSION} VERSION_GREATER 16)
    set(CLANG_COMPILE_FLAGS "${CLANG_COMPILE_FLAGS} -Wno-unsafe-buffer-usage")
  endif()
  set_source_files_properties(${TINYEXR_SOURCES} PROPERTIES COMPILE_FLAGS ${CLANG_COMPILE_FLAGS})
endif ()

if (TINYEXR_BUILD_SAMPLE)

  set(TINYEXR_SAMPLE_SOURCES
      ${PROJECT_SOURCE_DIR}/test_tinyexr.cc
      )

  add_executable(${SAMPLE_TARGET} ${TINYEXR_SAMPLE_SOURCES})
  target_link_libraries(${SAMPLE_TARGET} ${TINYEXR_EXT_LIBRARIES})
  add_sanitizers(${SAMPLE_TARGET})
  if (WIN32)
    target_compile_definitions(${SAMPLE_TARGET} PUBLIC UNICODE)
    target_compile_definitions(${SAMPLE_TARGET} PUBLIC _UNICODE)

    # Set ${SAMPLE_TARGET} as a startup project for VS IDE
    set_property(DIRECTORY PROPERTY VS_STARTUP_PROJECT ${SAMPLE_TARGET})

    # For easier debugging in VS IDE(cmake 3.8.0 or later required) Set working
    # directory to ${BUILD_TARGET} git repo root.
    if(CMAKE_VERSION VERSION_GREATER 3.8.0)
      set_target_properties(${SAMPLE_TARGET}
              PROPERTIES VS_DEBUGGER_WORKING_DIRECTORY
              "${CMAKE_CURRENT_SOURCE_DIR}")
    endif()
  endif(WIN32)

endif (TINYEXR_BUILD_SAMPLE)
