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

using namespace nqr;

#include "mpc/mpcdec.h"
#include "mpc/reader.h"
#include "musepack/libmpcdec/decoder.h"
#include "musepack/libmpcdec/internal.h"

#define MINIMP3_FLOAT_OUTPUT
#define MINIMP3_IMPLEMENTATION
#include "minimp3/minimp3.h"
#include "minimp3/minimp3_ex.h"

#include <cstring>

void mp3_decode_internal(AudioData * d, const std::vector<uint8_t> & fileData)
{
    mp3dec_t mp3d;
    mp3dec_file_info_t info;
    mp3dec_load_buf(&mp3d, (const uint8_t*)fileData.data(), fileData.size(), &info, 0, 0);

    d->sampleRate = info.hz;
    d->channelCount = info.channels;
    d->sourceFormat = MakeFormatForBits(32, true, false);
    d->lengthSeconds = ((float)info.samples / (float)d->channelCount) / (float)d->sampleRate;

    if (info.samples == 0) throw std::runtime_error("mp3: could not read any data");

    d->samples.resize(info.samples);
    std::memcpy(d->samples.data(), info.buffer, sizeof(float) * info.samples);

    delete info.buffer;
}

//////////////////////
// Public Interface //
//////////////////////

void Mp3Decoder::LoadFromPath(AudioData * data, const std::string & path)
{
    auto fileBuffer = nqr::ReadFile(path);
    mp3_decode_internal(data, fileBuffer.buffer);
}

void Mp3Decoder::LoadFromBuffer(AudioData * data, const std::vector<uint8_t> & memory)
{
    mp3_decode_internal(data, memory);
}

std::vector<std::string> Mp3Decoder::GetSupportedFileExtensions()
{
    return {"mp3"};
}
