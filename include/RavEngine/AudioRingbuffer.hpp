#pragma once
#include "AudioRenderBuffer.hpp"

namespace RavEngine {

	class AudioRingbuffer {
		SingleAudioRenderBufferNoScratch renderBuffer;
		uint32_t playhead = 0;
		const uint8_t nchannels;
	public:
		AudioRingbuffer(uint8_t nchannels);
		void WriteSampleData(PlanarSampleBufferInlineView data);
		uint32_t GetTotalSize() const;
		auto GetNChannels() const {
			return nchannels;
		}
		void UnwindSampleData(PlanarSampleBufferInlineView data) const;
	};

}