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

#include "Decoders.h"
#include <cstring>

using namespace nqr;

struct ADPCMState
{
    int frame_size;
    int firstDataBlockByte;
    int dataSize;
    int currentByte;
    const uint8_t * inBuffer;
};

static const int ima_index_table[16] =
{
    -1, -1, -1, -1,  // +0 / +3 : - the step
    2, 4, 6, 8,      // +4 / +7 : + the step
    -1, -1, -1, -1,  // -0 / -3 : - the step
    2, 4, 6, 8,      // -4 / -7 : + the step
};

static inline int ima_clamp_index(int index)
{
    if (index < 0) return 0;
    else if (index > 88) return 88;
    return index;
}

static inline int16_t ima_clamp_predict(int16_t predict)
{
    if (predict < -32768) return -32768;
    else if (predict > 32767) return 32767;
    return predict;
}

static const int ima_step_table[89] =
{
    7, 8, 9, 10, 11, 12, 13, 14, 16, 17, 19, 21, 23, 25, 28, 31, 34,
    37, 41, 45, 50, 55, 60, 66, 73, 80, 88, 97, 107, 118, 130, 143,
    157, 173, 190, 209, 230, 253, 279, 307, 337, 371, 408, 449, 494,
    544, 598, 658, 724, 796, 876, 963, 1060, 1166, 1282, 1411, 1552,
    1707, 1878, 2066, 2272, 2499, 2749, 3024, 3327, 3660, 4026,
    4428, 4871, 5358, 5894, 6484, 7132, 7845, 8630, 9493, 10442,
    11487, 12635, 13899, 15289, 16818, 18500, 20350, 22385, 24623,
    27086, 29794, 32767
};

// Decodes an IMA ADPCM nibble to a 16 bit pcm sample
static inline int16_t decode_nibble(uint8_t nibble, int16_t & p, int & s)
{
    // Compute a delta to add to the predictor value
    int diff = ima_step_table[s] >> 3;
    if (nibble & 4) diff += ima_step_table[s];
    if (nibble & 2) diff += ima_step_table[s] >> 1;
    if (nibble & 1) diff += ima_step_table[s] >> 2;

    // Sign
    if (nibble & 8) diff = -diff;

    // Add delta
    p += diff;

    s += ima_index_table[nibble];
    s = ima_clamp_index(s);

    return ima_clamp_predict(p);
}

void decode_ima_adpcm(ADPCMState & state, int16_t * outBuffer, uint32_t num_channels)
{
    const uint8_t * data = state.inBuffer;

    // Loop over the interleaved channels
    for (uint32_t ch = 0; ch < num_channels; ch++)
    {
        const int byteOffset = ch * 4;

        // Header Structure:
        // Byte0: packed low byte of the initial predictor
        // Byte1: packed high byte of the initial predictor
        // Byte2: initial step index
        // Byte3: Reserved empty value
        int16_t predictor = ((int16_t)data[byteOffset + 1] << 8) | data[byteOffset];
        int stepIndex = data[byteOffset + 2];

        uint8_t reserved = data[byteOffset + 3];
        if (reserved != 0) throw std::runtime_error("adpcm decode error");

        int byteIdx = num_channels * 4 + byteOffset; //the byte index of the first data word for this channel
        int idx = ch;

        // Decode nibbles of the remaining data
        while (byteIdx < state.frame_size)
        {
            for (int j = 0; j < 4; j++)
            {
                outBuffer[idx] = decode_nibble(data[byteIdx] & 0xf, predictor, stepIndex); // low nibble
                idx += num_channels;
                outBuffer[idx] = decode_nibble(data[byteIdx] >> 4, predictor, stepIndex); // high nibble
                idx += num_channels;
                byteIdx++;
            }
            byteIdx += (num_channels - 1) << 2; // Jump to the next data word for the current channel
        }

    }

}

//////////////////////
// Public Interface //
//////////////////////

