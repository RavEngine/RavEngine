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
#include "libvorbis/include/vorbis/vorbisfile.h"

#include <string.h>

using namespace nqr;

class VorbisDecoderInternal
{
    
public:
    
    VorbisDecoderInternal(AudioData * d, const std::vector<uint8_t> & memory) : d(d)
    {
        void * data = const_cast<uint8_t*>(memory.data());
        
        ogg_file t;
        t.curPtr = t.filePtr = static_cast<char*>(data);
        t.fileSize = memory.size();
        
        fileHandle = new OggVorbis_File;
        memset(fileHandle, 0, sizeof(OggVorbis_File));
        
        ov_callbacks callbacks;
        callbacks.read_func = AR_readOgg;
        callbacks.seek_func = AR_seekOgg;
        callbacks.close_func = AR_closeOgg;
        callbacks.tell_func = AR_tellOgg;
        
        loadAudioData(&t, callbacks);
    }
    
    VorbisDecoderInternal(AudioData * d, std::string filepath) : d(d)
    {
        fileHandle = new OggVorbis_File();
        FILE * f = fopen(filepath.c_str(), "rb");
        if (!f) throw std::runtime_error("Can't open file");
        loadAudioData(f, OV_CALLBACKS_DEFAULT);
    }
    
    ~VorbisDecoderInternal()
    {
        ov_clear(fileHandle);
    }
    
    size_t readInternal(size_t requestedFrameCount, size_t frameOffset = 0)
    {
        //@todo support offset
        
        float **buffer = nullptr;
        size_t framesRemaining = requestedFrameCount;
        size_t totalFramesRead = 0;
        int bitstream = 0;
        
        while(0 < framesRemaining)
        {
            int64_t framesRead = ov_read_float(fileHandle, &buffer, std::min(2048, (int) framesRemaining), &bitstream);
            
            // end of file
            if(!framesRead) break;
            
            // Probably OV_HOLE, OV_EBADLINK, OV_EINVAL. @todo - log warning here.
            if (framesRead < 0)
            {
                continue;
            }
            
            for (int i = 0; i < framesRead; ++i)
            {
                for(int ch = 0; ch < d->channelCount; ch++)
                {
                    d->samples[totalFramesRead] = buffer[ch][i];
                    totalFramesRead++;
                }
            }
        }
        
        return totalFramesRead;
    }
    
    std::string errorAsString(int ovErrorCode)
    {
        switch(ovErrorCode)
        {
            case OV_FALSE: return "OV_FALSE";
            case OV_EOF: return "OV_EOF";
            case OV_HOLE: return "OV_HOLE";
            case OV_EREAD: return "OV_EREAD";
            case OV_EFAULT: return "OV_EFAULT";
            case OV_EIMPL: return "OV_EIMPL";
            case OV_EINVAL: return "OV_EINVAL";
            case OV_ENOTVORBIS: return "OV_ENOTVORBIS";
            case OV_EBADHEADER: return "OV_EBADHEADER";
            case OV_EVERSION: return "OV_EVERSION";
            case OV_ENOTAUDIO: return "OV_ENOTAUDIO";
            case OV_EBADPACKET: return "OV_EBADPACKET";
            case OV_EBADLINK: return "OV_EBADLINK";
            case OV_ENOSEEK: return "OV_ENOSEEK";
            default: return "OV_UNKNOWN_ERROR";
        }
    }
    
    //////////////////////
    // vorbis callbacks //
    //////////////////////
    
    //@todo: implement streaming support
    
private:
    
    struct ogg_file
    {
        char* curPtr;
        char* filePtr;
        size_t fileSize;
    };
    
    NO_COPY(VorbisDecoderInternal);
    
    OggVorbis_File * fileHandle;
    AudioData * d;
    
    inline int64_t getTotalSamples() const { return int64_t(ov_pcm_total(const_cast<OggVorbis_File *>(fileHandle), -1)); }
    inline int64_t getLengthInSeconds() const { return int64_t(ov_time_total(const_cast<OggVorbis_File *>(fileHandle), -1)); }
    inline int64_t getCurrentSample() const { return int64_t(ov_pcm_tell(const_cast<OggVorbis_File *>(fileHandle))); }
    
    static size_t AR_readOgg(void* dst, size_t size1, size_t size2, void* fh)
    {
        ogg_file* of = reinterpret_cast<ogg_file*>(fh);
        size_t len = size1 * size2;
        if ( of->curPtr + len > of->filePtr + of->fileSize )
        {
            len = of->filePtr + of->fileSize - of->curPtr;
        }
        memcpy( dst, of->curPtr, len );
        of->curPtr += len;
        return len;
    }
    
    static int AR_seekOgg(void * fh, ogg_int64_t to, int type) 
    {
        ogg_file * of = reinterpret_cast<ogg_file*>(fh);
    
        switch (type)
        {
            case SEEK_CUR: of->curPtr += to; break;
            case SEEK_END: of->curPtr = of->filePtr + of->fileSize - to; break;
            case SEEK_SET: of->curPtr = of->filePtr + to; break;
            default: return -1;
        }

        if (of->curPtr < of->filePtr) 
        {
            of->curPtr = of->filePtr;
            return -1;
        }

        if (of->curPtr > of->filePtr + of->fileSize)
        {
            of->curPtr = of->filePtr + of->fileSize;
            return -1;
        }

        return 0;
    }
    
    static int AR_closeOgg(void * fh) 
    {
        return 0;
    }
    
    static long AR_tellOgg(void * fh)
    {
        ogg_file * of = reinterpret_cast<ogg_file*>(fh);
        return (of->curPtr - of->filePtr);
    }
    
    void loadAudioData(void *source, ov_callbacks callbacks)
    {
        if (auto r = ov_test_callbacks(source, fileHandle, nullptr, 0, callbacks) != 0)
        {
            std::cerr << errorAsString(r) << std::endl;
            throw std::runtime_error("File is not a valid ogg vorbis file");
        }
        
        if (auto r = ov_test_open(fileHandle) != 0)
        {
            std::cerr << errorAsString(r) << std::endl;
            throw std::runtime_error("ov_test_open failed");
        }
        
        // Don't need to fclose() after an open -- vorbis does this internally
        
        vorbis_info * ovInfo = ov_info(fileHandle, -1);
        
        if (ovInfo == nullptr) throw std::runtime_error("Reading metadata failed");
        
        if (auto r = ov_streams(fileHandle) != 1)
        {
            std::cerr << errorAsString(r) << std::endl;
            throw std::runtime_error( "Unsupported: file contains multiple bitstreams");
        }
        
        d->sampleRate = int(ovInfo->rate);
        d->channelCount = ovInfo->channels;
        d->sourceFormat = MakeFormatForBits(32, true, false);
        d->lengthSeconds = double(getLengthInSeconds());
        d->frameSize = ovInfo->channels * GetFormatBitsPerSample(d->sourceFormat);
        
        // Samples in a single channel
        auto totalSamples = size_t(getTotalSamples());
        
        d->samples.resize(totalSamples * d->channelCount);
        
        if (!readInternal(totalSamples)) throw std::runtime_error("could not read any data");
    }
    
};

//////////////////////
// Public Interface //
//////////////////////

void VorbisDecoder::LoadFromPath(AudioData * data, const std::string & path)
{
    VorbisDecoderInternal decoder(data, path);
}

void VorbisDecoder::LoadFromBuffer(AudioData * data, const std::vector<uint8_t> & memory)
{
    VorbisDecoderInternal decoder(data, memory);
}

std::vector<std::string> VorbisDecoder::GetSupportedFileExtensions()
{
    return {"ogg"};
}
