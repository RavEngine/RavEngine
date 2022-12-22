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

// http://lists.xiph.org/pipermail/flac-dev/2012-March/003276.html
#define FLAC__NO_DLL

#include "FLAC/all.h"
#include "FLAC/stream_decoder.h"

#include <cstring>

using namespace nqr;

// FLAC is a big-endian format. All values are unsigned.
class FlacDecoderInternal
{
  
public:

    FlacDecoderInternal(AudioData * d, const std::string & filepath) : d(d)
    {
        decoderInternal = FLAC__stream_decoder_new();
        
        FLAC__stream_decoder_set_metadata_respond(decoderInternal, FLAC__METADATA_TYPE_STREAMINFO);
        
        //@todo: check if OGG flac
        bool initialized = FLAC__stream_decoder_init_file(decoderInternal,
                                                          filepath.c_str(),
                                                          s_writeCallback,
                                                          s_metadataCallback,
                                                          s_errorCallback,
                                                          this) == FLAC__STREAM_DECODER_INIT_STATUS_OK;
        
        FLAC__stream_decoder_set_md5_checking(decoderInternal, true);

        if (initialized)
        {
            // Find the size and allocate memory
            FLAC__stream_decoder_process_until_end_of_metadata(decoderInternal);
            
            // Read memory out into our temporary internalBuffer
            FLAC__stream_decoder_process_until_end_of_stream(decoderInternal);

            // Presently unneeded, but useful for reference
            // FLAC__ChannelAssignment channelAssignment = FLAC__stream_decoder_get_channel_assignment(decoderInternal);
            
            // Fill out remaining user data
            d->lengthSeconds = (float) numSamples / (float) d->sampleRate;
            
            auto totalSamples = numSamples * d->channelCount;

            // Next, process internal buffer into the user-visible samples array
            ConvertToFloat32(d->samples.data(), internalBuffer.data(), totalSamples, d->sourceFormat);
        }
        else throw std::runtime_error("Unable to initialize FLAC decoder");
    }

    FlacDecoderInternal(AudioData * d, const std::vector<uint8_t> & memory) : d(d), data(std::move(memory)), dataPos(0)
    {
        decoderInternal = FLAC__stream_decoder_new();
        
        FLAC__stream_decoder_set_metadata_respond(decoderInternal, FLAC__METADATA_TYPE_STREAMINFO);
        
        bool initialized = FLAC__stream_decoder_init_stream(
          decoderInternal,
          read_callback,
          seek_callback,
          tell_callback,
          length_callback,
          eof_callback,
          s_writeCallback,
          s_metadataCallback,
          s_errorCallback,
          this
        ) == FLAC__STREAM_DECODER_INIT_STATUS_OK;
        
        FLAC__stream_decoder_set_md5_checking(decoderInternal, true);
        
        if (initialized)
        {
            // Find the size and allocate memory
            FLAC__stream_decoder_process_until_end_of_metadata(decoderInternal);
            
            // Read memory out into our temporary internalBuffer
            FLAC__stream_decoder_process_until_end_of_stream(decoderInternal);

            // Presently unneeded, but useful for reference
            // FLAC__ChannelAssignment channelAssignment = FLAC__stream_decoder_get_channel_assignment(decoderInternal);
            
            // Fill out remaining user data
            d->lengthSeconds = (float) numSamples / (float) d->sampleRate;
            
            auto totalSamples = numSamples * d->channelCount;

            // Next, process internal buffer into the user-visible samples array
            ConvertToFloat32(d->samples.data(), internalBuffer.data(), totalSamples, d->sourceFormat);
        }
        else throw std::runtime_error("Unable to initialize FLAC decoder");
    }
    
    ~FlacDecoderInternal()
    {
        if (decoderInternal)
        {
            FLAC__stream_decoder_finish(decoderInternal);
            FLAC__stream_decoder_delete(decoderInternal);
        }
    }
    
    void processMetadata(const FLAC__StreamMetadata_StreamInfo & info)
    {
        // Currently the reference encoder and decoders only support up to 24 bits per sample.
        d->sampleRate = info.sample_rate;
        d->channelCount = info.channels; // Assert 1 to 8
        d->sourceFormat = MakeFormatForBits(info.bits_per_sample, false, true);
        d->frameSize = info.channels * info.bits_per_sample;
        
        const size_t bytesPerSample = GetFormatBitsPerSample(d->sourceFormat) / 8;

        numSamples = (size_t) info.total_samples;
        
        internalBuffer.resize(numSamples * info.channels * bytesPerSample); // as array of bytes
        d->samples.resize(numSamples * info.channels); // as audio samples in float32
    }

    ///////////////////////
    // libflac callbacks //
    ///////////////////////
    
