/*
Copyright (c) 2019, Dimitri Diakopoulos All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are met:

* Redistributions of source code must retain the above copyright notice, this
  list of conditions and the following disclaimer.

* Redistributions in binary form must reproduce the above copyright notice,
  this list of conditions and the following disclaimer in the documentation
  and/or other materials provided with the distribution.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#undef VERSION
#define VERSION "1.3.1"
    
#define FLAC__NO_DLL 1
#define FLAC__USE_VISIBILITY_ATTR 1

#if (_MSC_VER)
#pragma warning (push)
#pragma warning (disable: 181 111 4267 4996 4244 4701 4702 4133 4100 4127 4206 4312 4505 4365 4005 4013 4334)
#ifndef _WIN32
#define _WIN32
#endif
#endif

#if defined(__APPLE__) && defined(__MACH__)
#define FLAC__SYS_DARWIN 1
#endif
    
#ifndef SIZE_MAX
#define SIZE_MAX (size_t) (-1)
#endif
    
#ifdef __clang__
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wconversion"
#pragma clang diagnostic ignored "-Wshadow"
#pragma clang diagnostic ignored "-Wdeprecated-register"
#endif
    
#if CPU_X86
#ifdef __i386__
#define FLAC__CPU_IA32 1
#endif
#ifdef __x86_64__
#define FLAC__CPU_X86_64 1
#endif
#define FLAC__HAS_X86INTRIN 1
#endif
    
// Ensure libflac can use non-standard <stdint> types
#undef __STDC_LIMIT_MACROS
#define __STDC_LIMIT_MACROS 1
    
#if defined(__APPLE__) && defined(__MACH__)
#define flac_max(a,b) ((a) > (b) ? a : b)
#define flac_min(a,b) ((a) < (b) ? a : b)
#elif defined(_MSC_VER)
#include <stdlib.h>
#define flac_max(a,b) __max(a,b)
#define flac_min(a,b) __min(a,b)
#endif
    
#define HAVE_LROUND 1

#include "FLAC/all.h"

#if defined(_MSC_VER)
#include "FLAC/src/win_utf8_io.c"
#endif

#include "FLAC/src/bitmath.c"
#include "FLAC/src/bitreader.c"
#include "FLAC/src/bitwriter.c"
#include "FLAC/src/cpu.c"
#include "FLAC/src/crc.c"
#include "FLAC/src/fixed.c"
#include "FLAC/src/float.c"
#include "FLAC/src/format.c"
#include "FLAC/src/lpc.c"
#include "FLAC/src/md5.c"
#include "FLAC/src/memory.c"
#include "FLAC/src/stream_decoder.c"
#include "FLAC/src/window.c"

#undef VERSION

#ifdef __clang__
#pragma clang diagnostic pop
#endif

#if (_MSC_VER)
#pragma warning (pop)
#endif
