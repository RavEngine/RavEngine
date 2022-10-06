// SPDX-License-Identifier: BSD-2-Clause

// This code is part of the sfizz library and is licensed under a BSD 2-clause
// license. You should have receive a LICENSE.md file along with the code.
// If not, contact the sfizz maintainers at https://github.com/sfztools/sfizz

#include "st_audiofile.h"
#include <stddef.h>

const char* st_get_type_string(st_audio_file* af)
{
    return st_type_string(st_get_type(af));
}

const char* st_type_string(int type)
{
    const char *type_string = NULL;

    switch (type) {
    case st_audio_file_wav:
        type_string = "WAV";
        break;
    case st_audio_file_flac:
        type_string = "FLAC";
        break;
    case st_audio_file_aiff:
        type_string = "AIFF";
        break;
    case st_audio_file_ogg:
        type_string = "OGG";
        break;
    case st_audio_file_mp3:
        type_string = "MP3";
        break;
    case st_audio_file_other:
        type_string = "other";
        break;
    }

    return type_string;
}
