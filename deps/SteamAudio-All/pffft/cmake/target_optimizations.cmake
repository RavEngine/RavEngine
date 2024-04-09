
# cmake options: TARGET_C_ARCH / TARGET_CPP_ARCH:
#   and optionally:  TARGET_C_EXTRA TARGET_CXX_EXTRA
#
# provided:
#   - function: target_set_c_arch_flags(<target>)    # uses options TARGET_C_ARCH and TARGET_C_EXTRA
#   - function: target_set_cxx_arch_flags(<target>)  # uses options TARGET_CXX_ARCH and TARGET_CXX_EXTRA
#   - macro:    target_set_cxx_arch_option(<target> <gcc/clang_march> <gcc/clang_extra> <msvc_arch>)
#
# see https://en.wikichip.org/wiki/x86/extensions
# and https://gcc.gnu.org/onlinedocs/gcc/x86-Options.html
#   for gcc specific architecture options
# and https://docs.microsoft.com/en-us/cpp/build/reference/arch-x64
# or  https://docs.microsoft.com/en-us/cpp/build/reference/arch-x86
#   for msvc specific architecture options

# https://en.wikichip.org/wiki/arm/versions
# https://en.wikipedia.org/wiki/Raspberry_Pi
# https://gcc.gnu.org/onlinedocs/gcc/ARM-Options.html#ARM-Options
# https://en.wikipedia.org/wiki/Comparison_of_ARMv7-A_cores
# https://en.wikipedia.org/wiki/Comparison_of_ARMv8-A_cores

# arm32_rpi1 untested
#   -mcpu=arm1176jzf-s -mfloat-abi=hard -mfpu=vfp         -mtune=arm1176jzf-s
# arm32_rpi2 untested
#   "-march=armv7-a"   "-mfloat-abi=hard" "-mfpu=neon-vfpv4"
#   "-march=armv8-a"   "-mfloat-abi=hard" "-mfpu=neon-vfpv4"
# arm32_rpi3 with "armv7-a" tested on Raspbian GNU/Linux 10 (buster), 32-bit  => MIPP test reports: NEONv1, 128 bits
#   "-march=armv7-a"   "-mfloat-abi=hard" "-mfpu=neon-vfpv4"
# arm32_rpi3 with "armv8-a" tested on Raspbian GNU/Linux 10 (buster), 32-bit  => MIPP test reports: NEONv1, 128 bits
#   "-march=armv8-a"   "-mfloat-abi=hard" "-mfpu=neon-vfpv4"
# arm32_rpi3 with "armv8-a" tested on Raspbian GNU/Linux 10 (buster), 32-bit  => MIPP test reports: NEONv1, 128 bits
#   "-march=armv8-a"   "-mfloat-abi=hard" "-mfpu=neon-vfpv4" "-mtune=cortex-a53"
# arm32_rpi4 untested
#   RPi 4 Model B:    Cortex-A72  =>  "-mtune=cortex-a72"  ?
#   "-mcpu=cortex-a72 -mfloat-abi=hard -mfpu=neon-fp-armv8 -mneon-for-64bits  -mtune=cortex-a72"

set(MSVC_EXTRA_OPT_none "")
set(GCC_EXTRA_OPT_none "")
set(GCC_EXTRA_OPT_neon_vfpv4    "-mfloat-abi=hard" "-mfpu=neon-vfpv4")
set(GCC_EXTRA_OPT_neon_rpi3_a53 "-mfloat-abi=hard" "-mfpu=neon-vfpv4" "-mtune=cortex-a53")
set(GCC_EXTRA_OPT_neon_rpi4_a72 "-mfloat-abi=hard" "-mfpu=neon-fp-armv8" "-mtune=cortex-a72")

if ( (CMAKE_SYSTEM_PROCESSOR STREQUAL "i686") OR (CMAKE_SYSTEM_PROCESSOR STREQUAL "x86_64") )
    set(GCC_MARCH_DESC "native/SSE2:pentium4/SSE3:core2/SSE4:nehalem/AVX:sandybridge/AVX2:haswell")
    set(GCC_MARCH_VALUES "none;native;pentium4;core2;nehalem;sandybridge;haswell" CACHE INTERNAL "List of possible architectures")
    set(GCC_EXTRA_VALUES "" CACHE INTERNAL "List of possible EXTRA options")
