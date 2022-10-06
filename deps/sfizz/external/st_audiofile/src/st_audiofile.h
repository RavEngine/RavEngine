// SPDX-License-Identifier: BSD-2-Clause

// This code is part of the sfizz library and is licensed under a BSD 2-clause
// license. You should have receive a LICENSE.md file along with the code.
// If not, contact the sfizz maintainers at https://github.com/sfztools/sfizz

#pragma once
#if defined(ST_AUDIO_FILE_USE_SNDFILE)
#if defined(_WIN32)
#define ENABLE_SNDFILE_WINDOWS_PROTOTYPES 1
#include <windows.h>
#endif
#include <sndfile.h>
#endif
#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>
#if defined(_WIN32)
#include <wchar.h>
#endif

#ifdef __cplusplus
extern "C" {
#endif

typedef struct st_audio_file st_audio_file;

typedef enum st_audio_file_type {
    st_audio_file_wav,
    st_audio_file_flac,
    st_audio_file_aiff,
    st_audio_file_ogg,
    st_audio_file_mp3,
    st_audio_file_other,
} st_audio_file_type;

st_audio_file* st_open_memory(const void* memory, size_t length);
st_audio_file* st_open_file(const char* filename);
#if defined(_WIN32)
st_audio_file* st_open_file_w(const wchar_t* filename);
#endif
void st_close(st_audio_file* af);
int st_get_type(st_audio_file* af);
const char* st_get_type_string(st_audio_file* af);
const char* st_type_string(int type);
uint32_t st_get_channels(st_audio_file* af);
float st_get_sample_rate(st_audio_file* af);
uint64_t st_get_frame_count(st_audio_file* af);
bool st_seek(st_audio_file* af, uint64_t frame);
uint64_t st_read_s16(st_audio_file* af, int16_t* buffer, uint64_t count);
uint64_t st_read_f32(st_audio_file* af, float* buffer, uint64_t count);

#if defined(ST_AUDIO_FILE_USE_SNDFILE)
SNDFILE* st_get_sndfile_handle(st_audio_file* af);
int st_get_sndfile_format(st_audio_file* af);
#endif

#ifdef __cplusplus
} // extern "C"
#endif
