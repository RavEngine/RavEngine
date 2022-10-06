// SPDX-License-Identifier: BSD-2-Clause

// This code is part of the sfizz library and is licensed under a BSD 2-clause
// license. You should have receive a LICENSE.md file along with the code.
// If not, contact the sfizz maintainers at https://github.com/sfztools/sfizz

// Note: Based on some format research from Surge synthesizer
//       made by Paul Walker and Mario Kruselj
//       cf. Surge src/common/WavSupport.cpp

#include "FileMetadata.h"
#include <absl/strings/ascii.h>
#include <absl/strings/numbers.h>
#include <absl/strings/string_view.h>
#include <map>
#include <vector>
#include <string>
#include <cstdio>
#include <cstring>
#include <cmath>

namespace sfz {

// Utility: file cleanup

struct FILE_deleter {
    void operator()(FILE* x) const noexcept { fclose(x); }
};
typedef std::unique_ptr<FILE, FILE_deleter> FILE_u;

// Utility: binary file IO

static uint32_t u32le(const uint8_t *bytes)
{
    return bytes[0] | (bytes[1] << 8) | (bytes[2] << 16) | (bytes[3] << 24);
}

static uint32_t u32be(const uint8_t *bytes)
{
    return (bytes[0] << 24) | (bytes[1] << 16) | (bytes[2] << 8) | bytes[3];
}

#if 0
static uint16_t u16le(const uint8_t *bytes)
{
    return bytes[0] | (bytes[1] << 8);
}
#endif

static uint16_t u16be(const uint8_t *bytes)
{
    return (bytes[0] << 8) | bytes[1];
}

//------------------------------------------------------------------------------

struct MetadataReader::Impl {
    MetadataReader* parent;
    std::vector<RiffChunkInfo> riffChunks_;
    bool opened_ { false };
    enum class ChunkType { None, Riff, Aiff };
    ChunkType chunkType_ = ChunkType::None;

    bool openFlac();
    bool openRiff();
    bool openAiff();

    bool extractClmWavetable(WavetableInfo &wt);
    bool extractSurgeWavetable(WavetableInfo &wt);
    bool extractUheWavetable(WavetableInfo &wt);

