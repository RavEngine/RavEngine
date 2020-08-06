include(ExternalProject)

# provide configuration options to choose which of the dependencies to build
if (WIN32)
  option(OGREDEPS_BUILD_AMD_QBS "Build AMD Quad Buffer SDK dependency" TRUE)
  option(OGREDEPS_BUILD_AMD_AGS "Build AMD AGS" TRUE)
  option(OGREDEPS_BUILD_NVIDIA_NVAPI "Build NVIDIA NVAPI dependency" TRUE)
  option(OGREDEPS_BUILD_SDL2 "Build SDL2 dependency" TRUE)
endif ()
option(OGREDEPS_BUILD_FREEIMAGE "Build FreeImage dependency" TRUE)
option(OGREDEPS_BUILD_FREETYPE "Build FreeType dependency" TRUE)
option(OGREDEPS_BUILD_OPENVR "Install prebuilt OpenVR dependency" TRUE)
option(OGREDEPS_BUILD_ZLIB "Build zlib dependency" TRUE)
option(OGREDEPS_BUILD_ZZIPLIB "Build zziplib dependency" TRUE)
option(OGREDEPS_BUILD_RAPIDJSON "Build rapidjson dependency" TRUE)
if( NOT ANDROID )
	option(OGREDEPS_BUILD_REMOTERY "Build Remotery dependency" TRUE)
endif()
option(OGREDEPS_INSTALL_DEV "Install development headers and libraries" TRUE)
set(OGREDEPS_LIB_DIRECTORY "lib${LIB_SUFFIX}" CACHE STRING
  "Install path for libraries, e.g. 'lib64' on some 64-bit Linux distros.")
mark_as_advanced(OGREDEPS_LIB_DIRECTORY)

# choose paths to build and install dependencies to
if (NOT OGREDEPS_RUNTIME_OUTPUT)
  set(OGREDEPS_RUNTIME_OUTPUT ${OGREDEPS_BINARY_DIR}/bin)
endif ()
if (WIN32 OR (APPLE AND NOT OGRE_BUILD_PLATFORM_APPLE_IOS))
  set(OGREDEPS_RELEASE_PATH "/Release")
  set(OGREDEPS_DEBUG_PATH "/Debug")
else ()
  set(OGREDEPS_RELEASE_PATH "")
  set(OGREDEPS_DEBUG_PATH "")
endif ()

# postfix debug libraries with a _d
if (NOT APPLE)
set(CMAKE_DEBUG_POSTFIX "_d")
endif()

if (OGRE_BUILD_PLATFORM_APPLE_IOS)
#set(OGREDEPS_BUILD_ZLIB FALSE)
endif()

# determine if we are compiling for a 32bit or 64bit system
include(CheckTypeSize)
CHECK_TYPE_SIZE("void*" OGREDEPS_PTR_SIZE BUILTIN_TYPES_ONLY)
if (OGREDEPS_PTR_SIZE EQUAL 8)
  set(OGREDEPS_PLATFORM_X64 TRUE)
else ()
  set(OGREDEPS_PLATFORM_X64 FALSE)
endif ()

if (CMAKE_CXX_COMPILER_ID MATCHES "Clang")
  set(CMAKE_COMPILER_IS_CLANGXX 1)
endif ()

# Add -fPIC flag to x64 GCC builds
if ((CMAKE_COMPILER_IS_GNUCXX OR CMAKE_COMPILER_IS_CLANGXX) AND NOT MINGW)
  if (OGREDEPS_PLATFORM_X64 AND NOT APPLE)
    add_definitions(-fPIC)
  endif ()
endif ()

if(XCODE_VERSION VERSION_LESS 5.0.0)
  set(MIN_IOS_VERSION "5.0")
else()
  set(MIN_IOS_VERSION "6.0")
endif()

macro(set_xcode_property targ xc_prop_name xc_prop_val)
  set_property( TARGET ${targ} PROPERTY XCODE_ATTRIBUTE_${xc_prop_name} ${xc_prop_val} )
endmacro(set_xcode_property)

