// SPDX-License-Identifier: BSD-2-Clause

// This code is part of the sfizz library and is licensed under a BSD 2-clause
// license. You should have receive a LICENSE.md file along with the code.
// If not, contact the sfizz maintainers at https://github.com/sfztools/sfizz

#pragma once
#include "dr_wav.h"
#include "dr_flac.h"
#include "dr_mp3.h"
#if !defined(STB_VORBIS_HEADER_ONLY)
#   define STB_VORBIS_HEADER_ONLY 1
#elif STB_VORBIS_HEADER_ONLY == 0
#   undef STB_VORBIS_HEADER_ONLY
#endif
#include "stb_vorbis.c"
#include "libaiff/libaiff.h"

#if defined(_WIN32)
#include <wchar.h>
stb_vorbis* stb_vorbis_open_filename_w(const wchar_t* filename, int* error, const stb_vorbis_alloc* alloc);
#endif