elseif (CMAKE_SYSTEM_PROCESSOR MATCHES "aarch64")
    set(GCC_MARCH_DESC "native/ARMwNEON:armv8-a")
    set(GCC_MARCH_VALUES "none;native;armv8-a" CACHE INTERNAL "List of possible architectures")
    set(GCC_EXTRA_VALUES "" CACHE INTERNAL "List of possible additional options")
elseif (CMAKE_SYSTEM_PROCESSOR MATCHES "armv7l")
    set(GCC_MARCH_DESC "native/ARMwNEON:armv7-a")
    set(GCC_MARCH_VALUES "none;native;armv7-a" CACHE INTERNAL "List of possible architectures")
    set(GCC_EXTRA_VALUES "none;neon_vfpv4;neon_rpi3_a53;neon_rpi4_a72" CACHE INTERNAL "List of possible additional options")
else()
    message(WARNING "unsupported CMAKE_SYSTEM_PROCESSOR '${CMAKE_SYSTEM_PROCESSOR}'")
    # other PROCESSORs could be "ppc", "ppc64",  "arm" - or something else?!
    set(GCC_MARCH_DESC "native")
    set(GCC_MARCH_VALUES "none;native" CACHE INTERNAL "List of possible architectures")
    set(GCC_EXTRA_VALUES "" CACHE INTERNAL "List of possible additional options")
endif()

# cmake options - depending on C/C++ compiler
# how are chances, that C and C++ compilers are from different vendors?
if (CMAKE_C_COMPILER_ID STREQUAL "GNU")
    set(TARGET_C_ARCH "none" CACHE STRING "gcc target C architecture (-march): ${GCC_MARCH_DESC}")
    set_property(CACHE TARGET_C_ARCH PROPERTY STRINGS ${GCC_MARCH_VALUES})
    if ( NOT (GCC_EXTRA_VALUES STREQUAL "") )
        set(TARGET_C_EXTRA "none" CACHE STRING "gcc additional options for C")
        set_property(CACHE TARGET_C_EXTRA PROPERTY STRINGS ${GCC_EXTRA_VALUES})
    endif()
elseif (CMAKE_C_COMPILER_ID STREQUAL "Clang")
    set(TARGET_C_ARCH "none" CACHE STRING "clang target C architecture (-march): ${GCC_MARCH_DESC}")
    set_property(CACHE TARGET_C_ARCH PROPERTY STRINGS ${GCC_MARCH_VALUES})
    if ( NOT (GCC_EXTRA_VALUES STREQUAL "") )
        set(TARGET_C_EXTRA "none" CACHE STRING "gcc additional options for C")
        set_property(CACHE TARGET_C_EXTRA PROPERTY STRINGS ${GCC_EXTRA_VALUES})
    endif()
elseif (CMAKE_C_COMPILER_ID MATCHES "MSVC")
    set(TARGET_C_ARCH "none" CACHE STRING "msvc target C architecture (/arch): SSE2/AVX/AVX2/AVX512")
    set(TARGET_C_EXTRA "none" CACHE STRING "msvc additional options")
else()
    message(WARNING "unsupported C compiler '${CMAKE_C_COMPILER_ID}', see https://cmake.org/cmake/help/latest/variable/CMAKE_LANG_COMPILER_ID.html")
endif()

