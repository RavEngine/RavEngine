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

#ifndef LIBNYQUIST_COMMON_H
#define LIBNYQUIST_COMMON_H

#include <memory>
#include <vector>
#include <algorithm>
#include <cmath>
#include <iostream>
#include <string>
#include <stdint.h>
#include <cassert>
#include <type_traits>
#include <numeric>
#include <array>
#include <map>
#include <random>

namespace nqr
{

/////////////////
// Util Macros //
/////////////////

#define F_ROUND(x) ((x) > 0 ? (x) + 0.5f : (x) - 0.5f)
#define D_ROUND(x) ((x) > 0 ? (x) + 0.5  : (x) - 0.5)

#define NO_COPY(C) C(const C &) = delete; C & operator = (const C &) = delete
#define NO_MOVE(C) NO_COPY(C); C(C &&) = delete; C & operator = (const C &&) = delete

///////////////////////
// Endian Operations //
///////////////////////

#if defined(__x86_64__) || defined(_M_X64) || defined(__i386__) || defined(_M_IX86)
    #define CPU_X86 1
#endif

#if defined(__arm__) || defined(_M_ARM)
    #define CPU_ARM 1
#endif

#if defined(CPU_X86) && defined(CPU_ARM)
    #error CPU_X86 and CPU_ARM both defined.
#endif

#if !defined(ARCH_CPU_BIG_ENDIAN) && !defined(ARCH_CPU_LITTLE_ENDIAN)
    #if CPU_X86 || CPU_ARM || (defined(__BYTE_ORDER__) && __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__)
        #define ARCH_CPU_LITTLE_ENDIAN
    #elif defined(__BYTE_ORDER__) && __BYTE_ORDER__ == __ORDER_BIG_ENDIAN__
        #define ARCH_CPU_BIG_ENDIAN
    #else
        #error ARCH_CPU_BIG_ENDIAN or ARCH_CPU_LITTLE_ENDIAN should be defined.
    #endif
#endif

#if defined(ARCH_CPU_BIG_ENDIAN) && defined(ARCH_CPU_LITTLE_ENDIAN)
    #error ARCH_CPU_BIG_ENDIAN and ARCH_CPU_LITTLE_ENDIAN both defined.
#endif

static inline uint16_t Swap16(uint16_t value)
{
    return (uint16_t)((value >> 8) | (value << 8));
}

static inline uint32_t Swap24(uint32_t value)
{
    return (((value & 0x00ff0000) >> 16) |
            ((value & 0x0000ff00))       |
            ((value & 0x000000ff) << 16)) & 0x00FFFFFF;
}

static inline uint32_t Swap32(uint32_t value)
{
    return (((value & 0x000000ff) << 24) |
            ((value & 0x0000ff00) <<  8) |
            ((value & 0x00ff0000) >>  8) |
            ((value & 0xff000000) >> 24));
}

static inline uint64_t Swap64(uint64_t value)
{
    return (((value & 0x00000000000000ffLL) << 56) |
            ((value & 0x000000000000ff00LL) << 40) |
            ((value & 0x0000000000ff0000LL) << 24) |
            ((value & 0x00000000ff000000LL) << 8)  |
            ((value & 0x000000ff00000000LL) >> 8)  |
            ((value & 0x0000ff0000000000LL) >> 24) |
            ((value & 0x00ff000000000000LL) >> 40) |
            ((value & 0xff00000000000000LL) >> 56));
}

template<typename T>
inline bool isOdd(const T x)
{
    return (x & 0x1);
}
    
#ifdef ARCH_CPU_LITTLE_ENDIAN
    #define Read16(n) (n)
    #define Read24(n) (n)
    #define Read32(n) (n)
    #define Read64(n) (n)
    #define Write16(n) (n)
    #define Write24(n) (n)
    #define Write32(n) (n)
    #define Write64(n) (n)
#else
    #define Read16(n) Swap16(n)
    #define Read24(n) Swap24(n)
    #define Read32(n) Swap32(n)
    #define Read64(n) Swap64(n)
    #define Write16(n) Swap16(n)
    #define Write24(n) Swap24(n)
    #define Write32(n) Swap32(n)
    #define Write64(n) Swap64(n)
#endif

inline uint64_t Pack(uint32_t a, uint32_t b)
{
    uint64_t tmp = (uint64_t) b << 32 | (uint64_t) a;
#ifdef ARCH_CPU_LITTLE_ENDIAN
    return tmp;
#else
    return Swap64(tmp);
#endif
}

inline uint32_t Pack(uint16_t a, uint16_t b)
{
    uint32_t tmp = (uint32_t) b << 16 | (uint32_t) a;
#ifdef ARCH_CPU_LITTLE_ENDIAN
    return tmp;
#else
    return Swap32(tmp);
#endif
}

inline uint16_t Pack(uint8_t a, uint8_t b)
{
    uint16_t tmp = (uint16_t) b << 8 | (uint16_t) a;
#ifdef ARCH_CPU_LITTLE_ENDIAN
    return tmp;
#else
    return Swap16(tmp);
#endif
}

// http://www.dsprelated.com/showthread/comp.dsp/136689-1.php
inline int32_t Pack(uint8_t a, uint8_t b, uint8_t c)
{
    // uint32_t tmp = ((c & 0x80) ? (0xFF << 24) : 0x00 << 24) | (c << 16) | (b << 8) | (a << 0); // alternate method
    int32_t x = (c << 16) | (b << 8) | (a << 0);
    auto sign_extended = (x) | (!!((x) & 0x800000) * 0xff000000);
    #ifdef ARCH_CPU_LITTLE_ENDIAN
        return sign_extended;
    #else
        Swap32(sign_extended);
    #endif
}

inline std::array<uint8_t, 3> Unpack(uint32_t a)
{
    static std::array<uint8_t, 3> output;
    
    #ifdef ARCH_CPU_LITTLE_ENDIAN
        output[0] = a >> 0;
        output[1] = a >> 8;
        output[2] = a >> 16;
    #else
        output[0] = a >> 16;
        output[1] = a >> 8;
        output[2] = a >> 0;
    #endif
    return output;
}

//////////////////////////
// Resampling Utilities //
//////////////////////////

// This is a naieve implementation of a resampling filter where a lerp is used as a bad low-pass.
// It very far from the ideal case and should be used with caution (or not at all) on signals that matter.
// It is included here to upsample 44.1k to 48k for the purposes of microphone input => Opus, where the the 
// nominal frequencies of speech are particularly far from Nyquist.
inline void linear_resample(const double rate, const std::vector<float> & input, std::vector<float> & output, const uint32_t samplesToProcess)
{
    double virtualReadIndex = 0;
    double a, b, i, sample;
    uint32_t n = samplesToProcess - 1;
    while (n--)
    {
        uint32_t readIndex = static_cast<uint32_t>(virtualReadIndex);
        i = virtualReadIndex - readIndex;
        a = input[readIndex + 0];
        b = input[readIndex + 1];
        sample = (1.0 - i) * a + i * b; // linear interpolate
        output.push_back(static_cast<float>(sample));
        virtualReadIndex += rate;
    }
}

inline double sample_hermite_4p_3o(double x, double * y)
{
    static double c0, c1, c2, c3;
    c0 = y[1];
    c1 = (1.0 / 2.0)*(y[2] - y[0]);
    c2 = (y[0] - (5.0 / 2.0)*y[1]) + (2.0*y[2] - (1.0 / 2.0)*y[3]);
    c3 = (1.0 / 2.0)*(y[3] - y[0]) + (3.0 / 2.0)*(y[1] - y[2]);
    return ((c3*x + c2)*x + c1)*x + c0;
}

inline void hermite_resample(const double rate, const std::vector<float> & input, std::vector<float> & output, const uint32_t samplesToProcess)
{
    double virtualReadIndex = 1;
    double i, sample;
    uint32_t n = samplesToProcess - 1;
    while (n--)
    {
        uint32_t readIndex = static_cast<uint32_t>(virtualReadIndex);
        i = virtualReadIndex - readIndex;
        double samps[4] = { input[readIndex - 1], input[readIndex], input[readIndex + 1], input[readIndex + 2] };
        sample = sample_hermite_4p_3o(i, samps); // cubic hermite interpolate over 4 samples
        output.push_back(static_cast<float>(sample));
        virtualReadIndex += rate;
    }
}

//////////////////////////
// Conversion Utilities //
//////////////////////////

enum DitherType
{
    DITHER_NONE,
    DITHER_TRIANGLE
};

class Dither
{
    std::uniform_real_distribution<float> distribution;
    std::mt19937 gen;
    float previous;
    DitherType d;
public:

    Dither(DitherType d) : distribution(-0.5f, +0.5f), previous(0.f), d(d) {}

    float operator()(float s)
    {
        if (d == DITHER_TRIANGLE)
        {
            const float value = distribution(gen);
            s = s + value - previous;
            previous = value;
            return s;
        }
        else return s;
    }
};

// Signed maxes, defined for readabilty/convenience
#define NQR_INT16_MAX 32767.f
#define NQR_INT24_MAX 8388608.f
#define NQR_INT32_MAX 2147483648.f

static const float NQR_BYTE_2_FLT = 1.0f / 127.0f;

#define int8_to_float32(s)  ((float) (s) * NQR_BYTE_2_FLT)
#define uint8_to_float32(s)(((float) (s) - 128) * NQR_BYTE_2_FLT)
#define int16_to_float32(s) ((float) (s) / NQR_INT16_MAX)
#define int24_to_float32(s) ((float) (s) / NQR_INT24_MAX)
#define int32_to_float32(s) ((float) (s) / NQR_INT32_MAX)
    
#define float32_to_int8(s)  (float) (s * 127.f)
#define float32_to_uint8(s) (float) ((s * 127.f) + 128)
#define float32_to_int16(s) (float) (s * NQR_INT16_MAX)
#define float32_to_int24(s) (float) (s * NQR_INT24_MAX)
#define float32_to_int32(s) (float) (s * NQR_INT32_MAX)

//@todo add 12, 20 for flac
enum PCMFormat
{
    PCM_U8,
    PCM_S8,
    PCM_16,
    PCM_24,
    PCM_32,
    PCM_64,
    PCM_FLT,
    PCM_DBL,
    PCM_END
};

template<class T> T clamp(T a, T mn, T mx) { return std::max(std::min(a, mx), mn); }
    
// Src data is aligned to PCMFormat
// @todo normalize?
void ConvertToFloat32(float * dst, const uint8_t * src, const size_t N, PCMFormat f);

// Src data is always aligned to 4 bytes (WavPack, primarily)
void ConvertToFloat32(float * dst, const int32_t * src, const size_t N, PCMFormat f);

// Src data is always aligned to 2 bytes (IMA ADPCM, primarily)
void ConvertToFloat32(float * dst, const int16_t * src, const size_t N, PCMFormat f);
    
void ConvertFromFloat32(uint8_t * dst, const float * src, const size_t N, PCMFormat f, DitherType t = DITHER_NONE);

int GetFormatBitsPerSample(PCMFormat f);
PCMFormat MakeFormatForBits(int bits, bool floatingPt, bool isSigned);

//////////////////////////
// User Data + File Ops //
//////////////////////////

struct AudioData
{
    int channelCount;
    int sampleRate;
    double lengthSeconds;
    size_t frameSize; // channels * bits per sample
    std::vector<float> samples;
    PCMFormat sourceFormat;
    
