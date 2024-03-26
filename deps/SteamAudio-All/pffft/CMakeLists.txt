cmake_minimum_required(VERSION 2.8)
project(PRETTY_FAST_FFT)

# smaller library size?
option(PFFFT_USE_TYPE_FLOAT  "activate single precision 'float'?" ON)
option(PFFFT_USE_TYPE_DOUBLE "activate 'double' precision float?" ON)

# architecture/optimization options
option(PFFFT_USE_SIMD        "use SIMD (SSE/AVX/NEON/ALTIVEC) CPU features? - " ON)
option(PFFFT_USE_SCALAR_VECT "use 4-element vector scalar operations (if no other SIMD)" ON)

# what to install?
option(INSTALL_PFFFT      "install pffft to CMAKE_INSTALL_PREFIX?" ON)
option(INSTALL_PFDSP      "install pfdsp to CMAKE_INSTALL_PREFIX?" OFF)
option(INSTALL_PFFASTCONV "install pffastconv to CMAKE_INSTALL_PREFIX?" OFF)

# test options
option(PFFFT_USE_BENCH_FFTW   "use (system-installed) FFTW3 in fft benchmark?" OFF)
option(PFFFT_USE_BENCH_GREEN  "use Green FFT in fft benchmark? - if exists in subdir" ON)
option(PFFFT_USE_BENCH_KISS   "use KissFFT in fft benchmark? - if exists in subdir" ON)
option(PFFFT_USE_BENCH_POCKET "use PocketFFT in fft benchmark? - if exists in subdir" ON)
option(PFFFT_USE_BENCH_MKL    "use Intel MKL in fft benchmark? needs to be installed" OFF)
option(PFFFT_USE_FFTPACK      "compile and use FFTPACK in fft benchmark & validation?" ON)

option(PFFFT_USE_DEBUG_ASAN  "use GCC's address sanitizer?" OFF)

option(PFFFT_DISABLE_LINK_WITH_M "Disables linking with m library to build with clangCL from MSVC" OFF)

# C90 requires the gcc extensions for function attributes like always_inline
# C99 provides the function attributes: no gcc extensions required
set(CMAKE_C_STANDARD 99)
set(CMAKE_C_EXTENSIONS OFF)

set(CMAKE_CXX_STANDARD 98)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# populate what to install
set(INSTALL_TARGETS "")
set(INSTALL_HEADERS "")


if ( (NOT PFFFT_USE_TYPE_FLOAT) AND (NOT PFFFT_USE_TYPE_DOUBLE) )
  message(FATAL_ERROR "activate at least one of PFFFT_USE_TYPE_FLOAT or PFFFT_USE_TYPE_DOUBLE")
endif()

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake/")
include(cmake/target_optimizations.cmake)
include(cmake/compiler_warnings.cmake)
find_package(PAPI)
find_package(MIPP)
if (MIPP_FOUND)
# if (TARGET MIPP)
    message(STATUS "found MIPP")
else()
    message(STATUS "NOT found MIPP")
endif()


if (PFFFT_USE_DEBUG_ASAN)
  set(ASANLIB "asan")
else()
  set(ASANLIB "")
endif()

message(STATUS "INFO: CMAKE_C_COMPILER_ID is ${CMAKE_C_COMPILER_ID}")
message(STATUS "INFO: CMAKE_CXX_COMPILER_ID is ${CMAKE_CXX_COMPILER_ID}")
if (WIN32)
  message(STATUS "INFO: detected WIN32")
else()
  message(STATUS "INFO: NOT WIN32")
endif()
if (MINGW)
  message(STATUS "INFO: detected MINGW with compiler ${CMAKE_C_COMPILER_ID}")
else()
  message(STATUS "INFO: NOT MINGW")
endif()
if ( CMAKE_C_COMPILER_ID MATCHES "MSVC" )
  message(STATUS "INFO: detected MSVC with compiler ${CMAKE_C_COMPILER_ID}")
endif()


if (PFFFT_USE_BENCH_GREEN)
  if (EXISTS "${CMAKE_CURRENT_LIST_DIR}/greenffts/CMakeLists.txt")
    message(STATUS "found subdir greenffts")
    set(PATH_GREEN "${CMAKE_CURRENT_LIST_DIR}/greenffts")
    add_subdirectory( "${PATH_GREEN}" )
  else()
    message(WARNING "GreenFFT not found in subdir greenffts")
  endif()
