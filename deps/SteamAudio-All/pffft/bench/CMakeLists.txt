cmake_minimum_required(VERSION 2.8)
project(BENCH_PFFFT)

set(BENCH_ID  "default" CACHE STRING "ID: use single word without spaces. gets part of result filename")

option(BENCH_FAST_MATH  "Build with fast math - non IEEE compliant" ON)

if (CMAKE_C_COMPILER_ID STREQUAL "GNU")
  set(BENCH_ARCH "native" CACHE STRING "target architecture (-march): native/SSE:core2/AVX:sandybridge/ARM-NEON:armv7-a")
elseif (CMAKE_C_COMPILER_ID STREQUAL "Clang")
  set(BENCH_ARCH "native" CACHE STRING "target architecture (-march): native/SSE:core2/AVX:sandybridge")
elseif (CMAKE_C_COMPILER_ID STREQUAL "MSVC")  # others: "Intel"
  set(BENCH_ARCH "AVX" CACHE STRING "target architecture (/arch): SSE2/AVX")
else()
  set(BENCH_ARCH "" CACHE STRING "target architecture - use full compiler option!")
endif()

# architecture/optimization options
option(PFFFT_USE_SIMD        "use SIMD (SSE/AVX/NEON/ALTIVEC) CPU features? - " ON)
option(DISABLE_SIMD_AVX "disable AVX CPU features? - " OFF)
option(PFFFT_USE_SIMD_NEON   "force using NEON on ARM? (requires PFFFT_USE_SIMD)" OFF)
option(PFFFT_USE_SCALAR_VECT "use 4-element vector scalar operations (if no other SIMD)" ON)

option(PFFFT_USE_BENCH_FFTW   "use (system-installed) FFTW3 in fft benchmark?" OFF)
option(PFFFT_USE_BENCH_GREEN  "use Green FFT in fft benchmark? - if exists in subdir" ON)
option(PFFFT_USE_BENCH_KISS   "use KissFFT in fft benchmark? - if exists in subdir" ON)
option(PFFFT_USE_BENCH_POCKET "use PocketFFT in fft benchmark? - if exists in subdir" ON)
option(PFFFT_USE_BENCH_MKL    "use Intel MKL in fft benchmark? needs to be installed" OFF)


set(OSSTR "")
if (WIN32)
  set(OSSTR "Win32")
endif (WIN32)
if (UNIX)
  set(OSSTR "Unix")
endif (UNIX)

set(BUILD_DIR_TO_EXE "")
set(CMAKE_PLATFORM_OPT "")
set(CMAKE_MAKE_OPT "")
if (MSVC)
  set(BUILD_DIR_TO_EXE "Release/")
  set(CMAKE_PLATFORM_OPT "-A \"${CMAKE_GENERATOR_PLATFORM}\"")
  set(CMAKE_MAKE_OPT "-DCMAKE_MAKE_PROGRAM=${CMAKE_MAKE_PROGRAM}")
endif()


set(benchdir "${CMAKE_BINARY_DIR}/bench_${BENCH_ID}")
set(benchdir_flt "${CMAKE_BINARY_DIR}/bench_${BENCH_ID}/float")
set(benchdir_dbl "${CMAKE_BINARY_DIR}/bench_${BENCH_ID}/double")
set(builddir_flt "${CMAKE_BINARY_DIR}/build_${BENCH_ID}_float")
set(builddir_dbl "${CMAKE_BINARY_DIR}/build_${BENCH_ID}_double")

