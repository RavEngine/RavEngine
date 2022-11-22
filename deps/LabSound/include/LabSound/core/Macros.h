// SPDX-License-Identifier: BSD-2-Clause
// Copyright (C) 2015+, The LabSound Authors. All rights reserved.

#ifndef labsound_macros_h
#define labsound_macros_h

#define LABSOUND_DEFAULT_SAMPLERATE 48000.0f
#define LABSOUND_DEFAULT_CHANNELS (uint32_t) lab::Channels::Stereo

#if (defined(__linux) || defined(__unix) || defined(__posix) || defined(__LINUX__) || defined(__linux__))
  #define LABSOUND_PLATFORM_LINUX 1
#elif (defined(_WIN64) || defined(_WIN32) || defined(__CYGWIN32__) || defined(__MINGW32__))
  #define LABSOUND_PLATFORM_WINDOWS 1
#elif (defined(MACOSX) || defined(__DARWIN__) || defined(__APPLE__))
  #define LABSOUND_PLATFORM_OSX 1
#endif

#if (defined(WIN_32) || defined(__i386__) || defined(i386) || defined(__x86__))
  #define LABSOUND_ARCH_32 1
#elif (defined(__amd64) || defined(__amd64__) || defined(__x86_64) || defined(__x86_64__) || defined(_M_X64) || defined(__ia64__) || defined(_M_IA64))
  #define LABSOUND_ARCH_64 1
#endif

#if (defined(__clang__))
  #define LABSOUND_COMPILER_CLANG 1
#elif (defined(__GNUC__))
  #define LABSOUND_COMPILER_GCC 1
#elif (defined _MSC_VER)
  #define LABSOUND_COMPILER_VISUAL_STUDIO 1
#endif

#if ((_M_IX86_FP) && (_M_IX86_FP >= 2)) || (_M_AMD64) || defined(_M_X64)
  #define __SSE2__
#endif

#if defined(LABSOUND_COMPILER_VISUAL_STUDIO)
  #include <stdint.h>
  #define _USE_MATH_DEFINES
  #include <cmath>
  #include <math.h>
#endif

#if defined(__ARM_NEON__)
  #define ARM_NEON_INTRINSICS 1
#endif

#if defined(LABSOUND_PLATFORM_OSX) || defined(LABSOUND_PLATFORM_LINUX)
#  include <cmath>
#  include <math.h>
#endif

#if defined(LABSOUND_PLATFORM_WINDOWS)
  #define USE_KISS_FFT
#endif

#define LAB_PI          3.1415926535897931
#define LAB_HALF_PI     1.5707963267948966
#define LAB_QUARTER_PI  0.7853981633974483
#define LAB_TWO_PI      6.2831853071795862
#define LAB_TAU         6.2831853071795862
#define LAB_INV_PI      0.3183098861837907
#define LAB_INV_TWO_PI  0.1591549430918953
#define LAB_INV_HALF_PI 0.6366197723675813
#define LAB_SQRT_2      1.4142135623730951
#define LAB_INV_SQRT_2  0.7071067811865475
#define LAB_LN_2        0.6931471805599453
#define LAB_INV_LN_2    1.4426950408889634
#define LAB_LN_10       2.3025850929940459
#define LAB_INV_LN_10   0.4342944819032517

#endif  // end labsound_macros_h