void WavDecoder::LoadFromPath(AudioData * data, const std::string & path)
{
    auto fileBuffer = nqr::ReadFile(path);
    return LoadFromBuffer(data, fileBuffer.buffer);
}

void WavDecoder::LoadFromBuffer(AudioData * data, const std::vector<uint8_t> & memory)
{
    //////////////////////
    // Read RIFF Header //
    //////////////////////
    
    //@todo swap methods for rifx
    
    RiffChunkHeader riffHeader = {};
    memcpy(&riffHeader, memory.data(), 12);
    
    // Files should be 2-byte aligned
    // @tofix: enforce this
    // bool usePaddingShort = ((riffHeader.file_size % sizeof(uint16_t)) == 1) ? true : false;
    
    // Check RIFF
    if (riffHeader.id_riff != GenerateChunkCode('R', 'I', 'F', 'F'))
    {
        // Check RIFX + FFIR
        if (riffHeader.id_riff == GenerateChunkCode('R', 'I', 'F', 'X') || riffHeader.id_riff == GenerateChunkCode('F', 'F', 'I', 'R'))
        {
            // We're not RIFF, and we don't match RIFX or FFIR either
            throw std::runtime_error("libnyquist doesn't support big endian files");
        }
        else
        {
            throw std::runtime_error("bad RIFF/RIFX/FFIR file header");
        }
    }
    
    if (riffHeader.id_wave != GenerateChunkCode('W', 'A', 'V', 'E')) throw std::runtime_error("bad WAVE header");
    
    auto expectedSize = (memory.size() - riffHeader.file_size);
    if (expectedSize != sizeof(uint32_t) * 2)
    {
        throw std::runtime_error("declared size of file less than file size"); //@todo warning instead of runtime_error
    }
    
    //////////////////////
    // Read WAVE Header //
    //////////////////////
    
    auto WaveChunkInfo = ScanForChunk(memory, GenerateChunkCode('f', 'm', 't', ' '));
    
    if (WaveChunkInfo.offset == 0) throw std::runtime_error("couldn't find fmt chunk");
    
    assert(WaveChunkInfo.size == 16 || WaveChunkInfo.size == 18 || WaveChunkInfo.size == 20 || WaveChunkInfo.size == 40);
    
    WaveChunkHeader wavHeader = {};
    memcpy(&wavHeader, memory.data() + WaveChunkInfo.offset, sizeof(WaveChunkHeader));
    
    if (wavHeader.chunk_size < 16)
        throw std::runtime_error("format chunk too small");
        
    //@todo validate wav header (sane sample rate, bit depth, etc)
    
    data->channelCount = wavHeader.channel_count;
    data->sampleRate = wavHeader.sample_rate;
    data->frameSize = wavHeader.frame_size;
    
    auto bit_depth = wavHeader.bit_depth;
    switch (bit_depth)
    {
        case 4: data->sourceFormat = PCMFormat::PCM_16; break; // for IMA ADPCM
        case 8: data->sourceFormat = PCMFormat::PCM_U8; break;
        case 16: data->sourceFormat = PCMFormat::PCM_16; break;
        case 24: data->sourceFormat = PCMFormat::PCM_24; break;
        case 32: data->sourceFormat = (wavHeader.format == WaveFormatCode::FORMAT_IEEE) ? PCMFormat::PCM_FLT : PCMFormat::PCM_32; break;
        case 64: data->sourceFormat = (wavHeader.format == WaveFormatCode::FORMAT_IEEE) ? PCMFormat::PCM_DBL : PCMFormat::PCM_64; break;
    }
    
    //std::cout << wavHeader << std::endl;
    
    bool scanForFact = false;
    bool grabExtensibleData = false;
    bool adpcmEncoded = false;
    
    if (wavHeader.format == WaveFormatCode::FORMAT_IEEE)
    {
        scanForFact = true;
    }
    else if (wavHeader.format == WaveFormatCode::FORMAT_IMA_ADPCM)
    {
        adpcmEncoded = true;
        scanForFact = true;
    }
    else if (wavHeader.format == WaveFormatCode::FORMAT_EXT)
    {
        // Used when (1) PCM data has more than 16 bits; (2) channels > 2; (3) bits/sample !== container size; (4) channel/speaker mapping specified;
        scanForFact = true;
        grabExtensibleData = true;
    }
    else if (wavHeader.format ==  WaveFormatCode::FORMAT_UNKNOWN)
    {
        throw std::runtime_error("unknown wave format");
    }
    
    ////////////////////////////
    // Read Additional Chunks //
    ////////////////////////////
    
    FactChunk factChunk;
    if (scanForFact)
    {
        auto FactChunkInfo = ScanForChunk(memory, GenerateChunkCode('f', 'a', 'c', 't'));
        if (FactChunkInfo.size)
            memcpy(&factChunk, memory.data() + FactChunkInfo.offset, sizeof(FactChunk));
    }
    
    if (grabExtensibleData)
    {
        ExtensibleData extData = {};
        memcpy(&extData, memory.data() + WaveChunkInfo.offset + sizeof(WaveChunkHeader), sizeof(ExtensibleData));
        // extData can be compared against the multi-channel masks defined in the header
        // eg. extData.channel_mask == SPEAKER_5POINT1
    }
    
     //@todo smpl chunk could be useful
    
    /////////////////////
    // Read Bext Chunk //
    /////////////////////
    
    auto BextChunkInfo = ScanForChunk(memory, GenerateChunkCode('b', 'e', 'x', 't'));
    BextChunk bextChunk = {};
    
    if (BextChunkInfo.size)
    {
        memcpy(&bextChunk, memory.data() + BextChunkInfo.offset, sizeof(BextChunk));
    }
    
    /////////////////////
    // Read DATA Chunk //
    /////////////////////
    
    auto DataChunkInfo = ScanForChunk(memory, GenerateChunkCode('d', 'a', 't', 'a'));
    
    if (DataChunkInfo.offset == 0) 
        throw std::runtime_error("couldn't find data chunk");
    
    DataChunkInfo.offset += 2 * sizeof(uint32_t); // ignore the header and size fields

    if (adpcmEncoded)
    {
        ADPCMState s;
        s.frame_size = wavHeader.frame_size;
        s.firstDataBlockByte = 0;
        s.dataSize = DataChunkInfo.size;
        s.currentByte = 0;
        s.inBuffer = const_cast<uint8_t*>(memory.data() + DataChunkInfo.offset);
        
        size_t totalSamples = (factChunk.sample_length * wavHeader.channel_count); // Samples per channel times channel count
        std::vector<int16_t> adpcm_pcm16(totalSamples * 2, 0); // Each frame decodes into twice as many pcm samples
        
        uint32_t frameOffset = 0;
        uint32_t frameCount = DataChunkInfo.size / s.frame_size;

        for (uint32_t i = 0; i < frameCount; ++i)
        {
            decode_ima_adpcm(s, adpcm_pcm16.data() + frameOffset, wavHeader.channel_count);
            s.inBuffer += s.frame_size;
            frameOffset += (s.frame_size * 2) - (8 * wavHeader.channel_count);
        }
        
        data->lengthSeconds = ((float) totalSamples / (float) wavHeader.sample_rate) / wavHeader.channel_count;
        data->samples.resize(totalSamples);
        ConvertToFloat32(data->samples.data(), adpcm_pcm16.data(), totalSamples, data->sourceFormat);
    }
    else
    {
        data->lengthSeconds = ((float) DataChunkInfo.size / (float) wavHeader.sample_rate) / wavHeader.frame_size;
        size_t totalSamples = (DataChunkInfo.size / wavHeader.frame_size) * wavHeader.channel_count;
        data->samples.resize(totalSamples);
        ConvertToFloat32(data->samples.data(), memory.data() + DataChunkInfo.offset, totalSamples, data->sourceFormat);
    }
}

std::vector<std::string> WavDecoder::GetSupportedFileExtensions()
{
    return {"wav", "wave"};
}