# by default, install to ogredeps subfolder in build directory
if (CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT)
  if (OGREDEPS_SOURCE_DIR STREQUAL CMAKE_SOURCE_DIR)
      set(CMAKE_INSTALL_PREFIX "${OGREDEPS_BINARY_DIR}/ogredeps" CACHE PATH "OGREDEPS install prefix" FORCE)
  endif ()
endif ()

# enforce that CMAKE_BUILD_TYPE is set, otherwise install commands
# will not work for Makefile based generators.
if (NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE Release CACHE STRING "Choose the type of build, options are: Debug, Release" FORCE)
endif ()

# configure and install dependency LIBRARY
# INCLUDE_DIR specifies where header files need to be installed
# Any additional parameters are treated as a list of header files
# to install.
macro(install_dep LIBRARY INCLUDE_DIR)
  set_target_properties(${LIBRARY} PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${OGREDEPS_RUNTIME_OUTPUT}
  )

  if(APPLE AND OGRE_BUILD_PLATFORM_APPLE_IOS)
    set_xcode_property( ${LIBRARY} IPHONEOS_DEPLOYMENT_TARGET ${MIN_IOS_VERSION} )
    set_property( TARGET ${LIBRARY} PROPERTY XCODE_ATTRIBUTE_IPHONEOS_DEPLOYMENT_TARGET[arch=arm64] "7.0" )

    add_custom_command(TARGET ${LIBRARY} POST_BUILD
      COMMAND mkdir -p "${OGREDEPS_BINARY_DIR}/${OGREDEPS_LIB_DIRECTORY}/$(CONFIGURATION)"
      COMMAND mkdir -p "${CMAKE_INSTALL_PREFIX}/${OGREDEPS_LIB_DIRECTORY}/$(CONFIGURATION)"
      COMMAND if [ -e ${OGREDEPS_BINARY_DIR}/src/${LIBRARY}/$(CONFIGURATION)-iphonesimulator/lib${LIBRARY}.a ] && [ -e ${OGREDEPS_BINARY_DIR}/src/${LIBRARY}/$(CONFIGURATION)-iphoneos/lib${LIBRARY}.a ] \; then lipo ARGS -create ${OGREDEPS_BINARY_DIR}/src/${LIBRARY}/$(CONFIGURATION)-iphoneos/lib${LIBRARY}.a ${OGREDEPS_BINARY_DIR}/src/${LIBRARY}/$(CONFIGURATION)-iphonesimulator/lib${LIBRARY}.a
      -output "${CMAKE_INSTALL_PREFIX}/${OGREDEPS_LIB_DIRECTORY}/$(CONFIGURATION)/lib${LIBRARY}.a" \; fi
    )
  endif()

  # always install runtime libraries (i.e. DLL, so)
  install(TARGETS ${LIBRARY}
    RUNTIME DESTINATION "bin${OGREDEPS_RELEASE_PATH}" CONFIGURATIONS Release
    LIBRARY DESTINATION "${OGREDEPS_LIB_DIRECTORY}${OGREDEPS_RELEASE_PATH}" CONFIGURATIONS Release
    ARCHIVE DESTINATION "${OGREDEPS_BINARY_DIR}/notwanted" CONFIGURATIONS Release
  )
  install(TARGETS ${LIBRARY}
    RUNTIME DESTINATION "bin${OGREDEPS_DEBUG_PATH}" CONFIGURATIONS Debug
    LIBRARY DESTINATION "${OGREDEPS_LIB_DIRECTORY}${OGREDEPS_DEBUG_PATH}" CONFIGURATIONS Debug
    ARCHIVE DESTINATION "${OGREDEPS_BINARY_DIR}/notwanted" CONFIGURATIONS Debug
  )

  if (OGREDEPS_INSTALL_DEV)
    # also install development parts
    install(TARGETS ${LIBRARY}
      RUNTIME DESTINATION "bin${OGREDEPS_RELEASE_PATH}" CONFIGURATIONS Release
      LIBRARY DESTINATION "${OGREDEPS_LIB_DIRECTORY}${OGREDEPS_RELEASE_PATH}" CONFIGURATIONS Release
      ARCHIVE DESTINATION "${OGREDEPS_LIB_DIRECTORY}${OGREDEPS_RELEASE_PATH}" CONFIGURATIONS Release
    )
    install(TARGETS ${LIBRARY}
      RUNTIME DESTINATION "bin${OGREDEPS_DEBUG_PATH}" CONFIGURATIONS Debug
      LIBRARY DESTINATION "${OGREDEPS_LIB_DIRECTORY}${OGREDEPS_DEBUG_PATH}" CONFIGURATIONS Debug
      ARCHIVE DESTINATION "${OGREDEPS_LIB_DIRECTORY}${OGREDEPS_DEBUG_PATH}" CONFIGURATIONS Debug
    )
    install(FILES ${ARGN} DESTINATION ${INCLUDE_DIR})
  endif ()
