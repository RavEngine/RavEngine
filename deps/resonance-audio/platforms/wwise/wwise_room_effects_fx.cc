/*
Copyright 2018 Google Inc. All Rights Reserved.

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

    http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS-IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.
*/

#include "platforms/wwise/wwise_room_effects_fx.h"

#include <vector>

#include "Eigen/Dense"
#include "AK/Tools/Common/AkAssert.h"
#include "platforms/common/room_effects_utils.h"
#include "platforms/common/utils.h"
#include "platforms/wwise/wwise_common.h"
#include "platforms/wwise/wwise_fx_null_params.h"
#include "platforms/wwise/wwise_room_effects_fx_attachment_params.h"

namespace vraudio {
namespace wwise {

namespace {

// |ResonanceAudioSystem| to communicate with the internal API.

static ResonanceAudioSystem* resonance_audio = nullptr;

// Additional data that needs to be stored per each input source.
struct SourceData {
  // Source id.
  int id;
};

// Returns the transformation matrix from the given Wwise vectors.
//
// @param position Wwise position vector.
// @param forward Wwise forward vector.
// @param up Wwise up vector.
// @return A 4x4 transformation matrix.
Eigen::Matrix4f GetTransformMatrixFromWwise(const AkVector& wwise_position,
                                            const AkVector& wwise_forward,
                                            const AkVector& wwise_up) {
  const Eigen::Vector3f position(wwise_position.X, wwise_position.Y,
                                 wwise_position.Z);
  const Eigen::Vector3f forward(wwise_forward.X, wwise_forward.Y,
                                wwise_forward.Z);
  const Eigen::Vector3f up(wwise_up.X, wwise_up.Y, wwise_up.Z);
  return GetTransformMatrix(position, forward, up);
}

// Returns piecewise sum of two vectors.
//
// @param lhs First vector to be summed.
// @param rhs Second vector to be summed.
// @return Sum of two vectors.
AkVector VectorSum(const AkVector& lhs, const AkVector& rhs) {
  return AkVector{lhs.X + rhs.X, lhs.Y + rhs.Y, lhs.Z + rhs.Z};
}

// Wwise host callback method that registers the mixer plugin.
AK::IAkPlugin* CreateResonanceAudioRoomEffectsFX(
    AK::IAkPluginMemAlloc* memory_allocator) {
  AKASSERT(memory_allocator != nullptr);
  return AK_PLUGIN_NEW(memory_allocator, WwiseRoomEffectsFx());
}

// Wwise host callback method that registers the mixer plugin attachment
// parameter node.
AK::IAkPluginParam* CreateResonanceAudioRoomEffectsFXAttachmentParams(
    AK::IAkPluginMemAlloc* memory_allocator) {
  AKASSERT(memory_allocator != nullptr);
  return AK_PLUGIN_NEW(memory_allocator, WwiseRoomEffectsFxAttachmentParams());
}

// Wwise host callback method that registers the mixer plugin parameter node.
AK::IAkPluginParam* CreateResonanceAudioRoomEffectsFXParams(
    AK::IAkPluginMemAlloc* memory_allocator) {
  AKASSERT(memory_allocator != nullptr);
  // The room effects plugin does not expose any parameters.

  return AK_PLUGIN_NEW(memory_allocator, WwiseFxNullParams());
}

// Wwise global registration callback that is called by the sound engine.
void RegisterResonanceAudioRoomEffectsFX(
    AK::IAkGlobalPluginContext* plugin_context,
    AkGlobalCallbackLocation callback_location, void* cookie) {
  if (callback_location == AkGlobalCallbackLocation_Register) {
    if (resonance_audio == nullptr) {
      // Initialize the ResonanceAudio system.
      resonance_audio = new ResonanceAudioSystem();
      const size_t frames_per_buffer =
          static_cast<size_t>(plugin_context->GetMaxBufferLength());
      const int sample_rate = static_cast<int>(plugin_context->GetSampleRate());
      resonance_audio->api.reset(CreateResonanceAudioApi(
          kNumStereoChannels, frames_per_buffer, sample_rate));
    }
    ++resonance_audio->num_instances;
    // Register shutdown callback.
    plugin_context->RegisterGlobalCallback(RegisterResonanceAudioRoomEffectsFX,
                                           AkGlobalCallbackLocation_Term,
                                           nullptr);
  } else if (callback_location == AkGlobalCallbackLocation_Term) {
    AKASSERT(resonance_audio != nullptr);
    --resonance_audio->num_instances;
    if (resonance_audio->num_instances == 0) {
      // Shutdown the ResonanceAudio system.
      delete resonance_audio;
      resonance_audio = nullptr;
    }
  }
}

}  // namespace

// Plugin mechanism. Instantiation methods that must be registered to the plugin
// manager.
AK::PluginRegistration ResonanceAudioRoomEffectsFXRegistration(
    AkPluginTypeMixer, kCompanyId, kRoomEffectsPluginId,
    CreateResonanceAudioRoomEffectsFX, CreateResonanceAudioRoomEffectsFXParams,
    RegisterResonanceAudioRoomEffectsFX, nullptr);
AK::PluginRegistration ResonanceAudioRoomEffectsFXAttachmentRegistration(
    AkPluginTypeEffect, kCompanyId, kRoomEffectsAttachmentPluginId, nullptr,
    CreateResonanceAudioRoomEffectsFXAttachmentParams);

WwiseRoomEffectsFx::WwiseRoomEffectsFx()
    : memory_allocator_(nullptr),
      plugin_context_(nullptr),
      instance_id_(kInvalidInstanceId) {}

AKRESULT WwiseRoomEffectsFx::Init(AK::IAkPluginMemAlloc* memory_allocator,
                                  AK::IAkMixerPluginContext* plugin_context,
                                  AK::IAkPluginParam* plugin_params,
                                  AkAudioFormat& audio_format) {
  if (instance_id_ == kInvalidInstanceId) {
    // Assign a new id for the plugin instance.
    instance_id_ = resonance_audio->instance_id_counter;
    resonance_audio->instance_ids.push_back(instance_id_);
    ++resonance_audio->instance_id_counter;
  }
  memory_allocator_ = memory_allocator;
  plugin_context_ = plugin_context;
  // Set output channel config to stereo.

  audio_format.channelConfig.SetStandard(AK_SPEAKER_SETUP_STEREO);

  return AK_Success;
}

AKRESULT WwiseRoomEffectsFx::Term(AK::IAkPluginMemAlloc* memory_allocator) {
  if (instance_id_ != kInvalidInstanceId) {
    // Remove the plugin instance id.
    resonance_audio->instance_ids.remove(instance_id_);
    instance_id_ = kInvalidInstanceId;
  }
  // Effect must delete itself.
  AK_PLUGIN_DELETE(memory_allocator, this);

  return AK_Success;
}

AKRESULT WwiseRoomEffectsFx::Reset() { return AK_Success; }

AKRESULT WwiseRoomEffectsFx::GetPluginInfo(AkPluginInfo& plugin_info) {
  plugin_info.eType = AkPluginTypeMixer;
  plugin_info.bIsInPlace = false;
  plugin_info.uBuildVersion = AK_WWISESDK_VERSION_COMBINED;
  return AK_Success;
}

void WwiseRoomEffectsFx::OnInputConnected(
    AK::IAkMixerInputContext* input_context) {
  if (input_context->GetUserData() == nullptr) {
    SourceData* source_data = AK_PLUGIN_NEW(memory_allocator_, SourceData());
    source_data->id = resonance_audio->api->CreateSoundObjectSource(
        RenderingMode::kRoomEffectsOnly);
    input_context->SetUserData(source_data);
  }
}

void WwiseRoomEffectsFx::OnInputDisconnected(
    AK::IAkMixerInputContext* input_context) {
  SourceData* source_data =
      reinterpret_cast<SourceData*>(input_context->GetUserData());
  if (source_data != nullptr) {
    resonance_audio->api->DestroySource(source_data->id);
    AK_PLUGIN_DELETE(memory_allocator_, source_data);
  }
}

void WwiseRoomEffectsFx::ConsumeInput(AK::IAkMixerInputContext* input_context,
                                      AkRamp base_volume,
                                      AkRamp emitter_listener_volume,
                                      AkAudioBuffer* input,
                                      AkAudioBuffer* mix) {
  AKASSERT(!resonance_audio->instance_ids.empty());
  if (instance_id_ != resonance_audio->instance_ids.back()) {
    // Skip processing if there is a newer instance running.
    input->uValidFrames = 0;
    return;
  }
  if (input->uValidFrames == 0) {
    // No input buffer to consume, skip processing.
    return;
  }

  const size_t input_num_channels = static_cast<size_t>(input->NumChannels());
  AKASSERT(input_num_channels <= kNumStereoChannels);

  // Pad the input buffer with zeros if necessary.
  const AkUInt16 num_frames = input->MaxFrames();
  input->ZeroPadToMaxFrames();

  // Retrieve input source data.
  SourceData* source_data =
      reinterpret_cast<SourceData*>(input_context->GetUserData());
  if (source_data == nullptr) {
    // Return if the sound object was not initialized properly.
    return;
  }
  const int source_id = source_data->id;
  AKASSERT(source_id != ResonanceAudioApi::kInvalidSourceId);

  // Set sound object properties.

  if (input_context->GetNum3DPositions() > 0) {
    // Set input gain.
    resonance_audio->api->SetSourceVolume(source_id, base_volume.fNext);
    // Set position.
    AkEmitterListenerPair emitter_listener;
    input_context->Get3DPosition(0, emitter_listener);
    const AkVector source_position = VectorSum(
        listener_transform_.Position(), emitter_listener.emitter.Position());
    // Flip Z-axis to convert from left-handed to right-handed.
    resonance_audio->api->SetSourcePosition(
        source_id, source_position.X, source_position.Y, -source_position.Z);
    // Set distance attenuation.
    AkReal32 max_distance;
    input_context->GetMaxAttenuationDistance(max_distance);
    resonance_audio->api->SetSourceDistanceModel(
        source_id, DistanceRolloffModel::kNone, 0.0f, max_distance);
    AkReal32 distance_attenuation = emitter_listener.GetGainForConnectionType(
        input_context->GetConnectionType());
    resonance_audio->api->SetSourceDistanceAttenuation(source_id,
                                                       distance_attenuation);
    // Set room effects gain.
    float room_effects_gain = 0.0f;
    const auto params = reinterpret_cast<WwiseRoomEffectsFxAttachmentParams*>(
        input_context->GetInputParam());
    if (params != nullptr && !params->bypass()) {
      void* room_data = nullptr;
      AkUInt32 room_data_size = 0;
      plugin_context_->GetPluginCustomGameData(room_data, room_data_size);
      if (room_data != nullptr) {
        AKASSERT(room_data_size == sizeof(RoomProperties));
        auto* room = reinterpret_cast<RoomProperties*>(room_data);
        room_effects_gain = ComputeRoomEffectsGain(
            WorldPosition(source_position.X, source_position.Y,
                          source_position.Z),
            WorldPosition(room->position), WorldRotation(room->rotation),
            WorldPosition(room->dimensions));
      }
    }
    resonance_audio->api->SetSourceRoomEffectsGain(source_id,
                                                   room_effects_gain);

    // Update listener transform where available.
    AkListener listener;
    if (input_context->GetVoiceInfo()->GetListenerData(
            emitter_listener.ListenerID(), listener)) {
      listener_transform_ = listener.position;
    }
  }

  // Set the input buffer.
  const auto input_channel_ptrs =
      GetChannelPointerArrayFromAkAudioBuffer<const float*>(input);
  resonance_audio->api->SetPlanarBuffer(source_id, input_channel_ptrs.data(),
                                        input_num_channels, num_frames);
  input->uValidFrames = 0;
  mix->uValidFrames = num_frames;
}

void WwiseRoomEffectsFx::OnMixDone(AkAudioBuffer* mix) {
  AKASSERT(!resonance_audio->instance_ids.empty());
  if (instance_id_ != resonance_audio->instance_ids.back()) {
    // Skip processing if there is a newer instance running.
    mix->uValidFrames = 0;
    mix->eState = AK_NoMoreData;
    return;
  }

  const size_t num_channels = static_cast<size_t>(mix->NumChannels());
  AKASSERT(num_channels == kNumStereoChannels);
  const AkUInt16 num_frames = mix->MaxFrames();

  // Calculate the listener transform matrix.
  Eigen::Matrix4f transform_matrix = GetTransformMatrixFromWwise(
      listener_transform_.Position(), listener_transform_.OrientationFront(),
      listener_transform_.OrientationTop());
  // Switch to right-handed coordinate system (Wwise is left-handed).
  FlipZAxis(&transform_matrix);
  // Set listener transform.
  const Eigen::Vector3f position = GetPosition(transform_matrix);
  const Eigen::Quaternionf rotation = GetQuaternion(transform_matrix);
  resonance_audio->api->SetHeadPosition(position.x(), position.y(),
                                        position.z());
  resonance_audio->api->SetHeadRotation(rotation.x(), rotation.y(),
                                        rotation.z(), rotation.w());

  // Set room properties.
  void* room_data = nullptr;
  AkUInt32 room_data_size = 0;
  plugin_context_->GetPluginCustomGameData(room_data, room_data_size);
  if (room_data == nullptr) {
    resonance_audio->api->SetReflectionProperties(null_reflection_properties_);
    resonance_audio->api->SetReverbProperties(null_reverb_properties_);
  } else {
    AKASSERT(room_data_size == sizeof(RoomProperties));
    auto* room = reinterpret_cast<RoomProperties*>(room_data);
    resonance_audio->api->SetReflectionProperties(
        ComputeReflectionProperties(*room));
    resonance_audio->api->SetReverbProperties(ComputeReverbProperties(*room));
  }

  // Fill the output buffer.
  const auto mix_channel_ptrs =
      GetChannelPointerArrayFromAkAudioBuffer<float*>(mix);
  if (resonance_audio->api->FillPlanarOutputBuffer(num_channels, num_frames,
                                                   mix_channel_ptrs.data())) {
    // Update the output state to continue processing.
    mix->uValidFrames = num_frames;
    mix->eState = AK_DataReady;
  } else {
    // No valid output was rendered, the effect is done.
    mix->uValidFrames = 0;
    mix->eState = AK_NoMoreData;
  }
}

void WwiseRoomEffectsFx::OnEffectsProcessed(AkAudioBuffer* mix) {}

void WwiseRoomEffectsFx::OnFrameEnd(AkAudioBuffer* mix,
                                    AK::IAkMetering* metering) {}

}  // namespace wwise
}  // namespace vraudio
