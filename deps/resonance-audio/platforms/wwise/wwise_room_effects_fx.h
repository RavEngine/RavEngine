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

#ifndef RESONANCE_AUDIO_PLATFORM_WWISE_WWISE_ROOM_EFFECTS_FX_H_
#define RESONANCE_AUDIO_PLATFORM_WWISE_WWISE_ROOM_EFFECTS_FX_H_

#include "AK/SoundEngine/Common/AkTypes.h"
#include "AK/SoundEngine/Common/IAkMixerPlugin.h"
#include "AK/SoundEngine/Common/IAkPlugin.h"
#include "api/resonance_audio_api.h"

namespace vraudio {
namespace wwise {

class WwiseRoomEffectsFx : public AK::IAkMixerEffectPlugin {
 public:
  WwiseRoomEffectsFx();

  ~WwiseRoomEffectsFx() override = default;

  // Initializes the effect plugin.
  AKRESULT Init(AK::IAkPluginMemAlloc* memory_allocator,
                AK::IAkMixerPluginContext* plugin_context,
                AK::IAkPluginParam* plugin_params,
                AkAudioFormat& audio_format) override;

  // Terminates the effect plugin.
  AKRESULT Term(AK::IAkPluginMemAlloc* memory_allocator) override;

  // Resets the effect plugin.
  AKRESULT Reset() override;

  // Returns the plugin info.
  AKRESULT GetPluginInfo(AkPluginInfo& plugin_info) override;

  // Callback function for when an input gets connected to the mixer.
  void OnInputConnected(AK::IAkMixerInputContext* input_context) override;

  // Callback function for when an input gets disconnected from the mixer.
  void OnInputDisconnected(AK::IAkMixerInputContext* input_context) override;

  // Processes the next buffer of given input to fill the output mix buffer.
  void ConsumeInput(AK::IAkMixerInputContext* input_context, AkRamp base_volume,
                    AkRamp emitter_listener_volume, AkAudioBuffer* input,
                    AkAudioBuffer* mix) override;

  // Processes the accumulated output mix buffer *before* effects processing.
  void OnMixDone(AkAudioBuffer* mix) override;

  // Processes the accumulated output mix buffer *after* effects processing.
  void OnEffectsProcessed(AkAudioBuffer* mix) override;

  // Processes the accumulated output mix buffer *after* all processing
  // including metering has been completed.
  void OnFrameEnd(AkAudioBuffer* mix, AK::IAkMetering* metering) override;

 private:
  // Memory allocator interface.
  AK::IAkPluginMemAlloc* memory_allocator_;

  // Room effects mixer plugin context.
  AK::IAkMixerPluginContext* plugin_context_;

  // Current world transform of the listener.
  AkTransform listener_transform_;

  // Default room properties, which effectively disable the room effects.
  const ReflectionProperties null_reflection_properties_;
  const ReverbProperties null_reverb_properties_;

  // Plugin instance id.
  int instance_id_;
};

}  // namespace wwise
}  // namespace vraudio

#endif  // RESONANCE_AUDIO_PLATFORM_WWISE_WWISE_ROOM_EFFECTS_FX_H_