endif()

if (PFFFT_USE_BENCH_KISS)
  # git submodule add https://github.com/hayguen/kissfft.git
  if (EXISTS "${CMAKE_CURRENT_LIST_DIR}/kissfft/CMakeLists.txt")
    message(STATUS "found subdir kissfft")
    set(PATH_KISS "${CMAKE_CURRENT_LIST_DIR}/kissfft")
    add_subdirectory( "${PATH_KISS}" )
  else()
    message(WARNING "KissFFT not found in subdir kissfft")
  endif()
endif()

if (PFFFT_USE_BENCH_POCKET)
  # git submodule add https://github.com/hayguen/pocketfft.git
  if (EXISTS "${CMAKE_CURRENT_LIST_DIR}/pocketfft/pocketfft_double.c")
    message(STATUS "found subdir pocketfft")
    set(PATH_POCKET "${CMAKE_CURRENT_LIST_DIR}/pocketfft")
    add_subdirectory( "${PATH_POCKET}" )
  else()
    message(WARNING "PocketFFT not found in subdir pocketfft")
  endif()
endif()


########################################################################
# select the release build type by default to get optimization flags
########################################################################
if(NOT CMAKE_BUILD_TYPE)
   set(CMAKE_BUILD_TYPE "Release")
   message(STATUS "Build type not specified: defaulting to release.")
endif(NOT CMAKE_BUILD_TYPE)

if ( CMAKE_C_COMPILER_ID MATCHES "MSVC" )
  # using Visual Studio C++
  message(STATUS "INFO: detected MSVC: will not link math lib m")
  set(MATHLIB "")

  add_definitions("/D_CRT_SECURE_NO_WARNINGS")

  set(MSVC_DISABLED_WARNINGS_LIST
      "C4996"
  )

else()
  if(PFFFT_DISABLE_LINK_WITH_M)
  else()
    message(STATUS "INFO: detected NO MSVC: ${CMAKE_C_COMPILER_ID}: will link math lib m")
    set(MATHLIB "m")
  endif()
endif()

set(STDCXXLIB "")
if (MINGW)
  set(STDCXXLIB "stdc++")
endif()


set( SIMD_FLOAT_HDRS simd/pf_float.h simd/pf_sse1_float.h simd/pf_altivec_float.h simd/pf_neon_float.h simd/pf_scalar_float.h )
set( SIMD_DOUBLE_HDRS simd/pf_double.h simd/pf_avx_double.h simd/pf_scalar_double.h )

if (PFFFT_USE_TYPE_FLOAT)
  set( FLOAT_SOURCES pffft.c pffft.h ${SIMD_FLOAT_HDRS} )
  if (INSTALL_PFFFT)
    set(INSTALL_HEADERS ${INSTALL_HEADERS} pffft.h)
  endif()
else()
  set( FLOAT_SOURCES  )
endif()


if (PFFFT_USE_TYPE_DOUBLE)
  set( DOUBLE_SOURCES pffft_double.c pffft_double.h ${SIMD_DOUBLE_HDRS} )
  if (INSTALL_PFFFT)
    set(INSTALL_HEADERS ${INSTALL_HEADERS} pffft_double.h)
  endif()
else()
  set( DOUBLE_SOURCES )
endif()

######################################################

add_library(PFFFT STATIC ${FLOAT_SOURCES} ${DOUBLE_SOURCES} pffft_common.c pffft_priv_impl.h pffft.hpp )
set_target_properties(PFFFT PROPERTIES OUTPUT_NAME "pffft")
target_compile_definitions(PFFFT PRIVATE _USE_MATH_DEFINES)
target_activate_c_compiler_warnings(PFFFT)
if (PFFFT_USE_SCALAR_VECT)
  target_compile_definitions(PFFFT PRIVATE PFFFT_SCALVEC_ENABLED=1)
endif()
if (PFFFT_USE_DEBUG_ASAN)
  target_compile_options(PFFFT PRIVATE "-fsanitize=address")
endif()
target_set_c_arch_flags(PFFFT)
if (NOT PFFFT_USE_SIMD)
  target_compile_definitions(PFFFT PRIVATE PFFFT_SIMD_DISABLE=1)