add_custom_command(OUTPUT "${benchdir}/info.txt"
  COMMAND ${CMAKE_COMMAND} -E make_directory "${benchdir}"
  COMMAND ${CMAKE_COMMAND} -E echo "benchmark ${BENCH_ID}"   > "${benchdir}/info.txt"
  COMMAND ${CMAKE_COMMAND} -E echo "CMake major:    ${CMAKE_MAJOR_VERSION}"        >> "${benchdir}/info.txt"
  COMMAND ${CMAKE_COMMAND} -E echo "CMake minor:    ${CMAKE_MINOR_VERSION}"        >> "${benchdir}/info.txt"
  COMMAND ${CMAKE_COMMAND} -E echo "OS:             ${OSSTR}"                      >> "${benchdir}/info.txt"
  COMMAND ${CMAKE_COMMAND} -E echo "System:         ${CMAKE_SYSTEM_NAME}"          >> "${benchdir}/info.txt"
  COMMAND ${CMAKE_COMMAND} -E echo "System CPU:     ${CMAKE_SYSTEM_PROCESSOR}"     >> "${benchdir}/info.txt"
  COMMAND ${CMAKE_COMMAND} -E echo "System Version: ${CMAKE_HOST_SYSTEM_VERSION}"  >> "${benchdir}/info.txt"
  COMMAND ${CMAKE_COMMAND} -E echo "C   Compiler:   ${CMAKE_C_COMPILER_ID}"        >> "${benchdir}/info.txt"
  COMMAND ${CMAKE_COMMAND} -E echo "C   Version:    ${CMAKE_C_COMPILER_VERSION}"   >> "${benchdir}/info.txt"
  COMMAND ${CMAKE_COMMAND} -E echo "C++ Compiler:   ${CMAKE_CXX_COMPILER_ID}"      >> "${benchdir}/info.txt"
  COMMAND ${CMAKE_COMMAND} -E echo "C++ Version:    ${CMAKE_CXX_COMPILER_VERSION}" >> "${benchdir}/info.txt"
  COMMAND ${CMAKE_COMMAND} -E echo "MSVC Version:   ${MSVC_VERSION}"               >> "${benchdir}/info.txt"
  COMMAND ${CMAKE_COMMAND} -E echo "MSVC Toolset:   ${MSVC_TOOLSET_VERSION}"       >> "${benchdir}/info.txt"
  COMMAND ${CMAKE_COMMAND} -E echo "Exe Suffix:     ${CMAKE_EXECUTABLE_SUFFIX}"    >> "${benchdir}/info.txt"
  COMMAND ${CMAKE_COMMAND} -E echo "C   Byte Order: ${CMAKE_C_BYTE_ORDER}"         >> "${benchdir}/info.txt"
  COMMAND ${CMAKE_COMMAND} -E echo "C++ Byte Order: ${CMAKE_CXX_BYTE_ORDER}"       >> "${benchdir}/info.txt"
  COMMAND ${CMAKE_COMMAND} -E echo ""                                              >> "${benchdir}/info.txt"
  COMMAND ${CMAKE_COMMAND} -E echo "Architecture:   ${BENCH_ARCH}"                 >> "${benchdir}/info.txt"
  COMMAND ${CMAKE_COMMAND} -E echo "Fast math:      ${BENCH_FAST_MATH}"            >> "${benchdir}/info.txt"
  COMMAND ${CMAKE_COMMAND} -E echo "config PFFFT_USE_SIMD=${PFFFT_USE_SIMD}"                   >> "${benchdir}/info.txt"
  COMMAND ${CMAKE_COMMAND} -E echo "config DISABLE_SIMD_AVX=${DISABLE_SIMD_AVX}"   >> "${benchdir}/info.txt"
  COMMAND ${CMAKE_COMMAND} -E echo "config PFFFT_USE_SIMD_NEON=${PFFFT_USE_SIMD_NEON}"         >> "${benchdir}/info.txt"
  COMMAND ${CMAKE_COMMAND} -E echo "config PFFFT_USE_SCALAR_VECT=${PFFFT_USE_SCALAR_VECT}"     >> "${benchdir}/info.txt"
  COMMAND ${CMAKE_COMMAND} -E echo "config PFFFT_USE_BENCH_FFTW=${PFFFT_USE_BENCH_FFTW}"       >> "${benchdir}/info.txt"
  COMMAND ${CMAKE_COMMAND} -E echo "config PFFFT_USE_BENCH_GREEN=${PFFFT_USE_BENCH_GREEN}"     >> "${benchdir}/info.txt"
  COMMAND ${CMAKE_COMMAND} -E echo "config PFFFT_USE_BENCH_KISS=${PFFFT_USE_BENCH_KISS}"       >> "${benchdir}/info.txt"
  COMMAND ${CMAKE_COMMAND} -E echo "config PFFFT_USE_BENCH_POCKET=${PFFFT_USE_BENCH_POCKET}"   >> "${benchdir}/info.txt"
  COMMAND ${CMAKE_COMMAND} -E echo "config PFFFT_USE_BENCH_MKL=${PFFFT_USE_BENCH_MKL}"         >> "${benchdir}/info.txt"
)

