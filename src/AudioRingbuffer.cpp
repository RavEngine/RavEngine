#if !RVE_SERVER
#include "AudioRingbuffer.hpp"
#include "App.hpp"
#include "AudioPlayer.hpp"

constexpr static int nsec = 10;

namespace RavEngine {
	// it's always a multiple of the buffer size
	// so we don't have to do any math around wrapping mid-buffer
	AudioRingbuffer::AudioRingbuffer(uint8_t nchannels) : nchannels(nchannels), renderBuffer{AudioPlayer::GetSamplesPerSec() * nsec, nchannels }
	{

	}
	void AudioRingbuffer::WriteSampleData(PlanarSampleBufferInlineView data)
	{
		auto outView = renderBuffer.GetDataBufferView();

		for (int i = 0; i < data.GetNChannels(); i++) {
			memcpy(outView[i].data() + playhead, data[i].data(), data.bytesOneChannel());
		}
		playhead += data.sizeOneChannel();
		playhead %= outView.sizeOneChannel();
	}
	uint32_t AudioRingbuffer::GetTotalSize() const
	{
		return renderBuffer.GetDataBufferView().size();
	}
	void AudioRingbuffer::UnwindSampleData(PlanarSampleBufferInlineView outView) const
	{
		auto data = renderBuffer.GetDataBufferView();
		for (int i = 0; i < data.GetNChannels(); i++) {
			// copy from the current playhead to the end of the buffer
			memcpy(outView[i].data(), data[i].data() + playhead, data.bytesOneChannel() - playhead);

			// copy from the beginning of the data to the playhead
			memcpy(outView[i].data() + playhead, data[i].data(), playhead);
		}
	}
}
#endif