endif()
target_link_libraries( PFFFT ${ASANLIB} ${MATHLIB} )
set_property(TARGET PFFFT APPEND PROPERTY INTERFACE_INCLUDE_DIRECTORIES
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
)
if (INSTALL_PFFFT)
  set(INSTALL_TARGETS ${INSTALL_TARGETS} PFFFT)
  set(INSTALL_HEADERS ${INSTALL_HEADERS} pffft.hpp)
endif()

######################################################

if (PFFFT_USE_TYPE_FLOAT)
  add_library(PFDSP STATIC pf_mixer.cpp pf_mixer.h pf_cplx.h pf_carrier.cpp pf_carrier.h pf_cic.cpp pf_cic.h fmv.h )
  set_property(TARGET PFDSP PROPERTY CXX_STANDARD 11)
  set_property(TARGET PFDSP PROPERTY CXX_STANDARD_REQUIRED ON)
  set_target_properties(PFDSP PROPERTIES OUTPUT_NAME "pfdsp")
  target_compile_definitions(PFDSP PRIVATE _USE_MATH_DEFINES)
  target_activate_cxx_compiler_warnings(PFDSP)
  if (PFFFT_USE_DEBUG_ASAN)
      target_compile_options(PFDSP PRIVATE "-fsanitize=address")
  endif()
  if (PFFFT_USE_SIMD)
      target_set_cxx_arch_flags(PFDSP)
  else()
      target_compile_definitions(PFDSP PRIVATE PFFFT_SIMD_DISABLE=1)
  endif()
  target_link_libraries( PFDSP ${MATHLIB} )
  set_property(TARGET PFDSP APPEND PROPERTY INTERFACE_INCLUDE_DIRECTORIES
      $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
  )
  if (INSTALL_PFDSP)
      set(INSTALL_TARGETS ${INSTALL_TARGETS} PFDSP)
      set(INSTALL_HEADERS ${INSTALL_HEADERS} pf_mixer.h pf_cplx.h pf_carrier.h pf_cic.h)
  endif()
endif()

######################################################

if (PFFFT_USE_FFTPACK)

  # float / single precision
  add_library(FFTPACK_FLOAT STATIC fftpack.c fftpack.h)
  target_compile_definitions(FFTPACK_FLOAT PRIVATE _USE_MATH_DEFINES)
  target_activate_c_compiler_warnings(FFTPACK_FLOAT)
  target_link_libraries( FFTPACK_FLOAT ${MATHLIB} )
  set_property(TARGET FFTPACK_FLOAT APPEND PROPERTY INTERFACE_INCLUDE_DIRECTORIES
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
  )

  # double precision
  add_library(FFTPACK_DOUBLE STATIC fftpack.c fftpack.h)
  target_compile_definitions(FFTPACK_DOUBLE PRIVATE _USE_MATH_DEFINES)
  target_compile_definitions(FFTPACK_DOUBLE PUBLIC FFTPACK_DOUBLE_PRECISION)
  target_activate_c_compiler_warnings(FFTPACK_DOUBLE)
  target_link_libraries( FFTPACK_DOUBLE ${MATHLIB} )
  set_property(TARGET FFTPACK_DOUBLE APPEND PROPERTY INTERFACE_INCLUDE_DIRECTORIES
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
  )

  # builtin test program of fftpack
  add_executable(test_fftpack_float fftpack.c fftpack.h)
  target_compile_definitions(test_fftpack_float PRIVATE _USE_MATH_DEFINES TESTING_FFTPACK)
  target_link_libraries(test_fftpack_float ${MATHLIB})

  add_executable(test_fftpack_double fftpack.c fftpack.h)
  target_compile_definitions(test_fftpack_double PRIVATE _USE_MATH_DEFINES FFTPACK_DOUBLE_PRECISION TESTING_FFTPACK)
  target_link_libraries(test_fftpack_double ${MATHLIB})

endif()

######################################################

if (PFFFT_USE_TYPE_FLOAT)
  # only 'float' supported in PFFASTCONV
  add_library(PFFASTCONV STATIC pffastconv.c pffastconv.h pffft.h )
  set_target_properties(PFFASTCONV PROPERTIES OUTPUT_NAME "pffastconv")
  target_compile_definitions(PFFASTCONV PRIVATE _USE_MATH_DEFINES)
  target_activate_c_compiler_warnings(PFFASTCONV)
  if (PFFFT_USE_DEBUG_ASAN)
    target_compile_options(PFFASTCONV PRIVATE "-fsanitize=address")
  endif()
  target_link_libraries( PFFASTCONV PFFFT ${ASANLIB} ${MATHLIB} )
  set_property(TARGET PFFASTCONV APPEND PROPERTY INTERFACE_INCLUDE_DIRECTORIES
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
  )
  if (INSTALL_PFFASTCONV)
    set(INSTALL_TARGETS ${INSTALL_TARGETS} PFFASTCONV)
    set(INSTALL_HEADERS ${INSTALL_HEADERS} pffastconv.h)
  endif()
