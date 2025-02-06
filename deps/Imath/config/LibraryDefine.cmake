# SPDX-License-Identifier: BSD-3-Clause
# Copyright Contributors to the OpenEXR Project.

include(CheckLibraryExists)
check_library_exists(m sin "" HAVE_LIB_M)
if (HAVE_LIB_M)
  set(IMATH_EXTRA_LIBS m)
endif()

# NB: This function has a number if Imath specific names / variables
# in it, so be careful copying...
function(IMATH_DEFINE_LIBRARY libname)
  set(options)
  set(oneValueArgs PRIV_EXPORT CURDIR CURBINDIR)
  set(multiValueArgs SOURCES HEADERS DEPENDENCIES PRIVATE_DEPS)
  cmake_parse_arguments(IMATH_CURLIB "${options}" "${oneValueArgs}" "${multiValueArgs}" ${ARGN})

  if (MSVC)
    set(_imath_extra_flags "/EHsc")
  endif()
  set(objlib ${libname})
  add_library(${objlib}
    ${IMATH_CURLIB_HEADERS}
    ${IMATH_CURLIB_SOURCES})

  # Use ${IMATH_CXX_STANDARD} to determine the standard we use to compile
  # Imath itself. But the headers only require C++11 features, so that's
  # all we need to pass on as interface reqirements to downstream projects.
  # For example, it's fine for an Imath built with C++14 to be called from
  # an app that is compiled with C++11; Imath needn't force the app to
  # also use C++14.
  target_compile_features(${objlib}
                          PRIVATE cxx_std_${IMATH_CXX_STANDARD}
                          INTERFACE cxx_std_11 )

  if(IMATH_CURLIB_PRIV_EXPORT AND BUILD_SHARED_LIBS)
    target_compile_definitions(${objlib} PRIVATE ${IMATH_CURLIB_PRIV_EXPORT})
    if(WIN32)
      target_compile_definitions(${objlib} PUBLIC IMATH_DLL)
    endif()
  endif()
  if(IMATH_CURLIB_CURDIR)
    target_include_directories(${objlib} INTERFACE $<BUILD_INTERFACE:${IMATH_CURLIB_CURDIR}>)
  endif()
  if(IMATH_CURLIB_CURBINDIR)
    target_include_directories(${objlib} PRIVATE $<BUILD_INTERFACE:${IMATH_CURLIB_CURBINDIR}>)
  endif()
  target_link_libraries(${objlib} PUBLIC ${PROJECT_NAME}::Config ${IMATH_CURLIB_DEPENDENCIES})
  if(IMATH_CURLIB_PRIVATE_DEPS)
    target_link_libraries(${objlib} PRIVATE ${IMATH_CURLIB_PRIVATE_DEPS})
  endif()
  set_target_properties(${objlib} PROPERTIES
    CXX_STANDARD_REQUIRED ON
    CXX_EXTENSIONS OFF
    POSITION_INDEPENDENT_CODE ON
    )
  if (NOT IMATH_USE_DEFAULT_VISIBILITY)
    set_target_properties(${objlib} PROPERTIES
      C_VISIBILITY_PRESET hidden
      CXX_VISIBILITY_PRESET hidden
      VISIBILITY_INLINES_HIDDEN ON
      )
  else()
      target_compile_definitions(${objlib} PUBLIC IMATH_USE_DEFAULT_VISIBILITY)
  endif()
  if (_imath_extra_flags)
    target_compile_options(${objlib} PUBLIC ${_imath_extra_flags})
  endif()
  set_property(TARGET ${objlib} PROPERTY PUBLIC_HEADER ${IMATH_CURLIB_HEADERS})

  if(BUILD_SHARED_LIBS)
    set_target_properties(${libname} PROPERTIES
      SOVERSION ${IMATH_LIB_SOVERSION}
      VERSION ${IMATH_LIB_VERSION}
    )
  endif()
  if(IMATH_EXTRA_LIBS)
    target_link_libraries(${libname} PUBLIC ${IMATH_EXTRA_LIBS})
  endif()

  set_target_properties(${libname} PROPERTIES
      OUTPUT_NAME "${libname}${IMATH_LIB_SUFFIX}"
      RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin"
  )
  add_library(${PROJECT_NAME}::${libname} ALIAS ${libname})

  if (IMATH_INSTALL)
    
    install(TARGETS ${libname}
      EXPORT ${PROJECT_NAME}
      RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
      LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
      ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
      INCLUDES DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
      PUBLIC_HEADER
        DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/${IMATH_OUTPUT_SUBDIR}
    )
    if(BUILD_SHARED_LIBS AND (NOT "${IMATH_LIB_SUFFIX}" STREQUAL "") AND IMATH_INSTALL_SYM_LINK)
      string(TOUPPER "${CMAKE_BUILD_TYPE}" uppercase_CMAKE_BUILD_TYPE)
      set(verlibname ${CMAKE_SHARED_LIBRARY_PREFIX}${libname}${IMATH_LIB_SUFFIX}${CMAKE_${uppercase_CMAKE_BUILD_TYPE}_POSTFIX}${CMAKE_SHARED_LIBRARY_SUFFIX})
      set(baselibname ${CMAKE_SHARED_LIBRARY_PREFIX}${libname}${CMAKE_${uppercase_CMAKE_BUILD_TYPE}_POSTFIX}${CMAKE_SHARED_LIBRARY_SUFFIX})
      if(WIN32)
        install(CODE "execute_process(COMMAND ${CMAKE_COMMAND} -E chdir \"\$ENV\{DESTDIR\}${CMAKE_INSTALL_FULL_BINDIR}\" ${CMAKE_COMMAND} -E create_symlink ${verlibname} ${baselibname})")
        install(CODE "message(STATUS \"Creating symlink ${CMAKE_INSTALL_FULL_BINDIR}/${baselibname} -> ${verlibname}\")")
      else()
        install(CODE "execute_process(COMMAND ${CMAKE_COMMAND} -E chdir \"\$ENV\{DESTDIR\}${CMAKE_INSTALL_FULL_LIBDIR}\" ${CMAKE_COMMAND} -E create_symlink ${verlibname} ${baselibname})")
        install(CODE "message(STATUS \"Creating symlink ${CMAKE_INSTALL_FULL_LIBDIR}/${baselibname} -> ${verlibname}\")")
      endif()
      set(verlibname)
      set(baselibname)
    endif()

  endif()
endfunction()
