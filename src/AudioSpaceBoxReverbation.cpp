#if !RVE_SERVER
#include "AudioSpace.hpp"
#include <api/resonance_audio_api.h>
#include "AudioPlayer.hpp"

namespace RavEngine {
	RavEngine::BoxReverbationAudioSpace::BoxReverbationAudioSpace(entity_t owner) : roomData(std::make_unique<RoomData>()), ComponentWithOwner(owner)
	{

	}

	BoxReverbationAudioSpace::RoomData::RoomData() : audioEngine(vraudio::CreateResonanceAudioApi(AudioPlayer::GetNChannels(), AudioPlayer::GetBufferSize(), AudioPlayer::GetSamplesPerSec())) {}

	BoxReverbationAudioSpace::RoomData::~RoomData() {
		delete audioEngine;
	}
	void BoxReverbationAudioSpace::RoomData::RenderSpace(PlanarSampleBufferInlineView& outBuffer, PlanarSampleBufferInlineView& scratchBuffer, const vector3& listenerPosRoomSpace, const quaternion& listenerRotRoomSpace)
	{
		audioEngine->SetHeadPosition(listenerPosRoomSpace.x, listenerPosRoomSpace.y, listenerPosRoomSpace.z);
		audioEngine->SetHeadRotation(listenerRotRoomSpace.x, listenerRotRoomSpace.y, listenerRotRoomSpace.z, listenerRotRoomSpace.w);

		auto nchannels = AudioPlayer::GetNChannels();

		// convert to an array of pointers for Resonance
		stackarray(allchannelptrs, float*, nchannels);
		for (uint8_t i = 0; i < nchannels; i++) {
			allchannelptrs[i] = outBuffer[i].data();
		}

		audioEngine->FillPlanarOutputBuffer(nchannels, outBuffer.sizeOneChannel(), allchannelptrs);
		AudioGraphComposed::Render(outBuffer, scratchBuffer, nchannels); // process graph
	}
}

#endif
