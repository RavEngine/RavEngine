
cmake_minimum_required(VERSION 3.13)
include_guard()

project(LabSound)

set(LABSOUND_ROOT "${CMAKE_CURRENT_SOURCE_DIR}")

# default backend set up

if (WIN32)
elseif (APPLE)
elseif (ANDROID)
else()
    if (NOT LABSOUND_JACK AND NOT LABSOUND_PULSE AND NOT LABSOUND_ASOUND)
        #message(STATUS "No Linux backend specified, defaulting to Pulse.")
        #set(LABSOUND_PULSE ON)
    endif()
endif()

# if (LABSOUND_ASOUND)
#     set(LIBNYQUIST_ASOUND ON)
# elseif (LABSOUND_JACK)
#     set(LIBNYQUIST_JACK ON)
# elseif (LABSOUND_PULSE)
#     set(LIBNYQUIST_PULSE ON)
# endif()

set(CMAKE_CXX_STANDARD 14)

# turn off building libnyquist samples
set(BUILD_EXAMPLE OFF CACHE INTERNAL "")
add_subdirectory(third_party/libnyquist)

include(cmake/LabSound.cmake)

set_property(GLOBAL PROPERTY USE_FOLDERS ON)

if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE "Release")
endif()

add_definitions(${_LAB_CXX_DEFINITIONS})
set(CMAKE_CXX_FLAGS "${_LAB_CXX_FLAGS} ${CMAKE_CXX_FLAGS}")
