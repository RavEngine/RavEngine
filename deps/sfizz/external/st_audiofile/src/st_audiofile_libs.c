// SPDX-License-Identifier: BSD-2-Clause

// This code is part of the sfizz library and is licensed under a BSD 2-clause
// license. You should have receive a LICENSE.md file along with the code.
// If not, contact the sfizz maintainers at https://github.com/sfztools/sfizz

#define DR_WAV_IMPLEMENTATION
#define DR_FLAC_IMPLEMENTATION
#define DR_MP3_IMPLEMENTATION
#define STB_VORBIS_HEADER_ONLY 0
#include "st_audiofile_libs.h"

#if defined(_WIN32)
stb_vorbis* stb_vorbis_open_filename_w(const wchar_t* filename, int* error, const stb_vorbis_alloc* alloc)
{
   FILE* f;
#if defined(_WIN32) && defined(__STDC_WANT_SECURE_LIB__)
   if (0 != _wfopen_s(&f, filename, L"rb"))
      f = NULL;
#else
   f = _wfopen(filename, L"rb");
#endif
   if (f)
      return stb_vorbis_open_file(f, TRUE, error, alloc);
   if (error)
       *error = VORBIS_file_open_failure;
   return NULL;
}
#endif
