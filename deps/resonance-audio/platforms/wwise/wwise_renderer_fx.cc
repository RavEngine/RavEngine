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

#include "platforms/wwise/wwise_renderer_fx.h"

#include "AK/Tools/Common/AkAssert.h"
#include "base/constants_and_types.h"
#include "platforms/wwise/wwise_common.h"
#include "platforms/wwise/wwise_fx_null_params.h"

namespace vraudio {
namespace wwise {

namespace {

// |ResonanceAudioSystem| to communicate with the internal API.

static ResonanceAudioSystem* resonance_audio = nullptr;

// Wwise host callback method that registers the effect plugin.
AK::IAkPlugin* CreateResonanceAudioRendererFX(
    AK::IAkPluginMemAlloc* memory_allocator) {
  AKASSERT(memory_allocator != nullptr);
  return AK_PLUGIN_NEW(memory_allocator, WwiseRendererFx());
}

// Wwise host callback method that registers the effect plugin parameter node.
AK::IAkPluginParam* CreateResonanceAudioRendererFXParams(
    AK::IAkPluginMemAlloc* memory_allocator) {
  AKASSERT(memory_allocator != nullptr);
  // The binaural renderer plugin does not expose any parameters.
  return AK_PLUGIN_NEW(memory_allocator, WwiseFxNullParams());
}

// Wwise global registration callback that is called by the sound engine.
void RegisterResonanceAudioRendererFX(
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
    plugin_context->RegisterGlobalCallback(RegisterResonanceAudioRendererFX,
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

// Plugin mechanism. Instantiation method that must be registered to the plugin
// manager.
AK::PluginRegistration ResonanceAudioRendererFXRegistration(
    AkPluginTypeEffect, kCompanyId, kRendererPluginId,
    CreateResonanceAudioRendererFX, CreateResonanceAudioRendererFXParams,
    RegisterResonanceAudioRendererFX, nullptr);

WwiseRendererFx::WwiseRendererFx()
    : num_ambisonic_channels_(0),
      ambisonic_source_id_(ResonanceAudioApi::kInvalidSourceId),
      instance_id_(kInvalidInstanceId) {}

AKRESULT WwiseRendererFx::Init(AK::IAkPluginMemAlloc* memory_allocator,
                               AK::IAkEffectPluginContext* plugin_context,
                               AK::IAkPluginParam* plugin_params,
                               AkAudioFormat& audio_format) {
  if (instance_id_ == kInvalidInstanceId) {
    // Assign a new id for the plugin instance.
    instance_id_ = resonance_audio->instance_id_counter;
    resonance_audio->instance_ids.push_back(instance_id_);
    ++resonance_audio->instance_id_counter;
  }
  // Get input number of ambisonic channels.
  num_ambisonic_channels_ = static_cast<size_t>(audio_format.GetNumChannels());
  // Set output channel config to stereo (i.e., binaural).
  audio_format.channelConfig.SetStandard(AK_SPEAKER_SETUP_STEREO);
  // Create the ambisonic source.
  AKASSERT(resonance_audio != nullptr);
  ambisonic_source_id_ =
      resonance_audio->api->CreateAmbisonicSource(num_ambisonic_channels_);

  return AK_Success;
}

AKRESULT WwiseRendererFx::Term(AK::IAkPluginMemAlloc* memory_allocator) {
  AKASSERT(resonance_audio != nullptr);
  if (ambisonic_source_id_ != ResonanceAudioApi::kInvalidSourceId) {
    // Destroy the ambisonic source.
    resonance_audio->api->DestroySource(ambisonic_source_id_);
    ambisonic_source_id_ = ResonanceAudioApi::kInvalidSourceId;
  }
  if (instance_id_ != kInvalidInstanceId) {
    // Remove the plugin instance id.
    resonance_audio->instance_ids.remove(instance_id_);
    instance_id_ = kInvalidInstanceId;
  }
  // Effect must delete itself.
  AK_PLUGIN_DELETE(memory_allocator, this);

  return AK_Success;
}

AKRESULT WwiseRendererFx::Reset() { return AK_Success; }

AKRESULT WwiseRendererFx::GetPluginInfo(AkPluginInfo& plugin_info) {
  plugin_info.eType = AkPluginTypeEffect;
  plugin_info.bIsInPlace = false;
  plugin_info.uBuildVersion = AK_WWISESDK_VERSION_COMBINED;
  return AK_Success;
}

void WwiseRendererFx::Execute(AkAudioBuffer* input, AkUInt32 frame_offset,
                              AkAudioBuffer* output) {
  AKASSERT(!resonance_audio->instance_ids.empty());
  if (instance_id_ != resonance_audio->instance_ids.back()) {
    // Skip processing if there is a newer instance running.
    input->uValidFrames = 0;
    output->uValidFrames = 0;
    output->eState = AK_NoMoreData;
    return;
  }
  if (ambisonic_source_id_ == ResonanceAudioApi::kInvalidSourceId) {
    // Skip processing if the ambisonic source does not exist.
    input->uValidFrames = 0;
    output->uValidFrames = 0;
    output->eState = AK_NoMoreData;
    return;
  }

  const size_t input_num_channels = static_cast<size_t>(input->NumChannels());
  AKASSERT(input_num_channels == num_ambisonic_channels_);
  const size_t output_num_channels = static_cast<size_t>(output->NumChannels());
  AKASSERT(output_num_channels == kNumStereoChannels);
  const AkUInt16 num_frames = input->MaxFrames();

  if (input->uValidFrames > 0) {
    // Pad the input buffer with zeros if necessary.
    input->ZeroPadToMaxFrames();
    // Set the input buffer.
    const auto input_channel_ptrs =
        GetChannelPointerArrayFromAkAudioBuffer<const float*>(input);
    resonance_audio->api->SetPlanarBuffer(ambisonic_source_id_,
                                          input_channel_ptrs.data(),
                                          input_num_channels, num_frames);
    // Flag that the input buffer is fully consumed.
    input->uValidFrames = 0;
  }
  // Fill the output buffer.
  const auto output_channel_ptrs =
      GetChannelPointerArrayFromAkAudioBuffer<float*>(output);
  if (resonance_audio->api->FillPlanarOutputBuffer(
          output_num_channels, num_frames, output_channel_ptrs.data())) {
    // Update the output state to continue processing.
    output->uValidFrames = num_frames;
    output->eState = AK_DataReady;
  } else {
    // No valid output was rendered, the effect is done.
    output->uValidFrames = 0;
    output->eState = AK_NoMoreData;
  }
}

AKRESULT WwiseRendererFx::TimeSkip(AkUInt32& num_frames) {
  return AK_DataReady;
}

}  // namespace wwise
}  // namespace vraudio