if (UNIX)
  add_custom_command(OUTPUT "${benchdir}/unix_info.txt"
    COMMAND ${CMAKE_COMMAND} -E touch "${benchdir}/unix_info.txt"
    COMMAND bash "-c" "${CMAKE_CURRENT_SOURCE_DIR}/unix_info.sh"
    DEPENDS "${benchdir}/info.txt"
    WORKING_DIRECTORY ${benchdir}
  )
else()
  add_custom_command(OUTPUT "${benchdir}/unix_info.txt"
    COMMAND ${CMAKE_COMMAND} -E touch "${benchdir}/unix_info.txt"
    DEPENDS "${benchdir}/info.txt"
    WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
  )
endif()


add_custom_command(OUTPUT "${builddir_flt}/directory.txt"
  COMMAND ${CMAKE_COMMAND} -E make_directory "${builddir_flt}"
  COMMAND ${CMAKE_COMMAND} -E touch "${builddir_flt}/directory.txt"
)

add_custom_command(OUTPUT "${builddir_dbl}/directory.txt"
  COMMAND ${CMAKE_COMMAND} -E make_directory "${builddir_dbl}"
  COMMAND ${CMAKE_COMMAND} -E touch "${builddir_dbl}/directory.txt"
)

add_custom_command(OUTPUT "${benchdir_flt}/directory.txt"
  COMMAND ${CMAKE_COMMAND} -E make_directory "${benchdir_flt}"
  COMMAND ${CMAKE_COMMAND} -E touch "${benchdir_flt}/directory.txt"
)

add_custom_command(OUTPUT "${benchdir_dbl}/directory.txt"
  COMMAND ${CMAKE_COMMAND} -E make_directory "${benchdir_dbl}"
  COMMAND ${CMAKE_COMMAND} -E touch "${benchdir_dbl}/directory.txt"
)



add_custom_target(build_float
  COMMAND ${CMAKE_COMMAND} -E echo "start cmake for float in ${builddir_flt}"
  COMMAND ${CMAKE_COMMAND} -G "${CMAKE_GENERATOR}" ${CMAKE_PLATFORM_OPT}
                        "${CMAKE_MAKE_OPT}"
                        -DCMAKE_BUILD_TYPE=Release
                        "-DARCH=${BENCH_ARCH}"
                        -DUSE_FAST_MATH=${BENCH_FAST_MATH}
                        -DPFFFT_USE_TYPE_FLOAT=ON
                        -DPFFFT_USE_TYPE_DOUBLE=OFF
                        -DUSE_FLOAT_PREC=ON
                        -DPFFFT_USE_SIMD=${PFFFT_USE_SIMD}
                        -DDISABLE_SIMD_AVX=${DISABLE_SIMD_AVX}
                        -DPFFFT_USE_SIMD_NEON=${PFFFT_USE_SIMD_NEON}
                        -DPFFFT_USE_SCALAR_VECT=${PFFFT_USE_SCALAR_VECT}
                        -DPFFFT_USE_BENCH_FFTW=${PFFFT_USE_BENCH_FFTW}
                        -DPFFFT_USE_BENCH_GREEN=${PFFFT_USE_BENCH_GREEN}
                        -DPFFFT_USE_BENCH_KISS=${PFFFT_USE_BENCH_KISS}
                        -DPFFFT_USE_BENCH_POCKET=${PFFFT_USE_BENCH_POCKET}
                        -DPFFFT_USE_BENCH_MKL=${PFFFT_USE_BENCH_MKL}
                        "${CMAKE_SOURCE_DIR}/.."
  # COMMAND ${CMAKE_COMMAND} -E echo "start cmake --build . for float in ${builddir_flt}"
  COMMAND ${CMAKE_COMMAND} --build . --config Release
  DEPENDS "${builddir_flt}/directory.txt"
  WORKING_DIRECTORY "${builddir_flt}"
)

