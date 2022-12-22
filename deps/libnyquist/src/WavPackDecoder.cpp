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
#include "wavpack.h"
#include <string.h>
#include <cstring>

using namespace nqr;

class WavPackInternal
{
    
public:
    
    WavPackInternal(AudioData * d, const std::string & path) : d(d)
    {
        char errorStr[128];
        context = WavpackOpenFileInput(path.c_str(), errorStr, OPEN_WVC | OPEN_NORMALIZE, 0);
        
        if (!context) throw std::runtime_error("Not a WavPack file");

        auto totalSamples = size_t(WavpackGetNumSamples(context));

        decode(totalSamples);
    }

    WavPackInternal(AudioData * d, const std::vector<uint8_t> & memory) : d(d)
    {
        char errorStr[128];
        context = WavpackOpenRawDecoder((void *) memory.data(), memory.size(), nullptr, 0, 0, errorStr, OPEN_WVC | OPEN_NORMALIZE, 0);

        // Since we are using OpenRawDecoder, WavpackGetNumSamples won't work.
        // Instead, find the first block and get totalSamples from its header.
        WavpackHeader wph;
        auto headerOffset = readNextHeader(memory, &wph, 0);

        if (!context || headerOffset == -1)
        {
            throw std::runtime_error("Not a WavPack file");
        }

        auto totalSamples = wph.total_samples;

        decode(totalSamples);
    }
    
    ~WavPackInternal()
    {
        WavpackCloseFile(context);
    }
    
    size_t readInternal(size_t requestedFrameCount, size_t frameOffset = 0)
    {
        size_t framesRemaining = requestedFrameCount;
        size_t totalFramesRead = 0;
        
        // The samples returned are handled differently based on the file's mode
        int mode = WavpackGetMode(context);
        
        while (0 < framesRemaining)
        {
            uint32_t framesRead = -1;
            
            if (MODE_FLOAT & mode)
            {
                // Since it's float, we can decode directly into our buffer as a huge blob
                framesRead = WavpackUnpackSamples(context, reinterpret_cast<int32_t*>(&d->samples.data()[0]), uint32_t(d->samples.size() / d->channelCount));
            }
            
            else if(MODE_LOSSLESS & mode)
            {
                // Lossless files will be handed off as integers
                framesRead = WavpackUnpackSamples(context, internalBuffer.data(), uint32_t(internalBuffer.size() / d->channelCount));
            }
            
            // EOF
            //if (framesRead == 0) break;

            totalFramesRead += framesRead;
            framesRemaining -= framesRead;
        }

        return totalFramesRead;
    }

private:
    void decode(size_t totalSamples) {
        auto bitdepth = WavpackGetBitsPerSample(context);

        d->sampleRate = WavpackGetSampleRate(context);
        d->channelCount = WavpackGetNumChannels(context);
        d->lengthSeconds = double(totalSamples / WavpackGetSampleRate(context));
        d->frameSize = d->channelCount * bitdepth;

        //@todo support channel masks
        // WavpackGetChannelMask

        int mode = WavpackGetMode(context);
        bool isFloatingPoint = (MODE_FLOAT & mode);

        d->sourceFormat = MakeFormatForBits(bitdepth, isFloatingPoint, false);

        d->samples.resize(totalSamples * d->channelCount);

        if (!isFloatingPoint)
            internalBuffer.resize(totalSamples * d->channelCount);

        if (!readInternal(totalSamples))
            throw std::runtime_error("could not read any data");

        // Next, process internal buffer into the user-visible samples array
        if (!isFloatingPoint)
            ConvertToFloat32(d->samples.data(), internalBuffer.data(), totalSamples * d->channelCount, d->sourceFormat);
    }

    int64_t readNextHeader(const std::vector<uint8_t> & memory, WavpackHeader *wphdr, size_t startOffset) {
        /// Based on read_next_header function in wavpack's openutils.c.
        /// This will find the position of the next WavPack header in the given vector, at or after startOffset.
        /// If a header is found, it will write the header to *wphdr and return the position of its first byte in the vector.
        /// Otherwise, it will return -1.
        unsigned char* sp;

        for (size_t i = startOffset; i < memory.size(); i++) {
            sp = const_cast<unsigned char *>(memory.data() + i);

            auto headerStartPoint = sp;

            if (*sp++ == 'w' && *sp == 'v' && *++sp == 'p' && *++sp == 'k' &&
                !(*++sp & 1) && sp [2] < 16 && !sp [3] && (sp [2] || sp [1] || *sp >= 24) && sp [5] == 4 &&
                sp [4] >= (MIN_STREAM_VERS & 0xff) && sp [4] <= (MAX_STREAM_VERS & 0xff) && sp [18] < 3 && !sp [19]) {

                memcpy (wphdr, headerStartPoint, sizeof (*wphdr));
                WavpackLittleEndianToNative (wphdr, (char*)WavpackHeaderFormat);
                return i;
            }
        }

        return -1;
    }
    
    NO_MOVE(WavPackInternal);
    
    //WavpackStreamReader streamReader; //@todo: streaming support
    
    WavpackContext * context; //@todo unique_ptr
    
    AudioData * d;

    std::vector<int32_t> internalBuffer;
    
    inline int64_t getTotalSamples() const { return WavpackGetNumSamples(context); }
    inline int64_t getLengthInSeconds() const { return getTotalSamples() / WavpackGetSampleRate(context); }
    
};

//////////////////////
// Public Interface //
//////////////////////

void WavPackDecoder::LoadFromPath(AudioData * data, const std::string & path)
{
    WavPackInternal decoder(data, path);
}

void WavPackDecoder::LoadFromBuffer(AudioData * data, const std::vector<uint8_t> & memory)
{
    WavPackInternal decoder(data, memory);
}

std::vector<std::string> WavPackDecoder::GetSupportedFileExtensions()
{
    return {"wv"};
}