endmacro()

# setup the individual dependencies
if (OGREDEPS_BUILD_AMD_QBS)
  add_subdirectory(AMD_Quad_Buffer_SDK_v11)
endif ()
if (OGREDEPS_BUILD_AMD_AGS)
  add_subdirectory(AMD_AGS)
endif ()
if (OGREDEPS_BUILD_NVIDIA_NVAPI)
  add_subdirectory(NVAPI-R313-developer)
endif ()
if (OGREDEPS_BUILD_FREEIMAGE)
  add_subdirectory(FreeImage)
endif ()
if (OGREDEPS_BUILD_FREETYPE)
  add_subdirectory(freetype)
endif ()
if (OGREDEPS_BUILD_OPENVR)
  add_subdirectory(openvr)
endif ()
if (OGREDEPS_BUILD_RAPIDJSON)
  add_subdirectory(rapidjson)
endif ()
if (OGREDEPS_BUILD_REMOTERY)
  add_subdirectory(Remotery)
endif ()
if (OGREDEPS_BUILD_SDL2)
  set(SDL2_CMAKE_ARGS "-DCMAKE_INSTALL_PREFIX:PATH=${CMAKE_INSTALL_PREFIX}" "-DSDL_CMAKE_DEBUG_POSTFIX:STRING=_d")
  if (MINGW)
    list(APPEND SDL2_CMAKE_ARGS "-DDIRECTX=0")
    set(SDL2_BIN_NAME "libSDL2.dll")
    # set(SDL2_LIB_NAME "libSDL2.a")
  else ()
    set(SDL2_BIN_NAME "SDL2.dll")
    set(SDL2_LIB_NAME "SDL2.lib")
  endif ()
  # Always build SDL2 in Release mode
  ExternalProject_Add(
    SDL2
    # Configure step.
    SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/SDL2
    CMAKE_ARGS ${SDL2_CMAKE_ARGS}
       BUILD_COMMAND ""
   INSTALL_COMMAND
        ${CMAKE_COMMAND}
        --build .
        --target install
        --config Release
    BUILD_ALWAYS 1
  )
  ExternalProject_Add_Step(
    SDL2
    MoveInstallFiles
    COMMENT "Move SDL2 install files into the Release and Debug folders."
    DEPENDEES install
    WORKING_DIRECTORY ${CMAKE_INSTALL_PREFIX}
    # For MinGW, the Release & Debug directories may not yet exist.
    COMMAND ${CMAKE_COMMAND} -E make_directory bin/Release
    COMMAND ${CMAKE_COMMAND} -E make_directory bin/Debug
    COMMAND ${CMAKE_COMMAND} -E rename bin/${SDL2_BIN_NAME} bin/Debug/${SDL2_BIN_NAME}
    COMMAND ${CMAKE_COMMAND} -E copy bin/Debug/${SDL2_BIN_NAME} bin/Release/${SDL2_BIN_NAME}
    LOG 1
  )
endif ()
if (OGREDEPS_BUILD_ZLIB)
  add_subdirectory(zlib)
endif ()
if (OGREDEPS_BUILD_ZZIPLIB)
  add_subdirectory(zziplib)
endif ()
