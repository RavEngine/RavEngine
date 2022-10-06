// SPDX-License-Identifier: BSD-2-Clause

// This code is part of the sfizz library and is licensed under a BSD 2-clause
// license. You should have receive a LICENSE.md file along with the code.
// If not, contact the sfizz maintainers at https://github.com/sfztools/sfizz

#pragma once
#if defined(SFIZZ_USE_SNDFILE)
#include <sndfile.h>
#endif
#include "ghc/fs_std.hpp"
#include <array>
#include <memory>
#include <cstdio>

namespace sfz {

typedef std::array<char, 4> RiffChunkId;

struct RiffChunkInfo {
    size_t index;
    off_t fileOffset;
    RiffChunkId id;
    uint32_t length;
};

#if !defined(SFIZZ_USE_SNDFILE)
/**
   @brief Loop mode, like SF_LOOP_*
 */
enum FileLoopMode {
    LoopNone,
    LoopForward,
    LoopBackward,
    LoopAlternating,
};

/**
   @brief Instrument information, like SF_INSTRUMENT
 */
struct InstrumentInfo {
    int gain;
    int8_t basenote, detune;
    int8_t velocity_lo, velocity_hi;
    int8_t key_lo, key_hi;
    int loop_count;
    struct {
        int mode;
        uint32_t start;
        uint32_t end;
        uint32_t count;
    } loops[16];
};
#else
enum FileLoopMode {
    LoopNone = SF_LOOP_NONE,
    LoopForward = SF_LOOP_FORWARD,
    LoopBackward = SF_LOOP_BACKWARD,
    LoopAlternating = SF_LOOP_ALTERNATING,
};

struct InstrumentInfo : SF_INSTRUMENT {};
#endif

struct WavetableInfo {
    /**
       @brief Size of each successive table in the file
     */
    uint32_t tableSize;
    /**
     * @brief Mode of interpolation between multiple tables
     *
     * 0: none, 1: crossfade, 2: spectral,
     * 3: spectral with fundamental phase set to zero
     * 4: spectral with all phases set to zero
     */
    int crossTableInterpolation;
    /**
     * @brief Whether the wavetable is one-shot (does not cycle)
     */
    bool oneShot;
};

class MetadataReader {
public:
    MetadataReader();
    virtual ~MetadataReader();

    /**
     * @brief Get the number of RIFF chunks in the file
     */
    size_t riffChunkCount() const;
    /**
     * @brief Get the information regarding the n-th RIFF chunk
     */
    const RiffChunkInfo* riffChunk(size_t index) const;
    /**
     * @brief Get the information regarding the RIFF chunk of given identifier
     */
    const RiffChunkInfo* riffChunkById(RiffChunkId id) const;
    /**
     * @brief Read the RIFF data up to the size given (header not included)
     */
    size_t readRiffData(size_t index, void* buffer, size_t count);

    /**
     * @brief Extract the instrument data and convert it to sndfile instrument
     */
    bool extractInstrument(InstrumentInfo& ins);

    /**
     * @brief Extract the RIFF 'smpl' data and convert it to sndfile instrument
     */
    bool extractRiffInstrument(InstrumentInfo& ins);

    /**
     * @brief Extract the AIFF 'INST' data and convert it to sndfile instrument
     */
    bool extractAiffInstrument(InstrumentInfo& ins);

    /**
     * @brief Extract the wavetable information from various relevant RIFF chunks
     */
    bool extractWavetableInfo(WavetableInfo& wt);

    /**
     * @brief Opens the metadata reader and perform initialization.
     *  If the reader is opened already it closes it and reopens.
     *
     * @return true
     * @return false
     */
    bool open();

    /**
     * @brief Check if the reader is already opened
     *
     * @return true
     * @return false
     */
    bool isOpened();

    /**
     * @brief Close the metadata reader
     *
     */
    void close();
protected:
    virtual bool doOpen() { return true; };
    virtual void doClose() {}
    virtual size_t doRead(void* ptr, size_t size, size_t n) = 0;
    virtual int doSeek(long off, int whence) = 0;
    virtual void doRewind() = 0;
    virtual long doTell() = 0;
private:
    struct Impl;
    std::unique_ptr<Impl> impl_;
};

class FileMetadataReader: public MetadataReader {
public:
    FileMetadataReader(const fs::path& path);
    ~FileMetadataReader();
private:
    bool doOpen() override;
    void doClose() override;
    size_t doRead(void* ptr, size_t size, size_t n) override;
    int doSeek(long off, int whence) override;
    void doRewind() override;
    long doTell() override;
    struct Impl;
    std::unique_ptr<Impl> impl_;
};

class MemoryMetadataReader: public MetadataReader {
public:
    MemoryMetadataReader(const void* memory, size_t length);
    ~MemoryMetadataReader();
private:
    bool doOpen() override;
    void doClose() override;
    size_t doRead(void* ptr, size_t size, size_t n) override;
    int doSeek(long off, int whence) override;
    void doRewind() override;
    long doTell() override;
    struct Impl;
    std::unique_ptr<Impl> impl_;
};

} // namespace sfz