    static FLAC__StreamDecoderWriteStatus s_writeCallback(const FLAC__StreamDecoder *, const FLAC__Frame* frame, const FLAC__int32 * const buffer[], void * userPtr)
    {
        FlacDecoderInternal * decoder = reinterpret_cast<FlacDecoderInternal *>(userPtr);
        const size_t bytesPerSample = GetFormatBitsPerSample(decoder->d->sourceFormat) / 8;
        auto dataPtr = decoder->internalBuffer.data();
        
        for (uint32_t i = 0;  i < frame->header.blocksize; i++)
        {
            for (int j = 0; j < decoder->d->channelCount; j++)
            {
                std::memcpy(dataPtr + decoder->bufferPosition, &buffer[j][i], bytesPerSample);
                decoder->bufferPosition += bytesPerSample;
            }
        }
        
        return FLAC__STREAM_DECODER_WRITE_STATUS_CONTINUE;
    }
    
    static void s_metadataCallback (const FLAC__StreamDecoder *, const FLAC__StreamMetadata * metadata, void * userPtr)
    {
        static_cast<FlacDecoderInternal*>(userPtr)->processMetadata(metadata->data.stream_info);
    }
    
    static void s_errorCallback (const FLAC__StreamDecoder *, FLAC__StreamDecoderErrorStatus status, void *)
    {
        throw std::runtime_error("FLAC decode exception " + std::string(FLAC__StreamDecoderErrorStatusString[status]));
    }

    static FLAC__StreamDecoderReadStatus read_callback(const FLAC__StreamDecoder *decoder, FLAC__byte buffer[], size_t *bytes, void *client_data) 
    {
        FlacDecoderInternal *decoderInternal = (FlacDecoderInternal *)client_data;
        size_t readLength = std::min<size_t>(*bytes, decoderInternal->data.size() - decoderInternal->dataPos);

        if (readLength > 0) 
        {
            std::memcpy(buffer, decoderInternal->data.data(), readLength);
            decoderInternal->dataPos += readLength;
            *bytes = readLength;
            if (decoderInternal->dataPos < decoderInternal->data.size()) return FLAC__STREAM_DECODER_READ_STATUS_CONTINUE;
            else return FLAC__STREAM_DECODER_READ_STATUS_END_OF_STREAM;
        }
        else return FLAC__STREAM_DECODER_READ_STATUS_END_OF_STREAM;
    }

    static FLAC__StreamDecoderSeekStatus seek_callback(const FLAC__StreamDecoder *decoder, FLAC__uint64 absolute_byte_offset, void *client_data) 
    {
        FlacDecoderInternal *decoderInternal = (FlacDecoderInternal *)client_data;
        size_t newPos = std::min<size_t>(absolute_byte_offset, decoderInternal->data.size() - decoderInternal->dataPos);
        decoderInternal->dataPos = newPos;
        return FLAC__STREAM_DECODER_SEEK_STATUS_OK;
    }

    static FLAC__StreamDecoderTellStatus tell_callback(const FLAC__StreamDecoder *decoder, FLAC__uint64 *absolute_byte_offset, void *client_data) 
    {
        FlacDecoderInternal *decoderInternal = (FlacDecoderInternal *)client_data;
        *absolute_byte_offset = decoderInternal->dataPos;
        return FLAC__STREAM_DECODER_TELL_STATUS_OK;
    }

    static FLAC__StreamDecoderLengthStatus length_callback(const FLAC__StreamDecoder *decoder, FLAC__uint64 *stream_length, void *client_data) 
    {
        FlacDecoderInternal *decoderInternal = (FlacDecoderInternal *)client_data;
        *stream_length = decoderInternal->data.size();
        return FLAC__STREAM_DECODER_LENGTH_STATUS_OK;
    }

    static FLAC__bool eof_callback(const FLAC__StreamDecoder *decoder, void *client_data) 
    {
        FlacDecoderInternal *decoderInternal = (FlacDecoderInternal *)client_data;
        return decoderInternal->dataPos == decoderInternal->data.size();
    }
    
private:
    
    NO_COPY(FlacDecoderInternal);
    
    FLAC__StreamDecoder * decoderInternal;
    std::vector<uint8_t> data;
    size_t dataPos;
    size_t bufferPosition = 0;
    size_t numSamples = 0;
    
    AudioData * d;
    
    std::vector<uint8_t> internalBuffer;
};

//////////////////////
// Public Interface //
//////////////////////

void FlacDecoder::LoadFromPath(AudioData * data, const std::string & path)
{
    FlacDecoderInternal decoder(data, path);
}

void FlacDecoder::LoadFromBuffer(AudioData * data, const std::vector<uint8_t> & memory)
{
    FlacDecoderInternal decoder(data, memory);
}

std::vector<std::string> FlacDecoder::GetSupportedFileExtensions()
{
    return {"flac"};
}