    const RiffChunkInfo* riffChunk(size_t index) const;
    const RiffChunkInfo* riffChunkById(RiffChunkId id) const;
    size_t readRiffData(size_t index, void* buffer, size_t count);
    bool read_u32le(uint32_t& value);
    bool read_u32be(uint32_t& value);
    bool read_u16le(uint16_t& value);
    bool read_u16be(uint16_t& value);
};

bool MetadataReader::Impl::read_u32le(uint32_t& value)
{
    uint8_t bytes[4];
    if (parent->doRead(bytes, 4, 1) != 1)
        return false;
    value = u32le(bytes);
    return true;
}

bool MetadataReader::Impl::read_u32be(uint32_t& value)
{
    uint8_t bytes[4];
    if (parent->doRead(bytes, 4, 1) != 1)
        return false;
    value = u32be(bytes);
    return true;
}

#if 0
bool MetadataReader::Impl::read_u16le(uint16_t& value)
{
    uint8_t bytes[2];
    if (parent->do_read(bytes, 2, 1) != 1)
        return false;
    value = u16le(bytes);
    return true;
}
#endif

bool MetadataReader::Impl::read_u16be(uint16_t& value)
{
    uint8_t bytes[2];
    if (parent->doRead(bytes, 2, 1) != 1)
        return false;
    value = u16be(bytes);
    return true;
}

MetadataReader::MetadataReader()
    : impl_(new Impl)
{
    impl_->parent = this;
    impl_->riffChunks_.reserve(16);
}

MetadataReader::~MetadataReader()
{

}

bool MetadataReader::open()
{
    close();
    if (!doOpen())
        return false;

    Impl& impl = *impl_;
    impl.opened_ = true;
    char magic[4];
    size_t count = doRead(magic, 1, sizeof(magic));

    if (count >= 4 && !memcmp(magic, "fLaC", 4)) {
        if (!impl_->openFlac()) {
            close();
            return false;
        }
        impl_->chunkType_ = Impl::ChunkType::Riff;
    }
    else if (count >= 4 && !memcmp(magic, "RIFF", 4)) {
        if (!impl_->openRiff()) {
            close();
            return false;
        }
        impl_->chunkType_ = Impl::ChunkType::Riff;
    }
    else if (count >= 4 && !memcmp(magic, "FORM", 4)) {
        if (!impl_->openAiff()) {
            close();
            return false;
        }
        impl_->chunkType_ = Impl::ChunkType::Aiff;
    }

    return true;
}

bool MetadataReader::isOpened()
{
    return impl_->opened_;
}

void MetadataReader::close()
{
    Impl& impl = *impl_;
    impl.riffChunks_.clear();
    doClose();
    impl.opened_ = false;
}

bool MetadataReader::Impl::openFlac()
{
    std::vector<RiffChunkInfo>& riffChunks = riffChunks_;

    if (parent->doSeek(4, SEEK_SET) != 0)
        return false;

    uint32_t header = 0;
    while (((header >> 31) & 1) != 1) {
        if (!read_u32be(header))
            return false;

        const uint32_t blockType = (header >> 24) & 0x7f;
        const uint32_t blockSize = header & ((1 << 24) - 1);

        const off_t offStartBlock = parent->doTell();
        const off_t offNextBlock = offStartBlock + blockSize;

        if (blockType == 2) { // APPLICATION block
            char blockId[4];
            char riffId[4];
            uint32_t riffChunkSize;
            if (parent->doRead(blockId, 4, 1) == 1 && memcmp(blockId, "riff", 4) == 0 &&
                parent->doRead(riffId, 4, 1) == 1 &&
                read_u32le(riffChunkSize) && riffChunkSize <= blockSize - 12)
            {
                RiffChunkInfo info;
                info.index = riffChunks.size();
                info.fileOffset = parent->doTell();
                memcpy(info.id.data(), riffId, 4);
                info.length = riffChunkSize;
                riffChunks.push_back(info);
            }
        }

        if (parent->doSeek(offNextBlock, SEEK_SET) != 0)
            return false;
    }

    return true;
}

bool MetadataReader::Impl::openRiff()
{
    std::vector<RiffChunkInfo>& riffChunks = riffChunks_;

    if (parent->doSeek(12, SEEK_SET) != 0)
        return false;

    char riffId[4];
    uint32_t riffChunkSize;
    while (parent->doRead(riffId, 4, 1) == 1 && read_u32le(riffChunkSize)) {
        RiffChunkInfo info;
        info.index = riffChunks.size();
        info.fileOffset = parent->doTell();
        memcpy(info.id.data(), riffId, 4);
        info.length = riffChunkSize;
        riffChunks.push_back(info);

        if (parent->doSeek(riffChunkSize + (riffChunkSize & 1), SEEK_CUR) != 0)
            return false;
    }

    return true;
}

bool MetadataReader::Impl::openAiff()
{
    parent->doRewind();

    char formId[4];
    uint32_t formSize;
    if (parent->doRead(formId, 4, 1) != 1 || memcmp(formId, "FORM", 4) ||
        !read_u32be(formSize))
    {
        return false;
    }

    char aiffId[4];
    if (parent->doRead(aiffId, 4, 1) != 1 ||
        (memcmp(aiffId, "AIFF", 4) && memcmp(aiffId, "AIFC", 4)))
    {
        return false;
    }

    std::vector<RiffChunkInfo>& riffChunks = riffChunks_;

    char riffId[4];
    uint32_t riffChunkSize;
    while (parent->doRead(riffId, 4, 1) == 1 && read_u32be(riffChunkSize)) {
        RiffChunkInfo info;
        info.index = riffChunks.size();
        info.fileOffset = parent->doTell();
        memcpy(info.id.data(), riffId, 4);
        info.length = riffChunkSize;
        riffChunks.push_back(info);

        if (parent->doSeek(riffChunkSize + (riffChunkSize & 1), SEEK_CUR) != 0)
            return false;
    }

    return true;
}

size_t MetadataReader::riffChunkCount() const
{
    return impl_->riffChunks_.size();
}

const RiffChunkInfo* MetadataReader::riffChunk(size_t index) const
{
    return impl_->riffChunk(index);
}

const RiffChunkInfo* MetadataReader::Impl::riffChunk(size_t index) const
{
    return (index < riffChunks_.size()) ? &riffChunks_[index] : nullptr;
}

const RiffChunkInfo* MetadataReader::riffChunkById(RiffChunkId id) const
{
    return impl_->riffChunkById(id);
}

const RiffChunkInfo* MetadataReader::Impl::riffChunkById(RiffChunkId id) const
{
    for (const RiffChunkInfo& riff : riffChunks_) {
        if (riff.id == id)
            return &riff;
    }
    return nullptr;
}

size_t MetadataReader::readRiffData(size_t index, void* buffer, size_t count)
{
    return impl_->readRiffData(index, buffer, count);
}

size_t MetadataReader::Impl::readRiffData(size_t index, void* buffer, size_t count)
{
    const RiffChunkInfo* riff = riffChunk(index);
    if (!riff)
        return 0;

    count = (count < riff->length) ? count : riff->length;

    if (parent->doSeek(riff->fileOffset, SEEK_SET) != 0)
        return 0;

    return parent->doRead(buffer, 1, count);
}

bool MetadataReader::extractInstrument(InstrumentInfo& ins)
{
    if (extractRiffInstrument(ins))
        return true;

    if (extractAiffInstrument(ins))
        return true;

    return false;
}

bool MetadataReader::extractRiffInstrument(InstrumentInfo& ins)
{
    if (impl_->chunkType_ != Impl::ChunkType::Riff)
        return false;

    const RiffChunkInfo* riff = riffChunkById(RiffChunkId{'s', 'm', 'p', 'l'});
    if (!riff)
        return false;

    constexpr uint32_t maxLoops = 16;
    constexpr uint32_t maxChunkSize = 9 * 4 + maxLoops * 6 * 4;

    uint8_t data[maxChunkSize];
    uint32_t length = readRiffData(riff->index, data, sizeof(data));

    auto extractU32 = [&data, length](const uint32_t offset) -> uint32_t {
        const uint8_t* bytes = &data[offset];
        if (bytes + 4 > data + length)
            return 0;
        return bytes[0] | (bytes[1] << 8) | (bytes[2] << 16) | (bytes[3] << 24);
    };

    ins.gain = 1;
    ins.basenote = extractU32(0x14 - 8);
    ins.detune = static_cast<unsigned char>( // Q0,32 semitones to cents
        std::lround(extractU32(0x18 - 8) * (100.0 / (static_cast<int64_t>(1) << 32))));
    ins.velocity_lo = 0;
    ins.velocity_hi = 127;
    ins.key_lo = 0;
    ins.key_hi = 127;

    const uint32_t numLoops = std::min(maxLoops, extractU32(0x24 - 8));
    ins.loop_count = numLoops;

    for (uint32_t i = 0; i < numLoops; ++i) {
        const uint32_t loopOffset = 0x2c - 8 + i * 24;

        switch (extractU32(loopOffset + 0x04)) {
        default:
            ins.loops[i].mode = LoopNone;
            break;
        case 0:
            ins.loops[i].mode = LoopForward;
            break;
        case 1:
            ins.loops[i].mode = LoopAlternating;
            break;
        case 2:
            ins.loops[i].mode = LoopBackward;
            break;
        }

        ins.loops[i].start = extractU32(loopOffset + 0x08);
        ins.loops[i].end = extractU32(loopOffset + 0x0c) + 1;
        ins.loops[i].count = extractU32(loopOffset + 0x14);
    }

    return true;
}

bool MetadataReader::extractAiffInstrument(InstrumentInfo& ins)
{
    Impl& impl = *impl_;
    if (impl.chunkType_ != Impl::ChunkType::Aiff)
        return false;

    const RiffChunkInfo* instChunk = riffChunkById(RiffChunkId{'I', 'N', 'S', 'T'});
    if (!instChunk)
        return false;

    const RiffChunkInfo* markChunk = riffChunkById(RiffChunkId{'M', 'A', 'R', 'K'});

    uint8_t insData[20];
    uint32_t length = readRiffData(instChunk->index, insData, sizeof(insData));
    if (length != 20)
        return false;

    //
    std::map<uint16_t, uint32_t> markers;
    if (markChunk) {
        if (doSeek(markChunk->fileOffset, SEEK_SET) != 0)
            return false;

        uint16_t numMarkers;
        if (!impl.read_u16be(numMarkers))
            return false;

        for (uint32_t i = 0; i < numMarkers; ++i) {
            uint16_t id;
            uint32_t position;
            uint8_t size;
            char name[256];

            if (!impl.read_u16be(id) || !impl.read_u32be(position) || doRead(&size, 1, 1) != 1 || doRead(name, size, 1) != 1)
                return false;
            name[size] = '\0';

            if (i + 1 < numMarkers && ((~size) & 1)) {
                if (doSeek(1, SEEK_CUR) != 0)
                    return false;
            }

            markers[id] = position;
        }
    }

    //
    ins.basenote = insData[0];
    ins.detune = insData[1];
    ins.key_lo = insData[2];
    ins.key_hi = insData[3];
    ins.velocity_lo = insData[4];
    ins.velocity_hi = insData[5];
    ins.gain = (insData[6] << 8) | insData[7];

    uint32_t loopCount = 0;
    for (uint32_t loopIndex = 0; loopIndex < 2; ++loopIndex) {
        const uint32_t loopOffset = 8 + loopIndex * 6;

        int mode;
        switch ((insData[loopOffset] << 8) | insData[loopOffset + 1]) {
        default:
            mode = LoopNone;
            break;
        case 1:
            mode = LoopForward;
            break;
        case 2:
            mode = LoopBackward;
            break;
        }

        if (mode == LoopNone)
            break;

        const uint16_t startId = (insData[loopOffset + 2] << 8) | insData[loopOffset + 3];
        const uint16_t endId = (insData[loopOffset + 4] << 8) | insData[loopOffset + 5];

        //
        uint32_t startPos = 0;
        uint32_t endPos = 0;

        auto startIt = markers.find(startId);
        auto endIt = markers.find(endId);
        if (startIt != markers.end())
            startPos = startIt->second;
        if (endIt != markers.end())
            endPos = endIt->second;

        //
        ins.loops[loopIndex].mode = mode;
        ins.loops[loopIndex].start = startPos;
        ins.loops[loopIndex].end = endPos;
        ins.loops[loopIndex].count = 0;

        ++loopCount;
    }

    ins.loop_count = loopCount;

    return true;
}

bool MetadataReader::extractWavetableInfo(WavetableInfo& wt)
{
    if (impl_->extractClmWavetable(wt))
        return true;

    if (impl_->extractSurgeWavetable(wt))
        return true;

    if (impl_->extractUheWavetable(wt))
        return true;

    // there also exists a method based on cue chunks used in Surge
    // files possibly already covered by the Native case
    // otherwise do later when I will have a few samples at hand

    return false;
}

bool MetadataReader::Impl::extractClmWavetable(WavetableInfo &wt)
{
    const RiffChunkInfo* clm = riffChunkById(RiffChunkId{'c', 'l', 'm', ' '});
    if (!clm)
        return false;

    char data[16] {};
    if (readRiffData(clm->index, data, sizeof(data)) != sizeof(data))
        return false;

    // 0-2 are "<!>"
    // 3-6 is the decimal table size written in ASCII (most likely "2048")
    // 7 is a space character
    // 8-15 are flags as ASCII digit characters (eg. "01000000")
    // 16-end "wavetable (<maker name>)"

    if (!absl::SimpleAtoi(absl::string_view(data + 3, 4), &wt.tableSize))
        return false;

    int cti = static_cast<unsigned char>(data[8]);
    if (cti >= '0' && cti <= '4')
        cti -= '0';
    else
        cti = 0; // unknown interpolation
    wt.crossTableInterpolation = cti;

    wt.oneShot = false;

    return true;
}

bool MetadataReader::Impl::extractSurgeWavetable(WavetableInfo &wt)
{
    const RiffChunkInfo* srge;

    if ((srge = riffChunkById(RiffChunkId{'s', 'r', 'g', 'e'})))
        wt.oneShot = false;
    else if ((srge = riffChunkById(RiffChunkId{'s', 'r', 'g', 'o'})))
        wt.oneShot = true;
    else
        return false;

    uint8_t data[8];
    if (readRiffData(srge->index, data, sizeof(data)) != sizeof(data))
        return false;

    //const uint32_t version = u32le(data);
    wt.tableSize = u32le(data + 4);

    wt.crossTableInterpolation = 0;

    return true;
}

bool MetadataReader::Impl::extractUheWavetable(WavetableInfo &wt)
{
    const RiffChunkInfo* uhwt = riffChunkById(RiffChunkId{'u', 'h', 'W', 'T'});
    if (!uhwt)
        return false;

    // zeros (chunk version?), 4 bytes LE
    // number of tables, 4 bytes LE
    // table size, 4 bytes LE

    uint8_t data[12];
    if (readRiffData(uhwt->index, data, sizeof(data)) != sizeof(data))
        return false;

    wt.tableSize = u32le(data + 8);

    wt.crossTableInterpolation = 0;
    wt.oneShot = false;

    return true;
}

struct FileMetadataReader::Impl {
    fs::path path;
    FILE_u stream_;
};

FileMetadataReader::FileMetadataReader(const fs::path& path)
    : impl_(new Impl)
{
    impl_->path = path;
}

FileMetadataReader::~FileMetadataReader()
{
}

void FileMetadataReader::doClose()
{
    impl_->stream_.reset();
}

bool FileMetadataReader::doOpen()
{
    Impl& impl = *impl_;
#if !defined(_WIN32)
    FILE* stream = fopen(impl.path.c_str(), "rb");
#else
    FILE* stream = _wfopen(impl.path.wstring().c_str(), L"rb");
#endif

    if (!stream)
        return false;

    impl_->stream_.reset(stream);
    return true;
}

size_t FileMetadataReader::doRead(void* ptr, size_t size, size_t n)
{
    return fread(ptr, size, n, impl_->stream_.get());
}

int FileMetadataReader::doSeek(long off, int whence)
{
    return fseek(impl_->stream_.get(), off, whence);
}

void FileMetadataReader::doRewind()
{
    rewind(impl_->stream_.get());
}

long FileMetadataReader::doTell()
{
    return ftell(impl_->stream_.get());
}

struct MemoryMetadataReader::Impl {
    const char* memory;
    size_t length;
    size_t position { 0 };
};

MemoryMetadataReader::MemoryMetadataReader(const void* memory, size_t length)
    : impl_(new Impl)
{
    impl_->memory = reinterpret_cast<const char*>(memory);
    impl_->length = length;
}

MemoryMetadataReader::~MemoryMetadataReader()
{
}

void MemoryMetadataReader::doClose()
{
}

bool MemoryMetadataReader::doOpen()
{
    Impl& impl = *impl_;
    if (impl.length == 0)
        return false;

    return true;
}

size_t MemoryMetadataReader::doRead(void* ptr, size_t size, size_t n)
{
    Impl& impl = *impl_;
    size_t actualCount =
        (std::min(impl.position + n * size, impl.length) - impl.position) / size;
    size_t totalBytes = actualCount * size;
    memcpy(ptr, impl.memory + impl.position, totalBytes);
    impl.position += totalBytes;
    return actualCount;
}

int MemoryMetadataReader::doSeek(long off, int whence)
{
    Impl& impl = *impl_;

    switch(whence) {
    case SEEK_SET:
        {
            if (off < 0)
                return -1;

            const size_t offset = static_cast<size_t>(off);
            if (offset > impl.length)
                return -1;
            impl.position = offset;
        }
        break;
    case SEEK_CUR:
        {
            if (off < 0) {
                const size_t minusOffset = static_cast<size_t>(-off);
                if (minusOffset > impl.position)
                    return -1;

                impl.position -= minusOffset;
            } else {
                const size_t offset = static_cast<size_t>(off);
                const size_t newPosition = impl.position + offset;
                if (newPosition > impl.length)
                    return -1;

                impl.position = newPosition;
            }
        }
        break;
    case SEEK_END:
        {
            if (off > 0)
                return -1;

            const size_t minusOffset = static_cast<size_t>(-off);
            if (minusOffset > impl.length)
                return -1;
            impl.position = impl.length - minusOffset;
        }
        break;
    }
    return 0;
}

void MemoryMetadataReader::doRewind()
{
    impl_->position = 0;
}

long MemoryMetadataReader::doTell()
{
    return static_cast<long>(impl_->position);
}

} // namespace sfz
