// SPDX-License-Identifier: BSD-2-Clause

// This code is part of the sfizz library and is licensed under a BSD 2-clause
// license. You should have receive a LICENSE.md file along with the code.
// If not, contact the sfizz maintainers at https://github.com/sfztools/sfizz

#include "AudioReader.h"
#include "FileMetadata.h"
#include <absl/memory/memory.h>
#include <st_audiofile.hpp>
#if defined(SFIZZ_USE_SNDFILE)
#include <sndfile.h>
#endif
#include <algorithm>

namespace sfz {

class BasicSndfileReader : public AudioReader {
public:
    explicit BasicSndfileReader(ST_AudioFile handle, std::unique_ptr<MetadataReader> mdReader)
    : handle_(std::move(handle)), mdReader_(std::move(mdReader)) {}
    virtual ~BasicSndfileReader() {}

    int format() const override;
    int64_t frames() const override;
    unsigned channels() const override;
    unsigned sampleRate() const override;
    bool getInstrumentInfo(InstrumentInfo& instrument) override;
    bool getWavetableInfo(WavetableInfo& instrument) override;
protected:
    ST_AudioFile handle_;
    std::unique_ptr<MetadataReader> mdReader_;
};

int BasicSndfileReader::format() const
{
    return handle_.get_type();
}

int64_t BasicSndfileReader::frames() const
{
    return handle_.get_frame_count();
}

unsigned BasicSndfileReader::channels() const
{
    return handle_.get_channels();
}

unsigned BasicSndfileReader::sampleRate() const
{
    return handle_.get_sample_rate();
}

bool BasicSndfileReader::getWavetableInfo(WavetableInfo& wt)
{
    if (!mdReader_)
        return false;

    if (!mdReader_->isOpened())
        mdReader_->open();

    if (mdReader_->isOpened())
        return mdReader_->extractWavetableInfo(wt);

    return false;
};

bool BasicSndfileReader::getInstrumentInfo(InstrumentInfo& instrument)
{
#if defined(SFIZZ_USE_SNDFILE)
    SNDFILE* sndfile = reinterpret_cast<SNDFILE*>(handle_.get_sndfile_handle());
    SF_INSTRUMENT* sfins = instrument;
    if (sf_command(sndfile, SFC_GET_INSTRUMENT, sfins, sizeof(SF_INSTRUMENT)) == SF_TRUE)
        return true;
#else
    if (!mdReader_)
        return false;

    if (!mdReader_->isOpened())
        mdReader_->open();

    if (mdReader_->isOpened())
        return mdReader_->extractInstrument(instrument);
#endif
    return false;
}

//------------------------------------------------------------------------------

/**
 * @brief Audio file reader in forward direction
 */
class ForwardReader : public BasicSndfileReader {
public:
    explicit ForwardReader(ST_AudioFile handle, std::unique_ptr<MetadataReader> mdReader);
    AudioReaderType type() const override;
    size_t readNextBlock(float* buffer, size_t frames) override;
};

ForwardReader::ForwardReader(ST_AudioFile handle, std::unique_ptr<MetadataReader> mdReader)
    : BasicSndfileReader(std::move(handle), std::move(mdReader))
{
}

AudioReaderType ForwardReader::type() const
{
    return AudioReaderType::Forward;
}

size_t ForwardReader::readNextBlock(float* buffer, size_t frames)
{
    uint64_t readFrames = handle_.read_f32(buffer, frames);
    if (frames <= 0)
        return 0;

    return readFrames;
}

//------------------------------------------------------------------------------

template <size_t N, class T = float>
struct AudioFrame {
    T samples[N];
};

/**
 * @brief Reorder a sequence of frames in reverse
 */
static void reverse_frames(float* data, size_t frames, unsigned channels)
{
    switch (channels) {

#define SPECIALIZE_FOR(N)                                           \
        case N:                                                     \
            std::reverse(                                           \
                reinterpret_cast<AudioFrame<N> *>(data),            \
                reinterpret_cast<AudioFrame<N> *>(data) + frames);  \
            break

    SPECIALIZE_FOR(1);
    SPECIALIZE_FOR(2);

    default:
        for (size_t i = 0; i < frames / 2; ++i) {
            size_t j = frames - 1 - i;
            float* frame1 = &data[i * channels];
            float* frame2 = &data[j * channels];
            for (unsigned c = 0; c < channels; ++c)
                std::swap(frame1[c], frame2[c]);
        }
        break;

#undef SPECIALIZE_FOR
    }
}

//------------------------------------------------------------------------------

/**
 * @brief Audio file reader in reverse direction, for fast-seeking formats
 */
class ReverseReader : public BasicSndfileReader {
public:
    explicit ReverseReader(ST_AudioFile handle, std::unique_ptr<MetadataReader> mdReader);
    AudioReaderType type() const override;
    size_t readNextBlock(float* buffer, size_t frames) override;

private:
    uint64_t position_ {};
};

ReverseReader::ReverseReader(ST_AudioFile handle, std::unique_ptr<MetadataReader> mdReader)
    : BasicSndfileReader(std::move(handle), std::move(mdReader))
{
    position_ = handle_.get_frame_count();
}

AudioReaderType ReverseReader::type() const
{
    return AudioReaderType::Reverse;
}

size_t ReverseReader::readNextBlock(float* buffer, size_t frames)
{
    uint64_t position = position_;
    const unsigned channels = handle_.get_channels();

    const uint64_t readFrames = std::min<uint64_t>(frames, position);
    if (readFrames <= 0)
        return false;

    position -= readFrames;
    if (!handle_.seek(position) ||
        handle_.read_f32(buffer, readFrames) != readFrames)
        return false;

    position_ = position;
    reverse_frames(buffer, readFrames, channels);
    return readFrames;
}

//------------------------------------------------------------------------------

/**
 * @brief Audio file reader in reverse direction, for slow-seeking formats
 */
class NoSeekReverseReader : public BasicSndfileReader {
public:
    explicit NoSeekReverseReader(ST_AudioFile handle, std::unique_ptr<MetadataReader> mdReader);
    AudioReaderType type() const override;
    size_t readNextBlock(float* buffer, size_t frames) override;

private:
    void readWholeFile();

private:
    std::unique_ptr<float[]> fileBuffer_;
    uint64_t fileFramesLeft_ { 0 };
};

NoSeekReverseReader::NoSeekReverseReader(ST_AudioFile handle, std::unique_ptr<MetadataReader> mdReader)
    : BasicSndfileReader(std::move(handle), std::move(mdReader))
{
}

AudioReaderType NoSeekReverseReader::type() const
{
    return AudioReaderType::NoSeekReverse;
}

size_t NoSeekReverseReader::readNextBlock(float* buffer, size_t frames)
{
    float* fileBuffer = fileBuffer_.get();
    if (!fileBuffer) {
        readWholeFile();
        fileBuffer = fileBuffer_.get();
    }

    const unsigned channels = handle_.get_channels();
    const uint64_t fileFramesLeft = fileFramesLeft_;
    uint64_t readFrames = std::min<uint64_t>(frames, fileFramesLeft);
    if (readFrames <= 0)
        return 0;

    std::copy(
        &fileBuffer[channels * (fileFramesLeft - readFrames)],
        &fileBuffer[channels * fileFramesLeft], buffer);
    reverse_frames(buffer, readFrames, channels);

    fileFramesLeft_ = fileFramesLeft - readFrames;
    return readFrames;
}

void NoSeekReverseReader::readWholeFile()
{
    const uint64_t frames = handle_.get_frame_count();
    const unsigned channels = handle_.get_channels();
    float* fileBuffer = new float[channels * frames];
    fileBuffer_.reset(fileBuffer);
    fileFramesLeft_ = handle_.read_f32(fileBuffer, frames);
}

//------------------------------------------------------------------------------

#if defined(SFIZZ_USE_SNDFILE)
const std::error_category& sndfile_category()
{
    class sndfile_category : public std::error_category {
    public:
        const char* name() const noexcept override
        {
            return "sndfile";
        }

        std::string message(int condition) const override
        {
            const char* str = sf_error_number(condition);
            return str ? str : "";
        }
    };

    static const sndfile_category cat;
    return cat;
}
#endif

const std::error_category& undetailed_category()
{
    class undetailed_category : public std::error_category {
    public:
        const char* name() const noexcept override
        {
            return "undetailed";
        }

        std::string message(int condition) const override
        {
            return (condition == 0) ? "success" : "failure";
        }
    };

    static const undetailed_category cat;
    return cat;
}

//------------------------------------------------------------------------------
static fs::path emptyPath_ {};

class DummyAudioReader : public AudioReader {
public:
    explicit DummyAudioReader(AudioReaderType type) : type_(type) {}
    AudioReaderType type() const override { return type_; }
    int format() const override { return 0; }
    int64_t frames() const override { return 0; }
    unsigned channels() const override { return 1; }
    unsigned sampleRate() const override { return 44100; }
    size_t readNextBlock(float*, size_t) override { return 0; }
    bool getInstrumentInfo(InstrumentInfo& ) override { return false; }
private:
    AudioReaderType type_ {};
};

//------------------------------------------------------------------------------

#if defined(SFIZZ_USE_SNDFILE)
static bool formatHasFastSeeking(int format)
{
    bool fast;

    const int type = format & SF_FORMAT_TYPEMASK;
    const int subtype = format & SF_FORMAT_SUBMASK;

    switch (type) {
    case SF_FORMAT_WAV:
    case SF_FORMAT_AIFF:
    case SF_FORMAT_AU:
    case SF_FORMAT_RAW:
    case SF_FORMAT_WAVEX:
        // TODO: list more PCM formats that support fast seeking
        fast = subtype >= SF_FORMAT_PCM_S8 && subtype <= SF_FORMAT_DOUBLE;
        break;
    case SF_FORMAT_FLAC:
        // seeking has acceptable overhead
        fast = true;
        break;
    case SF_FORMAT_OGG:
        // ogg is prohibitively slow at seeking (possibly others)
        // cf. https://github.com/erikd/libsndfile/issues/491
        fast = false;
        break;
    default:
        fast = false;
        break;
    }

    return fast;
}
#endif

static AudioReaderPtr createAudioReaderWithHandle(ST_AudioFile handle, std::unique_ptr<MetadataReader> mdReader, bool reverse, std::error_code* ec)
{
    AudioReaderPtr reader;

    if (ec)
        ec->clear();

    if (!handle) {
        if (ec)
            *ec = std::error_code(1, undetailed_category());
        reader.reset(new DummyAudioReader(reverse ? AudioReaderType::Reverse : AudioReaderType::Forward));
    }
    else if (!reverse)
        reader.reset(new ForwardReader(std::move(handle), std::move(mdReader)));
    else {
#if defined(SFIZZ_USE_SNDFILE)
        bool hasFastSeeking = formatHasFastSeeking(handle.get_sndfile_format());
#else
        bool hasFastSeeking = true;
#endif
        if (hasFastSeeking)
            reader.reset(new ReverseReader(std::move(handle), std::move(mdReader)));
        else
            reader.reset(new NoSeekReverseReader(std::move(handle), std::move(mdReader)));
    }

    return reader;
}

AudioReaderPtr createAudioReader(const fs::path& path, bool reverse, std::error_code* ec)
{
    ST_AudioFile handle;
#if defined(_WIN32)
    handle.open_file_w(path.wstring().c_str());
#else
    handle.open_file(path.c_str());
#endif
    return createAudioReaderWithHandle(std::move(handle),
        absl::make_unique<FileMetadataReader>(path), reverse, ec);
}

AudioReaderPtr createAudioReaderFromMemory(const void* memory, size_t length, bool reverse, std::error_code* ec)
{
    ST_AudioFile handle;
    handle.open_memory(memory, length);
    return createAudioReaderWithHandle(std::move(handle),
        absl::make_unique<MemoryMetadataReader>(memory, length), reverse, ec);
}

} // namespace sfz
