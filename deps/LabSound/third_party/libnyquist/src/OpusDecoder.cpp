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
#include "opus/opusfile/include/opusfile.h"

using namespace nqr;

static const int OPUS_SAMPLE_RATE = 48000;

// Opus is a general-purpose codec designed to replace Vorbis at some point. Primarily, it's a low
// delay format making it suitable for high-quality, real time streaming. It's not really
// an archival format or something designed for heavy DSP post-processing since
// it's fundamentally limited to encode/decode at 48khz.
// https://mf4.xiph.org/jenkins/view/opus/job/opusfile-unix/ws/doc/html/index.html

class OpusDecoderInternal
{
    
public:
    
    OpusDecoderInternal(AudioData * d, const std::vector<uint8_t> & fileData) : d(d)
    {
        /* @todo proper steaming support + classes
        const opus_callbacks = {
            .read = s_readCallback,
            .seek = s_seekCallback,
            .tell = s_tellCallback,
            .close = nullptr
        };
         */
        
        int err;
        
        fileHandle = op_test_memory(fileData.data(), fileData.size(), &err);
        
        if (!fileHandle)
        {
            std::cerr << errorAsString(err) << std::endl;
            throw std::runtime_error("File is not a valid ogg vorbis file");
        }
        
        if (auto r = op_test_open(fileHandle) != 0)
        {
            std::cerr << errorAsString(r) << std::endl;
            throw std::runtime_error("Could not open file");
        }
        
        const OpusHead * header = op_head(fileHandle, 0);

        // int originalSampleRate = header->input_sample_rate;

        d->sampleRate = OPUS_SAMPLE_RATE;
        d->channelCount = (uint32_t) header->channel_count;
        d->sourceFormat = MakeFormatForBits(32, true, false);
        d->lengthSeconds = double(getLengthInSeconds());
        d->frameSize = (uint32_t) header->channel_count * GetFormatBitsPerSample(d->sourceFormat);
        
        // Samples in a single channel
        auto totalSamples = size_t(getTotalSamples());
        
        d->samples.resize(totalSamples * d->channelCount);
        
        if (!readInternal(totalSamples))
            throw std::runtime_error("could not read any data");
    }
    
    ~OpusDecoderInternal()
    {
        op_free(fileHandle);
    }
    
    size_t readInternal(size_t requestedFrameCount, size_t frameOffset = 0)
    {
        float * buffer = (float *) d->samples.data();
        size_t framesRemaining = requestedFrameCount;
        size_t totalFramesRead = 0;
        
        while(0 < framesRemaining)
        {
            int framesRead = op_read_float(fileHandle, buffer, (int)(framesRemaining * d->channelCount), nullptr);
            
            // EOF
            if(!framesRead)
                break;
            
            if (framesRead < 0)
            {
                std::cerr << "Opus decode error: " << framesRead << std::endl;
                return 0;
            }
            
            buffer += framesRead * d->channelCount;
            
            totalFramesRead += framesRead;
            framesRemaining -= framesRead;
        }
        
        return totalFramesRead;
    }

    std::string errorAsString(int opusErrorCode)
    {
        switch(opusErrorCode)
        {
            case OP_FALSE: return "A request did not succeed";
            case OP_EOF: return "End of File Reached";
            case OP_HOLE: return "There was a hole in the page sequence numbers (e.g., a page was corrupt or missing).";
            case OP_EREAD: return "An underlying read, seek, or tell operation failed when it should have succeeded.";
            case OP_EFAULT: return "A NULL pointer was passed where one was unexpected, or an internal memory allocation failed, or an internal library error was encountered.";
            case OP_EIMPL: return "The stream used a feature that is not implemented, such as an unsupported channel family. ";
            case OP_EINVAL: return "One or more parameters to a function were invalid. ";
            case OP_ENOTFORMAT: return "A purported Ogg Opus stream did not begin with an Ogg page, a purported header packet did not start with one of the required strings";
            case OP_EBADHEADER: return "A required header packet was not properly formatted, contained illegal values, or was missing altogether.";
            case OP_EVERSION: return "The ID header contained an unrecognized version number.";
            case OP_ENOTAUDIO: return "Not Audio";
            case OP_EBADPACKET: return "An audio packet failed to decode properly.";
            case OP_EBADLINK: return "We failed to find data we had seen before, or the bitstream structure was sufficiently malformed that seeking to the target destination was impossible.";
            case OP_ENOSEEK: return "An operation that requires seeking was requested on an unseekable stream.";
            case OP_EBADTIMESTAMP: return "The first or last granule position of a link failed basic validity checks.";
            default: return "Unknown Error";
        }
    }
    
    ////////////////////
    // opus callbacks //
    ////////////////////
    
private:
    
    NO_MOVE(OpusDecoderInternal);

    OggOpusFile * fileHandle;
    
    AudioData * d;
    
    inline int64_t getTotalSamples() const { return int64_t(op_pcm_total(const_cast<OggOpusFile *>(fileHandle), -1)); }
    inline int64_t getLengthInSeconds() const { return uint64_t(getTotalSamples() / OPUS_SAMPLE_RATE); }
    inline int64_t getCurrentSample() const { return int64_t(op_pcm_tell(const_cast<OggOpusFile *>(fileHandle))); }
    
};

//////////////////////
// Public Interface //
//////////////////////

void nqr::OpusDecoder::LoadFromPath(AudioData * data, const std::string & path)
{
    auto fileBuffer = nqr::ReadFile(path);
    OpusDecoderInternal decoder(data, fileBuffer.buffer);
}

void nqr::OpusDecoder::LoadFromBuffer(AudioData * data, const std::vector<uint8_t> & memory)
{
    OpusDecoderInternal decoder(data, memory);
}

std::vector<std::string> nqr::OpusDecoder::GetSupportedFileExtensions()
{
    return {"opus"};
}