endif()

add_executable( test_fft_factors  test_fft_factors.c )
if (PFFFT_USE_TYPE_FLOAT)
  target_compile_definitions(test_fft_factors PRIVATE PFFFT_ENABLE_FLOAT)
endif()
if (PFFFT_USE_TYPE_DOUBLE)
  target_compile_definitions(test_fft_factors PRIVATE PFFFT_ENABLE_DOUBLE)
endif()
target_link_libraries(test_fft_factors PFFFT ${ASANLIB} ${MATHLIB})

######################################################

add_executable( test_pffft_cpp test_pffft.cpp )
target_compile_definitions(test_pffft_cpp PRIVATE _USE_MATH_DEFINES)
if (PFFFT_USE_TYPE_FLOAT)
  target_compile_definitions(test_pffft_cpp PRIVATE PFFFT_ENABLE_FLOAT)
endif()
if (PFFFT_USE_TYPE_DOUBLE)
  target_compile_definitions(test_pffft_cpp PRIVATE PFFFT_ENABLE_DOUBLE)
endif()
target_link_libraries( test_pffft_cpp  PFFFT ${STDCXXLIB} ${ASANLIB} )

######################################################

add_executable( test_pffft_cpp_11 test_pffft.cpp )
target_compile_definitions(test_pffft_cpp_11 PRIVATE _USE_MATH_DEFINES)
if (PFFFT_USE_TYPE_FLOAT)
  target_compile_definitions(test_pffft_cpp_11 PRIVATE PFFFT_ENABLE_FLOAT)
endif()
if (PFFFT_USE_TYPE_DOUBLE)
  target_compile_definitions(test_pffft_cpp_11 PRIVATE PFFFT_ENABLE_DOUBLE)
endif()
target_link_libraries( test_pffft_cpp_11  PFFFT ${STDCXXLIB} ${ASANLIB} )

set_property(TARGET test_pffft_cpp_11 PROPERTY CXX_STANDARD 11)
set_property(TARGET test_pffft_cpp_11 PROPERTY CXX_STANDARD_REQUIRED ON)


######################################################

if (PFFFT_USE_TYPE_FLOAT)
  add_executable(bench_pffft_float   bench_pffft.c pffft.h)
  target_compile_definitions(bench_pffft_float PRIVATE _USE_MATH_DEFINES)
  target_compile_definitions(bench_pffft_float PRIVATE PFFFT_ENABLE_FLOAT)
  if (PFFFT_USE_DEBUG_ASAN)
    target_compile_options(bench_pffft_float PRIVATE "-fsanitize=address")
  endif()

  target_link_libraries( bench_pffft_float  PFFFT ${ASANLIB} )

  if (PFFFT_USE_FFTPACK)
    target_compile_definitions(bench_pffft_float PRIVATE HAVE_FFTPACK=1)
    target_link_libraries(bench_pffft_float  FFTPACK_FLOAT)
  endif()

  if (PFFFT_USE_BENCH_FFTW)
    target_compile_definitions(bench_pffft_float PRIVATE HAVE_FFTW=1)
    target_link_libraries(bench_pffft_float  fftw3f)
  endif()

  if (PATH_GREEN AND PFFFT_USE_BENCH_GREEN)
    target_compile_definitions(bench_pffft_float PRIVATE HAVE_GREEN_FFTS=1)
    target_link_libraries(bench_pffft_float  GreenFFT)
  endif()

  if (PATH_KISS AND PFFFT_USE_BENCH_KISS)
    target_compile_definitions(bench_pffft_float PRIVATE HAVE_KISS_FFT=1)
    target_link_libraries(bench_pffft_float  KissFFT)
  endif()

  if (PATH_POCKET AND PFFFT_USE_BENCH_POCKET)
    target_compile_definitions(bench_pffft_float PRIVATE HAVE_POCKET_FFT=1)
    target_link_libraries(bench_pffft_float  PocketFFT)
  endif()

  if (PFFFT_USE_BENCH_MKL)
    if ( (CMAKE_SYSTEM_PROCESSOR STREQUAL "i686") OR (CMAKE_SYSTEM_PROCESSOR STREQUAL "x86_64") )
      # has chances to work
    else()
      # other PROCESSORs could be "ppc", "ppc64",  "arm", "aarch64", "armv7l" - or something else?!
      message(WARNING "using Intel MKL on '${CMAKE_SYSTEM_PROCESSOR}' might fail.")
    endif()
    message(STATUS "In case compiling/linking with Intel MKL fails, check CMakeLists.txt or deactivate PFFFT_USE_BENCH_MKL")
    target_compile_definitions(bench_pffft_float PRIVATE HAVE_MKL=1)
    target_link_libraries(bench_pffft_float  mkl_intel_lp64 mkl_sequential -lmkl_core)
  endif()