if (CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
    set(TARGET_CXX_ARCH "none" CACHE STRING "gcc target C++ architecture (-march): ${GCC_MARCH_DESC}")
    set_property(CACHE TARGET_CXX_ARCH PROPERTY STRINGS ${GCC_MARCH_VALUES})
    if ( NOT (GCC_EXTRA_VALUES STREQUAL "") )
        set(TARGET_CXX_EXTRA "none" CACHE STRING "gcc additional options for C++")
        set_property(CACHE TARGET_CXX_EXTRA PROPERTY STRINGS ${GCC_EXTRA_VALUES})
    endif()
elseif (CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
    set(TARGET_CXX_ARCH "none" CACHE STRING "clang target C++ architecture (-march): ${GCC_MARCH_DESC}")
    set_property(CACHE TARGET_CXX_ARCH PROPERTY STRINGS ${GCC_MARCH_VALUES})
    if ( NOT (GCC_EXTRA_VALUES STREQUAL "") )
        set(TARGET_CXX_EXTRA "none" CACHE STRING "clang additional options for C++")
        set_property(CACHE TARGET_CXX_EXTRA PROPERTY STRINGS ${GCC_EXTRA_VALUES})
    endif()
elseif (CMAKE_CXX_COMPILER_ID MATCHES "MSVC")
    set(TARGET_CXX_ARCH "none" CACHE STRING "msvc target C++ architecture (/arch): SSE2/AVX/AVX2/AVX512")
    set(TARGET_CXX_EXTRA "none" CACHE STRING "msvc additional options")
else()
    message(WARNING "unsupported C++ compiler '${CMAKE_CXX_COMPILER_ID}', see https://cmake.org/cmake/help/latest/variable/CMAKE_LANG_COMPILER_ID.html")
endif()

######################################################

function(target_set_c_arch_flags target)
    if ( ("${TARGET_C_ARCH}" STREQUAL "") OR ("${TARGET_C_ARCH}" STREQUAL "none") )
        message(STATUS "C ARCH for target ${target} is not set!")
    else()
        if ( (CMAKE_C_COMPILER_ID STREQUAL "GNU") OR (CMAKE_C_COMPILER_ID STREQUAL "Clang") )
            target_compile_options(${target} PRIVATE "-march=${TARGET_C_ARCH}")
            message(STATUS "C ARCH for target ${target} set: ${TARGET_C_ARCH}")
        elseif (CMAKE_C_COMPILER_ID MATCHES "MSVC")
            target_compile_options(${target} PRIVATE "/arch:${TARGET_C_ARCH}")
            message(STATUS "C ARCH for target ${target} set: ${TARGET_C_ARCH}")
        else()
            message(WARNING "unsupported C compiler '${CMAKE_C_COMPILER_ID}' for target_set_c_arch_flags(), see https://cmake.org/cmake/help/latest/variable/CMAKE_LANG_COMPILER_ID.html")
        endif()
    endif()
    if ( ("${TARGET_C_EXTRA}" STREQUAL "") OR ("${TARGET_C_EXTRA}" STREQUAL "none") )
        message(STATUS "C additional options for target ${target} is not set!")
    else()
        if ( (CMAKE_C_COMPILER_ID STREQUAL "GNU") OR (CMAKE_C_COMPILER_ID STREQUAL "Clang") )
            target_compile_options(${target} PRIVATE "${GCC_EXTRA_OPT_${TARGET_C_EXTRA}}")
            message(STATUS "C additional options for target ${target} set: ${GCC_EXTRA_OPT_${TARGET_C_EXTRA}}")
        elseif (CMAKE_C_COMPILER_ID MATCHES "MSVC")
            # target_compile_options(${target} PRIVATE "${MSVC_EXTRA_OPT_${TARGET_C_EXTRA}}")
            message(STATUS "C additional options for target ${target} not usable with MSVC")
        else()
            message(WARNING "unsupported C compiler '${CMAKE_C_COMPILER_ID}' for target_set_c_arch_flags(), see https://cmake.org/cmake/help/latest/variable/CMAKE_LANG_COMPILER_ID.html")
        endif()
        if ( ("${TARGET_C_EXTRA}" MATCHES "^neon_.*") OR (CMAKE_SYSTEM_PROCESSOR MATCHES "aarch64") )
            message(STATUS "additional option contains neon: setting PFFFT_ENABLE_NEON for C target ${target}")
            target_compile_definitions(${target} PRIVATE PFFFT_ENABLE_NEON=1)
        endif()
    endif()
endfunction()

function(target_set_cxx_arch_flags target)
    if ( ("${TARGET_CXX_ARCH}" STREQUAL "") OR ("${TARGET_CXX_ARCH}" STREQUAL "none") )
        message(STATUS "C++ ARCH for target ${target} is not set!")
    else()
        if ( (CMAKE_CXX_COMPILER_ID STREQUAL "GNU") OR (CMAKE_CXX_COMPILER_ID STREQUAL "Clang") )
            target_compile_options(${target} PRIVATE "-march=${TARGET_CXX_ARCH}")
            message(STATUS "C++ ARCH for target ${target} set: ${TARGET_CXX_ARCH}")
        elseif (CMAKE_CXX_COMPILER_ID MATCHES "MSVC")
            target_compile_options(${target} PRIVATE "/arch:${TARGET_CXX_ARCH}")
            message(STATUS "C++ ARCH for target ${target} set: ${TARGET_CXX_ARCH}")
        else()
            message(WARNING "unsupported C++ compiler '${CMAKE_CXX_COMPILER_ID}' for target_set_cxx_arch_flags(), see https://cmake.org/cmake/help/latest/variable/CMAKE_LANG_COMPILER_ID.html")
        endif()
    endif()
    if ( ("${TARGET_CXX_EXTRA}" STREQUAL "") OR ("${TARGET_CXX_EXTRA}" STREQUAL "none") )
        message(STATUS "C++ additional options for target ${target} is not set!")
    else()
        if ( (CMAKE_C_COMPILER_ID STREQUAL "GNU") OR (CMAKE_C_COMPILER_ID STREQUAL "Clang") )
            target_compile_options(${target} PRIVATE "${GCC_EXTRA_OPT_${TARGET_CXX_EXTRA}}")
            message(STATUS "C++ additional options for target ${target} set: ${GCC_EXTRA_OPT_${TARGET_CXX_EXTRA}}")
        elseif (CMAKE_C_COMPILER_ID MATCHES "MSVC")
            # target_compile_options(${target} PRIVATE "${MSVC_EXTRA_OPT_${TARGET_CXX_EXTRA}}")
            message(STATUS "C++ additional options for target ${target} not usable with MSVC")
        else()
          message(WARNING "unsupported C compiler '${CMAKE_C_COMPILER_ID}' for target_set_c_arch_flags(), see https://cmake.org/cmake/help/latest/variable/CMAKE_LANG_COMPILER_ID.html")
        endif()
        if ( ("${TARGET_CXX_EXTRA}" MATCHES "^neon_.*") OR (CMAKE_SYSTEM_PROCESSOR MATCHES "aarch64") )
            message(STATUS "additional option contains 'neon': setting PFFFT_ENABLE_NEON for C++ target ${target}")
            target_compile_definitions(${target} PRIVATE PFFFT_ENABLE_NEON=1)
        endif()
    endif()
endfunction()


macro(target_set_cxx_arch_option target gcc_clang_arch gcc_clang_extra msvc_arch )
    if ( (CMAKE_CXX_COMPILER_ID STREQUAL "GNU") OR (CMAKE_CXX_COMPILER_ID STREQUAL "Clang") )

        if ( NOT (("${gcc_clang_arch}" STREQUAL "") OR ("${gcc_clang_arch}" STREQUAL "none") ) )
            target_compile_options(${target} PRIVATE "-march=${gcc_clang_arch}")
            message(STATUS "C++ ARCH for target ${target}: ${gcc_clang_arch}")
        endif()
        if (NOT ( ("${gcc_clang_extra}" STREQUAL "") OR ("${gcc_clang_extra}" STREQUAL "none") ) )
            target_compile_options(${target} PRIVATE "${GCC_EXTRA_OPT_${gcc_clang_extra}}")
            message(STATUS "C++ additional options for target ${target}: ${GCC_EXTRA_OPT_${gcc_clang_extra}}")
        endif()
    elseif (CMAKE_CXX_COMPILER_ID MATCHES "MSVC")
        if ( NOT (("${msvc_arch}" STREQUAL "") OR ("${msvc_arch}" STREQUAL "none") ) )
            target_compile_options(${target} PRIVATE "/arch:${msvc_arch}")
            message(STATUS "C++ ARCH for target ${target} set: ${msvc_arch}")
        endif()
    else()
        message(WARNING "unsupported C++ compiler '${CMAKE_CXX_COMPILER_ID}' for target_set_cxx_arch_option(), see https://cmake.org/cmake/help/latest/variable/CMAKE_LANG_COMPILER_ID.html")
    endif()
endmacro()

