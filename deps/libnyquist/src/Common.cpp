/*
Copyright (c) 2019, Dimitri Diakopoulos All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are met:

* Redistributions of source code must retain the above copyright notice, this
  list of conditions and the following disclaimer.

* Redistributions in binary form must reproduce the above copyright notice,
  this list of conditions and the following disclaimer in the documentation
  and/or other materials provided with the distribution.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#include "Common.h"
#include "Decoders.h"
#include <cstring>
#include <unordered_map>

using namespace nqr;

NyquistIO::NyquistIO() { BuildDecoderTable(); }
NyquistIO::~NyquistIO() { }

void NyquistIO::Load(AudioData * data, const std::string & path)
{
    if (IsFileSupported(path))
    {
        if (decoderTable.size())
        {
            auto fileExtension = ParsePathForExtension(path);
            auto decoder = GetDecoderForExtension(fileExtension);

            try
            {
                decoder->LoadFromPath(data, path);
            }
            catch (const std::exception & e)
            {
                std::cerr << "NyquistIO::Load(" << path << ") caught internal exception: " << e.what() << std::endl;
                throw;
            }

        }
        else throw std::runtime_error("No available decoders.");
    }
    else
    {
        throw UnsupportedExtensionEx();
    }
}

void NyquistIO::Load(AudioData * data, const std::vector<uint8_t> & buffer)
{

    const std::map<std::vector<int16_t>, std::string> magic_map{
        {{ 'w', 'v', 'p', 'k' },                                                     "wv"             },
        {{ 'M', 'P', 'C', 'K' },                                                     "mpc"            },
        {{ 0xFF, 0xFB },                                                             "mp3"            }, // ÿû, mp3 without ID3 header
        {{ 'I', 'D', '3' },                                                          "mp3"            }, // mp3 with ID3 header
        {{ 'O', 'g', 'g', 'S' },                                                     "ogg_or_vorbis"  }, // see `match_ogg_subtype`
        {{ 'f', 'L', 'a', 'C' },                                                     "flac"           },
        {{ 0x52, 0x49, 0x46, 0x46, -0x1, -0x1, -0x1, -0x1, 0x57, 0x41, 0x56, 0x45 }, "wav"            }  // RIFF....WAVE
    };

    auto match_magic = [](const uint8_t * data, const std::vector<int16_t> & magic)
    {
        for (int i = 0; i < magic.size(); ++i)
        {
            if (magic[i] != data[i] && magic[i] != -0x1) // -0x1 skips things that don't contribute to the magic number
            {
                return false;
            }
        }
        return true;
    };

    auto match_ogg_subtype = [](const uint8_t * data)
    {
        std::string header; // arbitrarily read the first 64 bytes as ascii characters
        for (int i = 0; i < 64; ++i) header += data[i];

        std::size_t found_opus = header.find("OpusHead");
        if (found_opus != std::string::npos) return "opus";

        std::size_t found_vorbis = header.find("vorbis");
        if (found_vorbis != std::string::npos) return "ogg";

        return "none";
    };

    std::string ext = "none";

    for (auto & filetype : magic_map)
    {
        if (match_magic(buffer.data(), filetype.first))
        {
            ext = filetype.second;
        }

        if (ext == "ogg_or_vorbis")
        {
            ext = match_ogg_subtype(buffer.data());
        }
    }

    NyquistIO::Load(data, ext, buffer);
}

void NyquistIO::Load(AudioData * data, const std::string & extension, const std::vector<uint8_t> & buffer)
{
    if (decoderTable.find(extension) == decoderTable.end())
    {
        throw UnsupportedExtensionEx();
    }

    if (decoderTable.size())
    {
        auto decoder = GetDecoderForExtension(extension);
        try
        {
            decoder->LoadFromBuffer(data, buffer);
        }
        catch (const std::exception & e)
        {
            std::cerr << "caught internal loading exception: " << e.what() << std::endl;
            throw;
        }
    }
    else throw std::runtime_error("fatal: no decoders available");
}

bool NyquistIO::IsFileSupported(const std::string & path) const
{
    auto fileExtension = ParsePathForExtension(path);
    if (decoderTable.find(fileExtension) == decoderTable.end()) return false;
    else return true;
}

std::string NyquistIO::ParsePathForExtension(const std::string & path) const
{
    if (path.find_last_of(".") != std::string::npos) return path.substr(path.find_last_of(".") + 1);
    return std::string("");
}

std::shared_ptr<BaseDecoder> NyquistIO::GetDecoderForExtension(const std::string & ext)
{
    if (decoderTable.size()) return decoderTable[ext];
    else throw std::runtime_error("No available decoders.");
    return nullptr;
}

void NyquistIO::AddDecoderToTable(std::shared_ptr<nqr::BaseDecoder> decoder)
{
    auto supportedExtensions = decoder->GetSupportedFileExtensions();

    for (const auto ext : supportedExtensions)
    {
        if (decoderTable.count(ext) >= 1) throw std::runtime_error("decoder already exists for extension");
        decoderTable.insert(DecoderPair(ext, decoder));
    }
}

void NyquistIO::BuildDecoderTable()
{
    AddDecoderToTable(std::make_shared<WavDecoder>());
    AddDecoderToTable(std::make_shared<WavPackDecoder>());
    AddDecoderToTable(std::make_shared<FlacDecoder>());
    AddDecoderToTable(std::make_shared<VorbisDecoder>());
    AddDecoderToTable(std::make_shared<OpusDecoder>());
    AddDecoderToTable(std::make_shared<MusepackDecoder>());
    AddDecoderToTable(std::make_shared<Mp3Decoder>());
}

NyquistFileBuffer nqr::ReadFile(const std::string & pathToFile)
{
    //std::cout << "[Debug] Open: " << pathToFile << std::endl;
    FILE * audioFile = fopen(pathToFile.c_str(), "rb");

    if (!audioFile)
    {
        throw std::runtime_error("file not found");
    }

    fseek(audioFile, 0, SEEK_END);
    size_t lengthInBytes = ftell(audioFile);
    fseek(audioFile, 0, SEEK_SET);

    // Allocate temporary buffer
    std::vector<uint8_t> fileBuffer(lengthInBytes);

    size_t elementsRead = fread(fileBuffer.data(), 1, lengthInBytes, audioFile);

    if (elementsRead == 0 || fileBuffer.size() < 64)
    {
        throw std::runtime_error("error reading file or file too small");
    }

    NyquistFileBuffer data = {std::move(fileBuffer), elementsRead};

    fclose(audioFile);

    return data;
}

// Src data is aligned to PCMFormat
// @todo normalize?
void nqr::ConvertToFloat32(float * dst, const uint8_t * src, const size_t N, PCMFormat f)
{
    assert(f != PCM_END);

    if (f == PCM_U8)
    {
        const uint8_t * dataPtr = reinterpret_cast<const uint8_t *>(src);
        for (size_t i = 0; i < N; ++i)
            dst[i] = uint8_to_float32(dataPtr[i]);
    }
    else if (f == PCM_S8)
    {
        const int8_t * dataPtr = reinterpret_cast<const int8_t *>(src);
        for (size_t i = 0; i < N; ++i)
            dst[i] = int8_to_float32(dataPtr[i]);
    }
    else if (f == PCM_16)
    {
        const int16_t * dataPtr = reinterpret_cast<const int16_t *>(src);
        for (size_t i = 0; i < N; ++i)
            dst[i] = int16_to_float32(Read16(dataPtr[i]));
    }
    else if (f == PCM_24)
    {
        const uint8_t * dataPtr = reinterpret_cast<const uint8_t *>(src);
        size_t c = 0;
        for (size_t i = 0; i < N; ++i)
        {
            int32_t sample = Pack(dataPtr[c], dataPtr[c+1], dataPtr[c+2]);
            dst[i] = int24_to_float32(sample); // Packed types don't need addtional endian helpers
            c += 3;
        }
    }
    else if (f == PCM_32)
    {
        const int32_t * dataPtr = reinterpret_cast<const int32_t *>(src);
        for (size_t i = 0; i < N; ++i)
            dst[i] = int32_to_float32(Read32(dataPtr[i]));
    }

    //@todo add int64 format

    else if (f == PCM_FLT)
    {
        std::memcpy(dst, src, N * sizeof(float));
        /* const float * dataPtr = reinterpret_cast<const float *>(src);
        for (size_t i = 0; i < N; ++i)
            dst[i] = (float) Read32(dataPtr[i]); */
    }
    else if (f == PCM_DBL)
    {
        const double * dataPtr = reinterpret_cast<const double *>(src);
        for (size_t i = 0; i < N; ++i)
            dst[i] = (float) Read64(dataPtr[i]);
    }
}

