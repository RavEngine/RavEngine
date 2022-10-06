// SPDX-License-Identifier: BSD-2-Clause

// This code is part of the sfizz library and is licensed under a BSD 2-clause
// license. You should have receive a LICENSE.md file along with the code.
// If not, contact the sfizz maintainers at https://github.com/sfztools/sfizz

#include "st_audiofile.h"
#if !defined(ST_AUDIO_FILE_USE_SNDFILE)
#include "st_audiofile_libs.h"
#include <stdlib.h>

struct st_audio_file {
    int type;
    union {
        drwav *wav;
        drflac *flac;
        AIFF_Ref aiff;
        drmp3 *mp3;
        stb_vorbis* ogg;
    };

    union {
        struct { uint32_t channels; float sample_rate; uint64_t frames; } aiff;
        struct { uint64_t frames; } mp3;
        struct { uint32_t channels; float sample_rate; uint64_t frames; } ogg;
    } cache;

    union {
        stb_vorbis_alloc ogg;
    } alloc;
};

static st_audio_file* st_generic_open_file(const void* filename, int widepath)
{
#if !defined(_WIN32)
    if (widepath)
        return NULL;
#endif

    st_audio_file* af = (st_audio_file*)malloc(sizeof(st_audio_file));
    if (!af)
        return NULL;

    // Try WAV
    {
        af->wav = (drwav*)malloc(sizeof(drwav));
        if (!af->wav) {
            free(af);
            return NULL;
        }
        drwav_bool32 ok =
#if defined(_WIN32)
            widepath ? drwav_init_file_w(af->wav, (const wchar_t*)filename, NULL) :
#endif
            drwav_init_file(af->wav, (const char*)filename, NULL);
        if (!ok)
            free(af->wav);
        else {
            af->type = st_audio_file_wav;
            return af;
        }
    }

    // Try FLAC
    {
        af->flac =
#if defined(_WIN32)
            widepath ? drflac_open_file_w((const wchar_t*)filename, NULL) :
#endif
            drflac_open_file((const char*)filename, NULL);
        if (af->flac) {
            af->type = st_audio_file_flac;
            return af;
        }
    }

    // Try AIFF
    {
        af->aiff =
#if defined(_WIN32)
            widepath ? AIFF_OpenFileW((const wchar_t*)filename, F_RDONLY) :
#endif
            AIFF_OpenFile((const char*)filename, F_RDONLY);
        if (af->aiff) {
            int channels;
            double sample_rate;
            uint64_t frames;
            if (AIFF_GetAudioFormat(af->aiff, &frames, &channels, &sample_rate, NULL, NULL) == -1) {
                AIFF_CloseFile(af->aiff);
                free(af);
                return NULL;
            }
            af->cache.aiff.channels = (uint32_t)channels;
            af->cache.aiff.sample_rate = (float)sample_rate;
            af->cache.aiff.frames = frames;
            af->type = st_audio_file_aiff;
            return af;
        }
    }

    // Try OGG
    {
        int err = 0;
        char* alloc_buffer = NULL;
        int alloc_size = 0;
        const int alloc_max_size = 16 * 1024 * 1024;

        af->alloc.ogg.alloc_buffer = NULL;
        af->alloc.ogg.alloc_buffer_length_in_bytes = 0;

        int try_again;
        do {
            af->ogg =
#if defined(_WIN32)
                widepath ? stb_vorbis_open_filename_w((const wchar_t*)filename, NULL, NULL) :
#endif
                stb_vorbis_open_filename((const char*)filename, &err, &af->alloc.ogg);
            try_again = 0;
            if (!af->ogg && err == VORBIS_outofmem) {
                int next_size = alloc_size ? (int)(alloc_size * 3L / 2) : (128 * 1024);
                if (next_size <= alloc_max_size) {
                    free(alloc_buffer);
                    alloc_size = next_size;
                    alloc_buffer = (char*)malloc(alloc_size);
                    af->alloc.ogg.alloc_buffer = alloc_buffer;
                    af->alloc.ogg.alloc_buffer_length_in_bytes = alloc_size;
                    try_again = alloc_buffer != NULL;
                }
            }
        } while (try_again);

        if (!af->ogg)
            free(alloc_buffer);
        else {
            af->cache.ogg.frames = stb_vorbis_stream_length_in_samples(af->ogg);
            if (af->cache.ogg.frames == 0) {
                stb_vorbis_close(af->ogg);
                free(alloc_buffer);
                free(af);
                return NULL;
            }
            stb_vorbis_info info = stb_vorbis_get_info(af->ogg);
            af->cache.ogg.channels = info.channels;
            af->cache.ogg.sample_rate = info.sample_rate;
            af->type = st_audio_file_ogg;
            return af;
        }
    }

    // Try MP3
    {
        af->mp3 = (drmp3*)malloc(sizeof(drmp3));
        if (!af->mp3) {
            free(af);
            return NULL;
        }
        drmp3_bool32 ok =
#if defined(_WIN32)
            widepath ? drmp3_init_file_w(af->mp3, (const wchar_t*)filename, NULL) :
#endif
            drmp3_init_file(af->mp3, (const char*)filename, NULL);
        if (!ok)
            free(af->mp3);
        else {
            af->cache.mp3.frames = drmp3_get_pcm_frame_count(af->mp3);
            if (af->cache.mp3.frames == 0) {
                free(af->mp3);
                free(af);
                return NULL;
            }
            af->type = st_audio_file_mp3;
            return af;
        }
    }

    free(af);
    return NULL;
}

