// SPDX-License-Identifier: BSD-2-Clause

// This code is part of the sfizz library and is licensed under a BSD 2-clause
// license. You should have receive a LICENSE.md file along with the code.
// If not, contact the sfizz maintainers at https://github.com/sfztools/sfizz

#pragma once
#include "st_audiofile.h"

class ST_AudioFile {
public:
    constexpr ST_AudioFile() noexcept;
    ~ST_AudioFile() noexcept;

    ST_AudioFile(ST_AudioFile&&) noexcept;
    ST_AudioFile& operator=(ST_AudioFile&&) noexcept;

    ST_AudioFile(const ST_AudioFile&) = delete;
    ST_AudioFile& operator=(const ST_AudioFile&) = delete;

    explicit operator bool() const noexcept;
    void reset(st_audio_file* new_af = nullptr) noexcept;

    bool open_file(const char* filename);
#if defined(_WIN32)
    bool open_file_w(const wchar_t* filename);
#endif
    bool open_memory(const void* memory, size_t length);

    int get_type() const noexcept;
    const char* get_type_string() const noexcept;
    static const char* type_string(int type) noexcept;
    uint32_t get_channels() const noexcept;
    float get_sample_rate() const noexcept;
    uint64_t get_frame_count() const noexcept;

    bool seek(uint64_t frame) noexcept;
    uint64_t read_s16(int16_t* buffer, uint64_t count) noexcept;
    uint64_t read_f32(float* buffer, uint64_t count) noexcept;

    st_audio_file* get_handle() const noexcept;

#if defined(ST_AUDIO_FILE_USE_SNDFILE)
    void* get_sndfile_handle() const noexcept;
    int get_sndfile_format() const noexcept;
#endif

private:
    st_audio_file* af_ = nullptr;
};

//------------------------------------------------------------------------------

inline constexpr ST_AudioFile::ST_AudioFile() noexcept
{
}

inline ST_AudioFile::~ST_AudioFile() noexcept
{
    reset();
}

ST_AudioFile::ST_AudioFile(ST_AudioFile&& other) noexcept
    : af_(other.af_)
{
    other.af_ = nullptr;
}

ST_AudioFile& ST_AudioFile::operator=(ST_AudioFile&& other) noexcept
{
    if (this != &other) {
        if (af_)
            st_close(af_);
        af_ = other.af_;
        other.af_ = nullptr;
    }
    return *this;
}

inline ST_AudioFile::operator bool() const noexcept
{
    return af_ != nullptr;
}

inline void ST_AudioFile::reset(st_audio_file* new_af) noexcept
{
    if (af_ != new_af) {
        if (af_)
            st_close(af_);
        af_ = new_af;
    }
}

bool ST_AudioFile::open_file(const char* filename)
{
    st_audio_file* new_af = st_open_file(filename);
    reset(new_af);
    return new_af != nullptr;
}

bool ST_AudioFile::open_memory(const void* memory, size_t length)
{
    st_audio_file* new_af = st_open_memory(memory, length);
    reset(new_af);
    return new_af != nullptr;
}
#if defined(_WIN32)
inline bool ST_AudioFile::open_file_w(const wchar_t* filename)
{
    st_audio_file* new_af = st_open_file_w(filename);
    reset(new_af);
    return new_af != nullptr;
}
#endif

inline int ST_AudioFile::get_type() const noexcept
{
    return st_get_type(af_);
}

inline const char* ST_AudioFile::get_type_string() const noexcept
{
    return st_get_type_string(af_);
}

inline const char* ST_AudioFile::type_string(int type) noexcept
{
    return st_type_string(type);
}

inline uint32_t ST_AudioFile::get_channels() const noexcept
{
    return st_get_channels(af_);
}

inline float ST_AudioFile::get_sample_rate() const noexcept
{
    return st_get_sample_rate(af_);
}

inline uint64_t ST_AudioFile::get_frame_count() const noexcept
{
    return st_get_frame_count(af_);
}

inline bool ST_AudioFile::seek(uint64_t frame) noexcept
{
    return st_seek(af_, frame);
}

inline uint64_t ST_AudioFile::read_s16(int16_t* buffer, uint64_t count) noexcept
{
    return st_read_s16(af_, buffer, count);
}

inline uint64_t ST_AudioFile::read_f32(float* buffer, uint64_t count) noexcept
{
    return st_read_f32(af_, buffer, count);
}

inline st_audio_file* ST_AudioFile::get_handle() const noexcept
{
    return af_;
}

#if defined(ST_AUDIO_FILE_USE_SNDFILE)
inline void* ST_AudioFile::get_sndfile_handle() const noexcept
{
    return st_get_sndfile_handle(af_);
}

int ST_AudioFile::get_sndfile_format() const noexcept
{
    return st_get_sndfile_format(af_);
}
#endif
