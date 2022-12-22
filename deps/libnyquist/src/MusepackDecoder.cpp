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

class MusepackInternal
{
    
    static const uint32_t STDIO_MAGIC = 0xF36D656D;
    
    // Methods borrowed from r-lyeh (https://github.com/r-lyeh) (zlib)
    struct mpc_reader_state
    {
        unsigned char *p_file;
        unsigned char *p_begin, *p_end;
        mpc_bool_t is_seekable;
        mpc_int32_t magic;
    };
    
    static mpc_int32_t read_mem(mpc_reader *p_reader, void *ptr, mpc_int32_t size)
    {
        mpc_reader_state *p_mem = (mpc_reader_state*) p_reader->data;
        if (p_mem->magic != STDIO_MAGIC) return MPC_STATUS_FAIL;
        mpc_int32_t max = mpc_int32_t(p_mem->p_end - p_mem->p_file);
        if (size >= max) size = max;
        memcpy((unsigned char *)ptr, p_mem->p_file, size);
        p_mem->p_file += size;
        return size;
    }
    
    static mpc_bool_t seek_mem(mpc_reader *p_reader, mpc_int32_t offset)
    {
        mpc_reader_state *p_mem = (mpc_reader_state*) p_reader->data;
        if (p_mem->magic != STDIO_MAGIC) return MPC_FALSE;
        if (!p_mem->is_seekable) return MPC_FALSE;
        p_mem->p_file = p_mem->p_begin + offset;
        if(p_mem->p_file <  p_mem->p_begin) return MPC_FALSE;
        if(p_mem->p_file >= p_mem->p_end  ) return MPC_FALSE;
        return MPC_TRUE;
    }
    
    static mpc_int32_t tell_mem(mpc_reader *p_reader)
    {
        mpc_reader_state *p_mem = (mpc_reader_state*) p_reader->data;
        if(p_mem->magic != STDIO_MAGIC) return MPC_STATUS_FAIL;
        return mpc_int32_t(p_mem->p_file - p_mem->p_begin);
    }
    
    static mpc_int32_t get_size_mem(mpc_reader *p_reader)
    {
        mpc_reader_state *p_mem = (mpc_reader_state*) p_reader->data;
        if (p_mem->magic != STDIO_MAGIC) return MPC_STATUS_FAIL;
        return mpc_int32_t(p_mem->p_end - p_mem->p_begin);
    }
    
    static mpc_bool_t canseek_mem(mpc_reader *p_reader)
    {
        mpc_reader_state *p_mem = (mpc_reader_state*) p_reader->data;
        if (p_mem->magic != STDIO_MAGIC) return MPC_FALSE;
        return p_mem->is_seekable;
    }
    
public:
    
    // Musepack is a purely variable bitrate format and does not work at a constant bitrate.
    MusepackInternal(AudioData * d, const std::vector<uint8_t> & fileData) : d(d)
    {
        decoderMemory = std::make_shared<mpc_reader_state>();
        
        decoderMemory->magic  = STDIO_MAGIC;
        decoderMemory->p_file = (unsigned char *) fileData.data();
        decoderMemory->p_begin = (unsigned char *) fileData.data();
        decoderMemory->p_end = (unsigned char *) fileData.data() + fileData.size();
        decoderMemory->is_seekable = MPC_TRUE;
        
        reader.data = decoderMemory.get();
        reader.canseek = canseek_mem;
        reader.get_size = get_size_mem;
        reader.read = read_mem;
        reader.seek = seek_mem;
        reader.tell = tell_mem;
        
        mpcDemux = mpc_demux_init(&reader);
        if (!mpcDemux) throw std::runtime_error("could not initialize mpc demuxer");
        
        mpc_demux_get_info(mpcDemux, &streamInfo);
        
        d->sampleRate = (int) streamInfo.sample_freq;
        d->channelCount = streamInfo.channels;
        d->sourceFormat = MakeFormatForBits(32, true, false);
        d->lengthSeconds = (double) mpc_streaminfo_get_length(&streamInfo);
        
        auto totalSamples = size_t(mpc_streaminfo_get_length_samples(&streamInfo));
        d->samples.reserve(totalSamples * d->channelCount + (MPC_DECODER_BUFFER_LENGTH / sizeof(MPC_SAMPLE_FORMAT))); // demux writes in chunks
        d->samples.resize(totalSamples * d->channelCount);
        
        if (!readInternal())
            throw std::runtime_error("could not read any data");
    }
    
    size_t readInternal()
    {
        mpc_status err = MPC_STATUS_OK;
        size_t totalSamplesRead = 0;
   
        while (true)
        {
            mpc_frame_info frame;
            frame.buffer = d->samples.data() + totalSamplesRead;

            err = mpc_demux_decode(mpcDemux, &frame);
            if (frame.bits == -1 || err != MPC_STATUS_OK) break;

            totalSamplesRead += (frame.samples * streamInfo.channels);
        }

        if (err != MPC_STATUS_OK)
            std::cerr << "An internal error occured in mpc_demux_decode" << std::endl;

        return totalSamplesRead;
    }

    ~MusepackInternal()
    {
        if (mpcDemux) mpc_demux_exit(mpcDemux);
    }
    
    
private:

    mpc_streaminfo streamInfo;
    mpc_reader_t reader;
    mpc_decoder decoder;
    mpc_demux * mpcDemux;
    
    std::shared_ptr<mpc_reader_state> decoderMemory;
    
    NO_MOVE(MusepackInternal);
    
    AudioData * d;
};


//////////////////////
// Public Interface //
//////////////////////

void MusepackDecoder::LoadFromPath(AudioData * data, const std::string & path)
{
    auto fileBuffer = nqr::ReadFile(path);
    MusepackInternal decoder(data, fileBuffer.buffer);
}

void MusepackDecoder::LoadFromBuffer(AudioData * data, const std::vector<uint8_t> & memory)
{
    MusepackInternal decoder(data, memory);
}

std::vector<std::string> MusepackDecoder::GetSupportedFileExtensions()
{
    return {"mpc", "mpp"};
}