endif()

if (PFFFT_USE_TYPE_DOUBLE)
  add_executable(bench_pffft_double   bench_pffft.c pffft.h)
  target_compile_definitions(bench_pffft_double PRIVATE _USE_MATH_DEFINES)
  target_compile_definitions(bench_pffft_double PRIVATE PFFFT_ENABLE_DOUBLE)
  if (PFFFT_USE_DEBUG_ASAN)
    target_compile_options(bench_pffft_double PRIVATE "-fsanitize=address")
  endif()
  target_link_libraries( bench_pffft_double  PFFFT ${ASANLIB} )

  if (PFFFT_USE_FFTPACK)
    target_compile_definitions(bench_pffft_double PRIVATE HAVE_FFTPACK=1)
    target_link_libraries(bench_pffft_double  FFTPACK_DOUBLE)
  endif()

  if (PFFFT_USE_BENCH_FFTW)
    target_compile_definitions(bench_pffft_double PRIVATE HAVE_FFTW=1)
    target_link_libraries(bench_pffft_double  fftw3)
  endif()

  if (PATH_POCKET AND PFFFT_USE_BENCH_POCKET)
    target_compile_definitions(bench_pffft_double PRIVATE HAVE_POCKET_FFT=1)
    target_link_libraries(bench_pffft_double  PocketFFT)
  endif()

  if (PFFFT_USE_BENCH_MKL)
    if ( (CMAKE_SYSTEM_PROCESSOR STREQUAL "i686") OR (CMAKE_SYSTEM_PROCESSOR STREQUAL "x86_64") )
      # has chances to work
    else()
      # other PROCESSORs could be "ppc", "ppc64",  "arm", "aarch64", "armv7l" - or something else?!
      message(WARNING "using Intel MKL on '${CMAKE_SYSTEM_PROCESSOR}' might fail.")
    endif()
    message(STATUS "In case compiling/linking with Intel MKL fails, check CMakeLists.txt or deactivate PFFFT_USE_BENCH_MKL")
    target_compile_definitions(bench_pffft_double PRIVATE HAVE_MKL=1)
    target_link_libraries(bench_pffft_double  mkl_intel_lp64 mkl_sequential -lmkl_core)
  endif()
endif()

######################################################