// Src data is always aligned to 4 bytes (WavPack, primarily)
void nqr::ConvertToFloat32(float * dst, const int32_t * src, const size_t N, PCMFormat f)
{
    assert(f != PCM_END);

    if (f == PCM_16)
    {
        for (size_t i = 0; i < N; ++i)
            dst[i] = int16_to_float32(Read32(src[i]));
    }
    else if (f == PCM_24)
    {
        const uint8_t * dataPtr = reinterpret_cast<const uint8_t *>(src);
        size_t c = 0;
        for (size_t i = 0; i < N; ++i)
        {
            int32_t sample = Pack(dataPtr[c], dataPtr[c+1], dataPtr[c+2]);
            dst[i] = int24_to_float32(sample);
            c += 4; // +4 for next 4 byte boundary
        }
    }
    else if (f == PCM_32)
    {
        for (size_t i = 0; i < N; ++i)
            dst[i] = int32_to_float32(Read32(src[i]));
    }
}

void nqr::ConvertToFloat32(float * dst, const int16_t * src, const size_t N, PCMFormat f)
{
    assert(f != PCM_END);
    if (f == PCM_16)
    {
        for (size_t i = 0; i < N; ++i)
            dst[i] = int16_to_float32(Read16(src[i]));
    }
}

void nqr::ConvertFromFloat32(uint8_t * dst, const float * src, const size_t N, PCMFormat f, DitherType t)
{
    assert(f != PCM_END);

    Dither dither(t);

    if (f == PCM_U8)
    {
        uint8_t * destPtr = reinterpret_cast<uint8_t *>(dst);
        for (size_t i = 0; i < N; ++i)
            destPtr[i] = (uint8_t) dither(lroundf(float32_to_uint8(src[i])));
    }
    else if (f == PCM_S8)
    {
        int8_t * destPtr = reinterpret_cast<int8_t *>(dst);
        for (size_t i = 0; i < N; ++i)
            destPtr[i] = (int8_t) dither(lroundf(float32_to_int8(src[i])));
    }
    else if (f == PCM_16)
    {
        int16_t * destPtr = reinterpret_cast<int16_t *>(dst);
        for (size_t i = 0; i < N; ++i)
            destPtr[i] =(int16_t) dither(lroundf(float32_to_int16(src[i])));
    }
    else if (f == PCM_24)
    {
        uint8_t * destPtr = reinterpret_cast<uint8_t *>(dst);
        size_t c = 0;
        for (size_t i = 0; i < N; ++i)
        {
            int32_t sample = (int32_t) dither(lroundf(float32_to_int24(src[i])));
            auto unpacked = Unpack(sample); // Handles endian swap
            destPtr[c] = unpacked[0];
            destPtr[c+1] = unpacked[1];
            destPtr[c+2] = unpacked[2];
            c += 3;
        }
    }
    else if (f == PCM_32)
    {
        int32_t * destPtr = reinterpret_cast<int32_t *>(dst);
        for (size_t i = 0; i < N; ++i)
            destPtr[i] = (int32_t) dither(lroundf(float32_to_int32(src[i])));
    }
}

int nqr::GetFormatBitsPerSample(PCMFormat f)
{
    switch(f)
    {
        case PCM_U8:
        case PCM_S8:
            return 8;
        case PCM_16:
            return 16;
        case PCM_24:
            return 24;
        case PCM_32:
        case PCM_FLT:
            return 32;
        case PCM_64:
        case PCM_DBL:
            return 64;
        default:
            return 0;
    }
}

PCMFormat nqr::MakeFormatForBits(int bits, bool floatingPt, bool isSigned)
{
    switch(bits)
    {
        case 8: return (isSigned) ? PCM_S8 : PCM_U8;
        case 16: return PCM_16;
        case 24: return PCM_24;
        case 32: return (floatingPt) ? PCM_FLT : PCM_32;
        case 64: return (floatingPt) ? PCM_DBL : PCM_64;
        default: return PCM_END;
    }
}