st_audio_file* st_open_memory(const void* memory, size_t length)
{
    st_audio_file* af = (st_audio_file*)malloc(sizeof(st_audio_file));
    if (!af)
        return NULL;

    // Try WAV
    {
        af->wav = (drwav*)malloc(sizeof(drwav));
        if (!af->wav) {
            free(af);
            return NULL;
        }
        drwav_bool32 ok =
            drwav_init_memory(af->wav, memory, length, NULL);
        if (!ok)
            free(af->wav);
        else {
            af->type = st_audio_file_wav;
            return af;
        }
    }

    // Try FLAC
    {
        af->flac =
            drflac_open_memory(memory, length, NULL);
        if (af->flac) {
            af->type = st_audio_file_flac;
            return af;
        }
    }

    // Try OGG
    {
        int err = 0;
        char* alloc_buffer = NULL;
        int alloc_size = 0;
        const int alloc_max_size = 16 * 1024 * 1024;

        af->alloc.ogg.alloc_buffer = NULL;
        af->alloc.ogg.alloc_buffer_length_in_bytes = 0;

        int try_again;
        do {
            af->ogg =
                stb_vorbis_open_memory(memory, length, &err, &af->alloc.ogg);
            try_again = 0;
            if (!af->ogg && err == VORBIS_outofmem) {
                int next_size = alloc_size ? (int)(alloc_size * 3L / 2) : (128 * 1024);
                if (next_size <= alloc_max_size) {
                    free(alloc_buffer);
                    alloc_size = next_size;
                    alloc_buffer = (char*)malloc(alloc_size);
                    af->alloc.ogg.alloc_buffer = alloc_buffer;
                    af->alloc.ogg.alloc_buffer_length_in_bytes = alloc_size;
                    try_again = alloc_buffer != NULL;
                }
            }
        } while (try_again);

        if (!af->ogg)
            free(alloc_buffer);
        else {
            af->cache.ogg.frames = stb_vorbis_stream_length_in_samples(af->ogg);
            if (af->cache.ogg.frames == 0) {
                stb_vorbis_close(af->ogg);
                free(alloc_buffer);
                free(af);
                return NULL;
            }
            stb_vorbis_info info = stb_vorbis_get_info(af->ogg);
            af->cache.ogg.channels = info.channels;
            af->cache.ogg.sample_rate = info.sample_rate;
            af->type = st_audio_file_ogg;
            return af;
        }
    }

    // Try MP3
    {
        af->mp3 = (drmp3*)malloc(sizeof(drmp3));
        if (!af->mp3) {
            free(af);
            return NULL;
        }
        drmp3_bool32 ok = drmp3_init_memory(af->mp3, memory, length, NULL);
        if (!ok)
            free(af->mp3);
        else {
            af->cache.mp3.frames = drmp3_get_pcm_frame_count(af->mp3);
            if (af->cache.mp3.frames == 0) {
                free(af->mp3);
                free(af);
                return NULL;
            }
            af->type = st_audio_file_mp3;
            return af;
        }
    }

    free(af);
    return NULL;
}

st_audio_file* st_open_file(const char* filename)
{
    return st_generic_open_file(filename, 0);
}

#if defined(_WIN32)
st_audio_file* st_open_file_w(const wchar_t* filename)
{
    return st_generic_open_file(filename, 1);
}
#endif

void st_close(st_audio_file* af)
{
    switch (af->type) {
    case st_audio_file_wav:
        drwav_uninit(af->wav);
        free(af->wav);
        break;
    case st_audio_file_flac:
        drflac_close(af->flac);
        break;
    case st_audio_file_aiff:
        AIFF_CloseFile(af->aiff);
        break;
    case st_audio_file_ogg:
        stb_vorbis_close(af->ogg);
        free(af->alloc.ogg.alloc_buffer);
        break;
    case st_audio_file_mp3:
        drmp3_uninit(af->mp3);
        free(af->mp3);
        break;
    }

    free(af);
}