if (PFFFT_USE_TYPE_FLOAT)

    add_executable(bench_pf_mixer_float   bench_mixers.cpp papi_perf_counter.h)
    target_compile_definitions(bench_pf_mixer_float PRIVATE _USE_MATH_DEFINES)
    target_compile_definitions(bench_pf_mixer_float PRIVATE PFFFT_ENABLE_FLOAT)
    target_link_libraries( bench_pf_mixer_float  ${ASANLIB} )
    if (PFFFT_USE_DEBUG_ASAN)
      target_compile_options(bench_pf_mixer_float PRIVATE "-fsanitize=address")
    endif()
    if (PAPI_FOUND)
        target_compile_definitions(bench_pf_mixer_float PRIVATE HAVE_PAPI=1)
        target_link_libraries(bench_pf_mixer_float ${PAPI_LIBRARIES})
    endif()
    target_link_libraries( bench_pf_mixer_float  PFDSP $<$<CXX_COMPILER_ID:GNU>:stdc++> )


  ############################################################################

  add_library(pf_conv_arch_none pf_conv.cpp pf_conv.h pf_cplx.h)
  target_compile_definitions(pf_conv_arch_none PRIVATE CONV_ARCH_POST=none MIPP_NO_INTRINSICS=1)
  set_property(TARGET pf_conv_arch_none PROPERTY CXX_STANDARD 11)
  set_property(TARGET pf_conv_arch_none PROPERTY CXX_STANDARD_REQUIRED ON)
  target_activate_cxx_compiler_warnings(pf_conv_arch_none)
  add_library(pf_conv_dispatcher  pf_conv_dispatcher.cpp pf_conv_dispatcher.h pf_conv.h pf_cplx.h)
  set_property(TARGET pf_conv_dispatcher PROPERTY CXX_STANDARD 11)
  set_property(TARGET pf_conv_dispatcher PROPERTY CXX_STANDARD_REQUIRED ON)
  target_activate_cxx_compiler_warnings(pf_conv_dispatcher)

  add_library(pf_conv_arch_dflt pf_conv.cpp pf_conv.h pf_cplx.h)
  target_compile_definitions(pf_conv_arch_dflt PRIVATE CONV_ARCH_POST=dflt)
  set_property(TARGET pf_conv_arch_dflt PROPERTY CXX_STANDARD 11)
  set_property(TARGET pf_conv_arch_dflt PROPERTY CXX_STANDARD_REQUIRED ON)
  target_activate_cxx_compiler_warnings(pf_conv_arch_dflt)
  target_set_cxx_arch_flags(pf_conv_arch_dflt)

  target_link_libraries(pf_conv_dispatcher pf_conv_arch_none pf_conv_arch_dflt)

  if ((CMAKE_SYSTEM_PROCESSOR MATCHES "x86_64") OR (CMAKE_SYSTEM_PROCESSOR MATCHES "AMD64"))

    if ((CMAKE_CXX_COMPILER_ID STREQUAL "GNU") OR CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
        set(PF_CONV_ARCHES "sse3;sse4;avx;avx2")
        set(PF_CONV_OPT_sse3 "core2")  # emulate a map
        set(PF_CONV_OPT_sse4 "nehalem")
        set(PF_CONV_OPT_avx  "sandybridge")
        set(PF_CONV_OPT_avx2 "haswell")
        target_compile_definitions(pf_conv_dispatcher PRIVATE CONV_ARCH_GCC_AMD64)
    elseif (CMAKE_CXX_COMPILER_ID MATCHES "MSVC")
        set(PF_CONV_ARCHES "sse2;avx;avx2")
        set(PF_CONV_OPT_sse2 "SSE2")  # emulate a map
        set(PF_CONV_OPT_avx  "AVX")
        set(PF_CONV_OPT_avx2 "AVX2")
        target_compile_definitions(pf_conv_dispatcher PRIVATE CONV_ARCH_MSVC_AMD64)
    else()
        set(PF_CONV_ARCHES "")
        message(WARNING "unknown compiler ${CMAKE_CXX_COMPILER_ID} on CMAKE_SYSTEM_PROCESSOR ${CMAKE_SYSTEM_PROCESSOR}: can't do architecture specific compilation")
    endif()

  elseif (CMAKE_SYSTEM_PROCESSOR MATCHES "aarch64")

      if ((CMAKE_CXX_COMPILER_ID STREQUAL "GNU") OR CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
          set(PF_CONV_ARCHES "armv8a")
          set(PF_CONV_OPT_armv8a   "armv8-a")  # emulate a map for arch

          target_compile_definitions(pf_conv_dispatcher PRIVATE CONV_ARCH_GCC_AARCH64)
      else()
          set(PF_CONV_ARCHES "")
          message(WARNING "unknown compiler ${CMAKE_CXX_COMPILER_ID} on CMAKE_SYSTEM_PROCESSOR ${CMAKE_SYSTEM_PROCESSOR}: can't do architecture specific compilation")
      endif()

  elseif (CMAKE_SYSTEM_PROCESSOR MATCHES "armv7l")

    if ((CMAKE_CXX_COMPILER_ID STREQUAL "GNU") OR CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
        set(PF_CONV_ARCHES "neon_vfpv4;neon_rpi3_a53;neon_rpi4_a72")
        set(PF_CONV_OPT_neon_vfpv4        "armv7-a")    # emulate a map for arch
        set(PF_CONV_EXTRA_neon_vfpv4      "neon_vfpv4") # emulate a map for additional options (EXTRA)
        set(PF_CONV_OPT_neon_rpi3_a53     "armv7-a")
        set(PF_CONV_EXTRA_neon_rpi3_a53   "neon_rpi3_a53")
        set(PF_CONV_OPT_neon_rpi4_a72     "armv7-a")
        set(PF_CONV_EXTRA_neon_rpi4_a72   "neon_rpi4_a72")

        target_compile_definitions(pf_conv_dispatcher PRIVATE CONV_ARCH_GCC_ARM32NEON)
    else()
        set(PF_CONV_ARCHES "")
        message(WARNING "unknown compiler ${CMAKE_CXX_COMPILER_ID} on CMAKE_SYSTEM_PROCESSOR ${CMAKE_SYSTEM_PROCESSOR}: can't do architecture specific compilation")
    endif()

  else()
      message(WARNING "this is unforseen CMAKE_SYSTEM_PROCESSOR ${CMAKE_SYSTEM_PROCESSOR}: can't do architecture specific compilation")
  endif()

  foreach (arch_opt ${PF_CONV_ARCHES})
      add_library(pf_conv_arch_${arch_opt} pf_conv.cpp pf_conv.h pf_cplx.h)
      set_property(TARGET pf_conv_arch_${arch_opt} PROPERTY CXX_STANDARD 11)
      set_property(TARGET pf_conv_arch_${arch_opt} PROPERTY CXX_STANDARD_REQUIRED ON)
      target_activate_cxx_compiler_warnings(pf_conv_arch_${arch_opt})
      target_compile_definitions(pf_conv_arch_${arch_opt} PRIVATE CONV_ARCH_POST=${arch_opt})

      target_set_cxx_arch_option(pf_conv_arch_${arch_opt} "${PF_CONV_OPT_${arch_opt}}" "${PF_CONV_EXTRA_${arch_opt}}"  "${PF_CONV_OPT_${arch_opt}}")
      target_link_libraries(pf_conv_dispatcher  pf_conv_arch_${arch_opt})
      message(STATUS "added library pf_conv_arch_${arch_opt}  with CONV_ARCH_POST=${arch_opt}")
  endforeach()

  if (PFFFT_USE_DEBUG_ASAN)
      foreach (arch_opt ${PF_CONV_ARCHES})
          target_compile_options(pf_conv_arch_${arch_opt} PRIVATE "-fsanitize=address")
          target_link_libraries( pf_conv_arch_${arch_opt} ${ASANLIB})
      endforeach()

      target_compile_options(pf_conv_arch_none  PRIVATE "-fsanitize=address")
      target_link_libraries( pf_conv_arch_none  ${ASANLIB})

      target_compile_options(pf_conv_dispatcher  PRIVATE "-fsanitize=address")
      target_link_libraries(pf_conv_dispatcher ${ASANLIB})
  endif()

  if(MIPP_FOUND)
      foreach (arch_opt ${PF_CONV_ARCHES})
          message(STATUS "link pf_conv_arch_${arch_opt} against MIPP")
          target_link_libraries(pf_conv_arch_${arch_opt} MIPP)
      endforeach()

      message(STATUS "link pf_conv_arch_none against MIPP")
      target_link_libraries(pf_conv_arch_none MIPP)
  endif()

  ############################################################################

  add_executable(bench_pf_conv_float   bench_conv.cpp papi_perf_counter.h)
  set_property(TARGET bench_pf_conv_float PROPERTY CXX_STANDARD 11)
  set_property(TARGET bench_pf_conv_float PROPERTY CXX_STANDARD_REQUIRED ON)
  target_compile_definitions(bench_pf_conv_float PRIVATE _USE_MATH_DEFINES)
  target_compile_definitions(bench_pf_conv_float PRIVATE PFFFT_ENABLE_FLOAT)
  if (PFFFT_USE_DEBUG_ASAN)
      target_compile_options(bench_pf_conv_float PRIVATE "-fsanitize=address")
  endif()
  target_link_libraries( bench_pf_conv_float  ${ASANLIB} )
  if (PAPI_FOUND)
      target_compile_definitions(bench_pf_conv_float PRIVATE HAVE_PAPI=1)
      target_link_libraries(bench_pf_conv_float ${PAPI_LIBRARIES})
  endif()
  if(MIPP_FOUND)
      target_link_libraries(bench_pf_conv_float MIPP)
  endif()
  target_link_libraries( bench_pf_conv_float  pf_conv_dispatcher PFDSP $<$<CXX_COMPILER_ID:GNU>:stdc++> )

endif()