add_custom_target(build_double
  COMMAND ${CMAKE_COMMAND} -E echo "start cmake for double in ${builddir_dbl}"
  COMMAND ${CMAKE_COMMAND} -G "${CMAKE_GENERATOR}" ${CMAKE_PLATFORM_OPT}
                        "${CMAKE_MAKE_OPT}"
                        -DCMAKE_BUILD_TYPE=Release
                        "-DARCH=${BENCH_ARCH}"
                        -DUSE_FAST_MATH=${BENCH_FAST_MATH}
                        -DPFFFT_USE_TYPE_FLOAT=OFF
                        -DPFFFT_USE_TYPE_DOUBLE=ON
                        -DUSE_FLOAT_PREC=OFF
                        -DPFFFT_USE_SIMD=${PFFFT_USE_SIMD}
                        -DDISABLE_SIMD_AVX=${DISABLE_SIMD_AVX}
                        -DPFFFT_USE_SIMD_NEON=${PFFFT_USE_SIMD_NEON}
                        -DPFFFT_USE_SCALAR_VECT=${PFFFT_USE_SCALAR_VECT}
                        -DPFFFT_USE_BENCH_FFTW=${PFFFT_USE_BENCH_FFTW}
                        -DPFFFT_USE_BENCH_GREEN=${PFFFT_USE_BENCH_GREEN}
                        -DPFFFT_USE_BENCH_KISS=${PFFFT_USE_BENCH_KISS}
                        -DPFFFT_USE_BENCH_POCKET=${PFFFT_USE_BENCH_POCKET}
                        -DPFFFT_USE_BENCH_MKL=${PFFFT_USE_BENCH_MKL}
                        "${CMAKE_SOURCE_DIR}/.."
  COMMAND ${CMAKE_COMMAND} -E echo "start cmake --build . for double in ${builddir_dbl}"
  COMMAND ${CMAKE_COMMAND} --build . --config Release
  DEPENDS "${builddir_dbl}/directory.txt"
  WORKING_DIRECTORY "${builddir_dbl}"
)

add_custom_target(bench_float
  COMMAND ${CMAKE_COMMAND} -E echo "start benchmark for float"
  COMMAND "${builddir_flt}/${BUILD_DIR_TO_EXE}bench_pffft_float${CMAKE_EXECUTABLE_SUFFIX}"
  DEPENDS "${benchdir_flt}/directory.txt" build_float
  WORKING_DIRECTORY "${benchdir_flt}"
)

add_custom_target(bench_double
  COMMAND ${CMAKE_COMMAND} -E echo "start benchmark for double"
  COMMAND "${builddir_dbl}/${BUILD_DIR_TO_EXE}bench_pffft_double${CMAKE_EXECUTABLE_SUFFIX}"
  DEPENDS "${benchdir_dbl}/directory.txt" build_double
  WORKING_DIRECTORY "${benchdir_dbl}"
)

add_custom_target(bench ALL
  COMMAND ${CMAKE_COMMAND} -E echo ""
  COMMAND ${CMAKE_COMMAND} -E tar cvz "bench_${BENCH_ID}.tgz" ${benchdir}
  COMMAND ${CMAKE_COMMAND} -E echo ""
  COMMAND ${CMAKE_COMMAND} -E echo "now mail result file bench_${BENCH_ID}.tgz"
  # DEPENDS "${benchdir}/info.txt" "${benchdir}/unix_info.txt"
  DEPENDS "${benchdir}/info.txt" bench_float bench_double "${benchdir}/unix_info.txt"
  WORKING_DIRECTORY "${CMAKE_BINARY_DIR}"
)

add_custom_target(bench_float_tar
  COMMAND ${CMAKE_COMMAND} -E echo ""
  COMMAND ${CMAKE_COMMAND} -E tar cvz "bench_${BENCH_ID}.tgz" ${benchdir}
  COMMAND ${CMAKE_COMMAND} -E echo ""
  COMMAND ${CMAKE_COMMAND} -E echo "now mail result file bench_${BENCH_ID}.tgz"
  DEPENDS "${benchdir}/info.txt" bench_float "${benchdir}/unix_info.txt"
  WORKING_DIRECTORY "${CMAKE_BINARY_DIR}"
)

add_custom_target(bench_double_tar
  COMMAND ${CMAKE_COMMAND} -E echo ""
  COMMAND ${CMAKE_COMMAND} -E tar cvz "bench_${BENCH_ID}.tgz" ${benchdir}
  COMMAND ${CMAKE_COMMAND} -E echo ""
  COMMAND ${CMAKE_COMMAND} -E echo "now mail result file bench_${BENCH_ID}.tgz"
  DEPENDS "${benchdir}/info.txt" bench_double "${benchdir}/unix_info.txt"
  WORKING_DIRECTORY "${CMAKE_BINARY_DIR}"
)

add_custom_target(clean_results
  COMMAND ${CMAKE_COMMAND} -E remove_directory "${builddir_flt}"
  COMMAND ${CMAKE_COMMAND} -E remove_directory "${builddir_dbl}"
  WORKING_DIRECTORY "${CMAKE_BINARY_DIR}"
)

