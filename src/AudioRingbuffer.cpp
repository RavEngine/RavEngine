#if !RVE_SERVER
#include "AudioRingbuffer.hpp"
#include "App.hpp"
#include "AudioPlayer.hpp"
#include "dr_wav.h"
#include "Debug.hpp"


namespace RavEngine {

	static uint32_t GetSizeForNSec(uint32_t nsecs) {
		return ceil((AudioPlayer::GetSamplesPerSec() * nsecs) / float(AudioPlayer::GetBufferSize())) * AudioPlayer::GetBufferSize();
	}

	// it's always a multiple of the buffer size
	// so we don't have to do any math around wrapping mid-buffer
	AudioRingbuffer::AudioRingbuffer(uint8_t nchannels) : nchannels(nchannels), renderBuffer{ GetSizeForNSec(5u), nchannels}
	{

	}
	void AudioRingbuffer::WriteSampleData(PlanarSampleBufferInlineView data)
	{
		auto outView = renderBuffer.GetWritableDataBufferView();

		for (int i = 0; i < data.GetNChannels(); i++) {
			memcpy(outView[i].data() + playhead, data[i].data(), data.bytesOneChannel());
		}
		playhead += data.sizeOneChannel();
        
        // wrap playhead around
        auto view = getDataView();
		playhead %= view.sizeOneChannel();
	}
	uint32_t AudioRingbuffer::GetTotalSize() const
	{
		return renderBuffer.GetReadonlyDataBufferView().size();
	}
	void AudioRingbuffer::UnwindSampleData(PlanarSampleBufferInlineView outView) const
	{
		auto data = renderBuffer.GetReadonlyDataBufferView();
		for (int i = 0; i < data.GetNChannels(); i++) {
			// copy from the current playhead to the end of the buffer
			memcpy(outView[i].data(), data[i].data() + playhead, (data.sizeOneChannel() - playhead) * sizeof(float));

			// copy from the beginning of the data to the playhead
			memcpy(outView[i].data() + playhead, data[i].data(), playhead * sizeof(float));
		}
	}

    void AudioRingbuffer::DumpToFileNoProcessing(const Filesystem::Path& path) const{
        //std::vector<float> audioData(debugBuffer.GetTotalSize(), 0.0f);
        //PlanarSampleBufferInlineView sourceView{ audioData.data(),audioData.size(), audioData.size() / debugBuffer.GetNChannels() };
        //UnwindSampleData(sourceView);

        auto sourceView = renderBuffer.GetReadonlyDataBufferView();
        
        const auto nchannels = GetNChannels();

        drwav outputFile;
        drwav_data_format outputFormat{};
        outputFormat.container = drwav_container_riff;
        outputFormat.format = DR_WAVE_FORMAT_IEEE_FLOAT;
        outputFormat.channels = nchannels;
        outputFormat.sampleRate = AudioPlayer::GetSamplesPerSec();
        outputFormat.bitsPerSample = 32;


    #if !defined(_WIN32)
        drwav_bool32 outputFileOk = drwav_init_file_write(&outputFile, path.c_str(), &outputFormat, nullptr);
    #else
        drwav_bool32 outputFileOk = drwav_init_file_write_w(&outputFile, path.c_str(), &outputFormat, nullptr);
    #endif
        std::vector<float> interleavedPcm(GetTotalSize(),0);
        // interleave audio
        for (int i = 0; i < interleavedPcm.size(); i++) {
            //mix with existing
            // also perform planar-to-interleaved conversion
            interleavedPcm[i] = sourceView[i % nchannels][i / nchannels];
        }

        // it's the number of samples irrespective of channel count
        drwav_write_pcm_frames(&outputFile, interleavedPcm.size() / nchannels, interleavedPcm.data());

        drwav_uninit(&outputFile);
        
        Debug::Log("Wrote debug audio {} for SimpleAudioSpace",path.string());
    }
}
#endif
