// SPDX-License-Identifier: BSD-2-Clause

// This code is part of the sfizz library and is licensed under a BSD 2-clause
// license. You should have receive a LICENSE.md file along with the code.
// If not, contact the sfizz maintainers at https://github.com/sfztools/sfizz

#pragma once
#include "absl/types/span.h"
#include "ghc/fs_std.hpp"
#include <st_audiofile.h>
#include <system_error>
#include <memory>
#include <cstdio>

namespace sfz {
struct InstrumentInfo;
struct WavetableInfo;

/**
 * @brief Designation of a particular kind of audio reader
 */
enum class AudioReaderType {
    //! Reader in forward direction
    Forward,
    //! Reader in reverse direction
    Reverse,
    //! Reader in reverse direction, operating on a whole file instead of seeking
    NoSeekReverse,
};

/**
 * @brief Reader of audio file data
 */
class AudioReader {
public:
    virtual ~AudioReader() {}
    virtual AudioReaderType type() const = 0;
    virtual int format() const = 0;
    virtual int64_t frames() const = 0;
    virtual unsigned channels() const = 0;
    virtual unsigned sampleRate() const = 0;
    virtual size_t readNextBlock(float* buffer, size_t frames) = 0;
    virtual bool getInstrumentInfo(InstrumentInfo&) { return false; };
    virtual bool getWavetableInfo(WavetableInfo&) { return false; };
};

typedef std::unique_ptr<AudioReader> AudioReaderPtr;

/**
 * @brief Create a file reader of detected type.
 */
AudioReaderPtr createAudioReader(const fs::path& path, bool reverse, std::error_code* ec = nullptr);

/**
 * @brief Create a memory reader of detected type.
 */
AudioReaderPtr createAudioReaderFromMemory(const void* memory, size_t length, bool reverse, std::error_code* ec = nullptr);

} // namespace sfz