int st_get_type(st_audio_file* af)
{
    return af->type;
}

uint32_t st_get_channels(st_audio_file* af)
{
    uint32_t channels = 0;

    switch (af->type) {
    case st_audio_file_wav:
        channels = af->wav->channels;
        break;
    case st_audio_file_flac:
        channels = af->flac->channels;
        break;
    case st_audio_file_aiff:
        channels = af->cache.aiff.channels;
        break;
    case st_audio_file_ogg:
        channels = af->cache.ogg.channels;
        break;
    case st_audio_file_mp3:
        channels = af->mp3->channels;
        break;
    }

    return channels;
}

float st_get_sample_rate(st_audio_file* af)
{
    float sample_rate = 0;

    switch (af->type) {
    case st_audio_file_wav:
        sample_rate = af->wav->sampleRate;
        break;
    case st_audio_file_flac:
        sample_rate = af->flac->sampleRate;
        break;
    case st_audio_file_aiff:
        sample_rate = af->cache.aiff.sample_rate;
        break;
    case st_audio_file_ogg:
        sample_rate = af->cache.ogg.sample_rate;
        break;
    case st_audio_file_mp3:
        sample_rate = af->mp3->sampleRate;
        break;
    }

    return sample_rate;
}

uint64_t st_get_frame_count(st_audio_file* af)
{
    uint64_t frames = 0;

    switch (af->type) {
    case st_audio_file_wav:
        frames = af->wav->totalPCMFrameCount;
        break;
    case st_audio_file_flac:
        frames = af->flac->totalPCMFrameCount;
        break;
    case st_audio_file_aiff:
        frames = af->cache.aiff.frames;
        break;
    case st_audio_file_ogg:
        frames = af->cache.ogg.frames;
        break;
    case st_audio_file_mp3:
        frames = af->cache.mp3.frames;
        break;
    }

    return frames;
}

bool st_seek(st_audio_file* af, uint64_t frame)
{
    bool success = false;

    switch (af->type) {
    case st_audio_file_wav:
        success = drwav_seek_to_pcm_frame(af->wav, frame);
        break;
    case st_audio_file_flac:
        success = drflac_seek_to_pcm_frame(af->flac, frame);
        break;
    case st_audio_file_aiff:
        success = AIFF_Seek(af->aiff, frame) != -1;
        break;
    case st_audio_file_ogg:
        success = stb_vorbis_seek(af->ogg, (unsigned)frame) != 0;
        break;
    case st_audio_file_mp3:
        success = drmp3_seek_to_pcm_frame(af->mp3, frame);
        break;
    }

    return success;
}

uint64_t st_read_s16(st_audio_file* af, int16_t* buffer, uint64_t count)
{
    switch (af->type) {
    case st_audio_file_wav:
        count = drwav_read_pcm_frames_s16(af->wav, count, buffer);
        break;
    case st_audio_file_flac:
        count = drflac_read_pcm_frames_s16(af->flac, count, buffer);
        break;
    case st_audio_file_aiff:
        {
            uint32_t channels = af->cache.aiff.channels;
            unsigned samples = AIFF_ReadSamples16Bit(af->aiff, buffer, (unsigned)(channels * count));
            count = ((int)samples != -1) ? (samples / channels) : 0;
        }
        break;
    case st_audio_file_ogg:
        count = stb_vorbis_get_samples_short_interleaved(
            af->ogg, af->cache.ogg.channels, buffer,
            count * af->cache.ogg.channels);
        break;
    case st_audio_file_mp3:
        count = drmp3_read_pcm_frames_s16(af->mp3, count, buffer);
        break;
    }

    return count;
}

uint64_t st_read_f32(st_audio_file* af, float* buffer, uint64_t count)
{
    switch (af->type) {
    case st_audio_file_wav:
        count = drwav_read_pcm_frames_f32(af->wav, count, buffer);
        break;
    case st_audio_file_flac:
        count = drflac_read_pcm_frames_f32(af->flac, count, buffer);
        break;
    case st_audio_file_aiff:
        {
            uint32_t channels = af->cache.aiff.channels;
            unsigned samples = AIFF_ReadSamplesFloat(af->aiff, buffer, (unsigned)(channels * count));
            count = ((int)samples != -1) ? (samples / channels) : 0;
        }
        break;
    case st_audio_file_ogg:
        count = stb_vorbis_get_samples_float_interleaved(
            af->ogg, af->cache.ogg.channels, buffer,
            count * af->cache.ogg.channels);
        break;
    case st_audio_file_mp3:
        count = drmp3_read_pcm_frames_f32(af->mp3, count, buffer);
        break;
    }

    return count;
}

#endif // !defined(ST_AUDIO_FILE_USE_SNDFILE)