    //@todo: add field: channel layout
    //@todo: add field: lossy / lossless
    //@todo: audio data loaded (for metadata only)
    //@todo: bitrate (if applicable)
    //@todo: original sample rate (if applicable)
};

struct StreamableAudioData : public AudioData
{
    //@todo: add field: is this format streamable?
    //@todo: hold file handle
};

struct NyquistFileBuffer
{
    std::vector<uint8_t> buffer;
    size_t size;
};

NyquistFileBuffer ReadFile(const std::string & pathToFile);

////////////////////
// Encoding Utils //
////////////////////

struct EncoderParams
{
    int channelCount;
    PCMFormat targetFormat;
    DitherType dither;
};

enum EncoderError
{
    NoError,
    InsufficientSampleData,
    FileIOError,
    UnsupportedSamplerate,
    UnsupportedChannelConfiguration,
    UnsupportedBitdepth,
    UnsupportedChannelMix,
    BufferTooBig,
};

//////////////////////
// Wav Format Utils //
//////////////////////

enum WaveFormatCode
{
    FORMAT_UNKNOWN = 0x0,       // Unknown Wave Format
    FORMAT_PCM = 0x1,           // PCM Format
    FORMAT_ADPCM = 0x2,         // Microsoft ADPCM Format
    FORMAT_IEEE = 0x3,          // IEEE float/double
    FORMAT_ALAW = 0x6,          // 8-bit ITU-T G.711 A-law
    FORMAT_MULAW = 0x7,         // 8-bit ITU-T G.711 µ-law
    FORMAT_IMA_ADPCM = 0x11,    // IMA ADPCM Format
    FORMAT_EXT = 0xFFFE         // Set via subformat
};

struct RiffChunkHeader
{
    uint32_t id_riff;           // Chunk ID: 'RIFF'
    uint32_t file_size;         // Entire file in bytes
    uint32_t id_wave;           // Chunk ID: 'WAVE'
};

struct WaveChunkHeader
{
    uint32_t fmt_id;            // Chunk ID: 'fmt '
    uint32_t chunk_size;        // Size in bytes
    uint16_t format;            // Format code
    uint16_t channel_count;     // Num interleaved channels
    uint32_t sample_rate;       // SR
    uint32_t data_rate;         // Data rate
    uint16_t frame_size;        // 1 frame = channels * bits per sample (also known as block align)
    uint16_t bit_depth;         // Bits per sample  
};

struct BextChunk 
{
    uint32_t fmt_id;            // Chunk ID: 'bext'
    uint32_t chunk_size;        // Size in bytes
    uint8_t description[256];   // Description of the sound (ascii)
    uint8_t origin[32];         // Name of the originator (ascii)
    uint8_t origin_ref[32];     // Reference of the originator (ascii)
    uint8_t orgin_date[10];     // yyyy-mm-dd (ascii)
    uint8_t origin_time[8];     // hh-mm-ss (ascii)
    uint64_t time_ref;          // First sample count since midnight
    uint32_t version;           // Version of the BWF
    uint8_t uimd[64];           // Byte 0 of SMPTE UMID
    uint8_t reserved[188];      // 190 bytes, reserved for future use & set to NULL
};

struct FactChunk
{
    uint32_t fact_id;           // Chunk ID: 'fact'
    uint32_t chunk_size;        // Size in bytes 
    uint32_t sample_length;     // number of samples per channel
};

struct ExtensibleData
{
    uint16_t size;
    uint16_t valid_bits_per_sample;
    uint32_t channel_mask;
    struct GUID
    {
        uint32_t data0;
        uint16_t data1;
        uint16_t data2;
        uint16_t data3;
        uint8_t data4[6];
    };
};

template<class C, class R> 
std::basic_ostream<C,R> & operator << (std::basic_ostream<C,R> & a, const WaveChunkHeader & b) 
{ 
    return a <<
        "Format ID:\t\t"        << b.fmt_id <<
        "\nChunk Size:\t\t"     << b.chunk_size <<
        "\nFormat Code:\t\t"    << b.format <<
        "\nChannels:\t\t"       << b.channel_count <<
        "\nSample Rate:\t\t"    << b.sample_rate <<
        "\nData Rate:\t\t"      << b.data_rate <<
        "\nFrame Size:\t\t"     << b.frame_size <<
        "\nBit Depth:\t\t"      << b.bit_depth << std::endl;
}

//@todo expose speaker/channel/layout masks in the API: 
    
enum SpeakerChannelMask
{
    SPEAKER_FRONT_LEFT = 0x00000001,
    SPEAKER_FRONT_RIGHT = 0x00000002,
    SPEAKER_FRONT_CENTER = 0x00000004,
    SPEAKER_LOW_FREQUENCY = 0x00000008,
    SPEAKER_BACK_LEFT = 0x00000010,
    SPEAKER_BACK_RIGHT = 0x00000020,
    SPEAKER_FRONT_LEFT_OF_CENTER = 0x00000040,
    SPEAKER_FRONT_RIGHT_OF_CENTER = 0x00000080,
    SPEAKER_BACK_CENTER = 0x00000100,
    SPEAKER_SIDE_LEFT = 0x00000200,
    SPEAKER_SIDE_RIGHT = 0x00000400,
    SPEAKER_TOP_CENTER = 0x00000800,
    SPEAKER_TOP_FRONT_LEFT = 0x00001000,
    SPEAKER_TOP_FRONT_CENTER = 0x00002000,
    SPEAKER_TOP_FRONT_RIGHT = 0x00004000,
    SPEAKER_TOP_BACK_LEFT = 0x00008000,
    SPEAKER_TOP_BACK_CENTER = 0x00010000,
    SPEAKER_TOP_BACK_RIGHT = 0x00020000,
    SPEAKER_RESERVED = 0x7FFC0000,
    SPEAKER_ALL = 0x80000000
};

enum SpeakerLayoutMask
{
    SPEAKER_MONO = (SPEAKER_FRONT_CENTER),
    SPEAKER_STEREO = (SPEAKER_FRONT_LEFT | SPEAKER_FRONT_RIGHT),
    SPEAKER_2POINT1 = (SPEAKER_FRONT_LEFT | SPEAKER_FRONT_RIGHT | SPEAKER_LOW_FREQUENCY),
    SPEAKER_SURROUND = (SPEAKER_FRONT_LEFT | SPEAKER_FRONT_RIGHT | SPEAKER_FRONT_CENTER | SPEAKER_BACK_CENTER),
    SPEAKER_QUAD = (SPEAKER_FRONT_LEFT | SPEAKER_FRONT_RIGHT | SPEAKER_BACK_LEFT | SPEAKER_BACK_RIGHT),
    SPEAKER_4POINT1 = (SPEAKER_FRONT_LEFT | SPEAKER_FRONT_RIGHT | SPEAKER_LOW_FREQUENCY | SPEAKER_BACK_LEFT | SPEAKER_BACK_RIGHT),
    SPEAKER_5POINT1 = (SPEAKER_FRONT_LEFT | SPEAKER_FRONT_RIGHT | SPEAKER_FRONT_CENTER | SPEAKER_LOW_FREQUENCY | SPEAKER_BACK_LEFT | SPEAKER_BACK_RIGHT),
    SPEAKER_7POINT1 = (SPEAKER_FRONT_LEFT | SPEAKER_FRONT_RIGHT | SPEAKER_FRONT_CENTER | SPEAKER_LOW_FREQUENCY | SPEAKER_BACK_LEFT | SPEAKER_BACK_RIGHT | SPEAKER_FRONT_LEFT_OF_CENTER | SPEAKER_FRONT_RIGHT_OF_CENTER),
    SPEAKER_5POINT1_SURROUND = (SPEAKER_FRONT_LEFT | SPEAKER_FRONT_RIGHT | SPEAKER_FRONT_CENTER | SPEAKER_LOW_FREQUENCY | SPEAKER_SIDE_LEFT | SPEAKER_SIDE_RIGHT),
    SPEAKER_7POINT1_SURROUND = (SPEAKER_FRONT_LEFT | SPEAKER_FRONT_RIGHT | SPEAKER_FRONT_CENTER | SPEAKER_LOW_FREQUENCY | SPEAKER_BACK_LEFT | SPEAKER_BACK_RIGHT | SPEAKER_SIDE_LEFT | SPEAKER_SIDE_RIGHT),
};

//@todo verify mask values
inline int ComputeChannelMask(const size_t channels)
{
    switch (channels)
    {
    case 1: return SPEAKER_MONO;
    case 2: return SPEAKER_STEREO;
    case 3: return SPEAKER_2POINT1;
    case 4: return SPEAKER_QUAD;
    case 5: return SPEAKER_4POINT1;
    case 6: return SPEAKER_5POINT1;
    default: return -1; 
    }
}

/////////////////////
// Chunk utilities //
/////////////////////

struct ChunkHeaderInfo
{
    uint32_t offset;            // Byte offset into chunk
    uint32_t size;              // Size of the chunk in bytes
};

inline uint32_t GenerateChunkCode(uint8_t a, uint8_t b, uint8_t c, uint8_t d)
{
#ifdef ARCH_CPU_LITTLE_ENDIAN
    return ((uint32_t)((a) | ((b) << 8) | ((c) << 16) | (((uint32_t)(d)) << 24)));
#else
    return ((uint32_t)((((uint32_t)(a)) << 24) | ((b) << 16) | ((c) << 8) | (d)));
#endif
}

inline char * GenerateChunkCodeChar(uint8_t a, uint8_t b, uint8_t c, uint8_t d)
{
    auto chunk = GenerateChunkCode(a, b, c, d);

    char * outArr = new char[4];

    uint32_t t = 0x000000FF;

    for (size_t i = 0; i < 4; i++)
    {
        outArr[i] = chunk & t;
        chunk >>= 8;
    }
    return outArr;
}

inline ChunkHeaderInfo ScanForChunk(const std::vector<uint8_t> & fileData, uint32_t chunkMarker)
{
    // D[n] aligned to 16 bytes now
    const uint16_t * d = reinterpret_cast<const uint16_t *>(fileData.data());

    for (size_t i = 0; i < fileData.size() / sizeof(uint16_t); i++)
    {
        // This will be in machine endianess
        uint32_t m = Pack(Read16(d[i]), Read16(d[i + 1]));

        if (m == chunkMarker)
        {
            uint32_t cSz = Pack(Read16(d[i + 2]), Read16(d[i + 3]));
            return { (uint32_t(i * sizeof(uint16_t))), cSz }; // return i in bytes to the start of the data
        }
        else continue;
    }
    return { 0, 0 };
};

inline WaveChunkHeader MakeWaveHeader(const EncoderParams param, const int sampleRate)
{
    WaveChunkHeader header;

    int bitdepth = GetFormatBitsPerSample(param.targetFormat);

    header.fmt_id = GenerateChunkCode('f', 'm', 't', ' ');
    header.chunk_size = 16;
    header.format = (param.targetFormat <= PCMFormat::PCM_32) ? WaveFormatCode::FORMAT_PCM : WaveFormatCode::FORMAT_IEEE;
    header.channel_count = param.channelCount;
    header.sample_rate = sampleRate;
    header.data_rate = sampleRate * param.channelCount * (bitdepth / 8);
    header.frame_size = param.channelCount * (bitdepth / 8);
    header.bit_depth = bitdepth;

    return header;
}

// @todo expose this in the FLAC API
inline std::map<int, std::string> GetFlacQualityTable()
{
    return {
        { 0, "0 (Fastest)" },
        { 1, "1" },
        { 2, "2" },
        { 3, "3" },
        { 4, "4" },
        { 5, "5 (Default)" },
        { 6, "6" },
        { 7, "7" },
        { 8, "8 (Highest)" },
    };
}

template <typename T>
inline void DeinterleaveStereo(T * c1, T * c2, T const * src, size_t count)
{
    auto src_end = src + count;
    while (src != src_end)
    {
        *c1 = src[0];
        *c2 = src[1];
        c1++;
        c2++;
        src += 2;
    }
}

template<typename T>
void InterleaveChannels(const T * src, T * dest, size_t numFramesPerChannel, size_t numChannels, size_t N)
{
    for (size_t ch = 0; ch < numChannels; ch++)
    {
        size_t x = ch;
        const T * srcChannel = &src[ch * numFramesPerChannel];
        for (size_t i = 0; i < N; i++)
        {
            dest[x] = srcChannel[i];
            x += numChannels;
        }
    }
}

template<typename T>
void DeinterleaveChannels(const T * src, T * dest, size_t numFramesPerChannel, size_t numChannels, size_t N)
{
    for (size_t ch = 0; ch < numChannels; ch++)
    {
        size_t x = ch;
        T *destChannel = &dest[ch * numFramesPerChannel];
        for (size_t i = 0; i < N; i++)
        {
            destChannel[i] = (T)src[x];
            x += numChannels;
        }
    }
}

template <typename T>
void StereoToMono(const T * src, T * dest, size_t N)
{
    for (size_t i = 0, j = 0; i < N; i += 2, ++j)
    {
        dest[j] = (src[i] + src[i + 1]) / 2.0f;
    }
}

template <typename T>
void MonoToStereo(const T * src, T * dest, size_t N)
{
    for (size_t i = 0, j = 0; i < N; ++i, j += 2)
    {
        dest[j] = src[i];
        dest[j + 1] = src[i];
    }
}

inline void TrimSilenceInterleaved(std::vector<float> & buffer, float v, bool fromFront, bool fromEnd)
{
    //@todo implement me!
}

} // end namespace nqr

#endif
