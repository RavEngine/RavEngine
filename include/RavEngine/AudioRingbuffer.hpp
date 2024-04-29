#pragma once
#include "AudioRenderBuffer.hpp"

namespace RavEngine {

	class AudioRingbuffer {
		SingleAudioRenderBufferNoScratch renderBuffer;
		uint32_t playhead = 0;
	public:
		AudioRingbuffer(uint8_t nchannels);
		void WriteSampleData(PlanarSampleBufferInlineView data);
	};

}