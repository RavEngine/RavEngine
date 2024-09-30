#if !RVE_SERVER
#include "AudioSpace.hpp"
#include "AudioPlayer.hpp"
#include <base/misc_math.h>
#include <common/room_effects_utils.h>
#include "Profile.hpp"

namespace RavEngine {
	RavEngine::BoxReverbationAudioSpace::BoxReverbationAudioSpace(Entity owner) : roomData(std::make_unique<RoomData>()), ComponentWithOwner(owner)
	{

	}

	BoxReverbationAudioSpace::RoomData::RoomData() : audioEngine(vraudio::CreateResonanceAudioApi(AudioPlayer::GetNChannels(), AudioPlayer::GetBufferSize(), AudioPlayer::GetSamplesPerSec())), workingBuffers(AudioPlayer::GetBufferSize(), AudioPlayer::GetNChannels()){}

	BoxReverbationAudioSpace::RoomData::~RoomData() {
		delete audioEngine;
	}

	void BoxReverbationAudioSpace::RoomData::ConsiderAudioSource(const PlanarSampleBufferInlineView& monoSourceData, const vector3& worldPos, const quaternion& worldRot, const matrix4& invRoomTransform, entity_t ownerID, const vector3& roomHalfExts)
	{
		auto roomSpacePos = invRoomTransform * vector4(worldPos, 1);
		// AABB check
		bool IsInRoom = RMath::pointInAABB(roomSpacePos, roomHalfExts);


		auto it = sourceMap.find(ownerID);
		vraudio::ResonanceAudioApi::SourceId src;
		if (it == sourceMap.end()) {
			if (!IsInRoom) {
				// not in the room, and wasn't before. bail
				return;
			}

			src = audioEngine->CreateSoundObjectSource(vraudio::RenderingMode::kBinauralLowQuality);
			sourceMap.emplace(ownerID, src);
		}
		else {
			src = it->second;
			if (!IsInRoom) {
				// not in the room, but was before. Destroy source and bail
				audioEngine->DestroySource(src);
				
				sourceMap.erase(ownerID);
				return;
			}
		}

		auto roomSpaceRot = glm::quat_cast(invRoomTransform * glm::toMat4(worldRot));
		
		//create Eigen structures to calculate attenuation
		vraudio::WorldRotation eroomrot(roomSpaceRot.w, roomSpaceRot.x, roomSpaceRot.y, roomSpaceRot.z);
		vraudio::WorldPosition eroompos(roomSpacePos.x, roomSpacePos.y, roomSpacePos.z);
		vraudio::WorldPosition eroomdim(roomHalfExts.x, roomHalfExts.y, roomHalfExts.z);
		auto gain = vraudio::ComputeRoomEffectsGain(eroompos, {0,0,0}, eroomrot, eroomdim);

		audioEngine->SetInterleavedBuffer(src, monoSourceData.data(), 1, monoSourceData.sizeOneChannel());   // they copy the contents of temp into their own buffer
		audioEngine->SetSourceVolume(src, 1);   // the AudioAsset already applied the volume
		audioEngine->SetSourcePosition(src, roomSpacePos.x, roomSpacePos.y, roomSpacePos.z);
		audioEngine->SetSourceRotation(src, roomSpaceRot.x, roomSpaceRot.y, roomSpaceRot.z, roomSpaceRot.w);
		audioEngine->SetSourceRoomEffectsGain(src, gain);

	}

	void BoxReverbationAudioSpace::RoomData::RenderSpace(PlanarSampleBufferInlineView& outBuffer, PlanarSampleBufferInlineView& scratchBuffer, const vector3& listenerPosRoomSpace, const quaternion& listenerRotRoomSpace, const vector3& roomHalfExts, const BoxReverbationAudioSpace::RoomProperties& roomProperties)
	{
		RVE_PROFILE_FN;
		audioEngine->SetHeadPosition(listenerPosRoomSpace.x, listenerPosRoomSpace.y, listenerPosRoomSpace.z);
		audioEngine->SetHeadRotation(listenerRotRoomSpace.x, listenerRotRoomSpace.y, listenerRotRoomSpace.z, listenerRotRoomSpace.w);

		if (wallsNeedUpdate) {
			vraudio::RoomProperties data;
			data.dimensions[0] = roomHalfExts.x * 2;
			data.dimensions[1] = roomHalfExts.y * 2;
			data.dimensions[2] = roomHalfExts.z * 2;

			data.reflection_scalar = roomProperties.reflection_scalar;
			data.reverb_gain = roomProperties.reverb_gain;
			data.reverb_time = roomProperties.reverb_time;
			data.reverb_brightness = roomProperties.reverb_brightness;

			//add material data
			std::memcpy(data.material_names, wallMaterials.data(), sizeof(data.material_names));

			auto ref_data = vraudio::ComputeReflectionProperties(data);
			auto rev_data = vraudio::ComputeReverbProperties(data);

			//write changes
			audioEngine->SetReflectionProperties(ref_data);
			audioEngine->SetReverbProperties(rev_data);

			wallsNeedUpdate = false;
		}

		auto nchannels = AudioPlayer::GetNChannels();

		// convert to an array of pointers for Resonance
		stackarray(allchannelptrs, float*, nchannels);
		for (uint8_t i = 0; i < nchannels; i++) {
			allchannelptrs[i] = outBuffer[i].data();
		}

		audioEngine->FillPlanarOutputBuffer(nchannels, outBuffer.sizeOneChannel(), allchannelptrs);
		AudioGraphComposed::Render(outBuffer, scratchBuffer, nchannels); // process graph
	}

	void BoxReverbationAudioSpace::RoomData::DeleteAudioDataForEntity(entity_t id) {
		auto it = sourceMap.find(id);
		if (it != sourceMap.end()) {
			audioEngine->DestroySource(it->second);
			sourceMap.erase(it);
		}
	}
}

#endif
