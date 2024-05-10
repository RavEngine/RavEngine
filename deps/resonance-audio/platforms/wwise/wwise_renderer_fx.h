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

#ifndef RESONANCE_AUDIO_PLATFORM_WWISE_WWISE_RENDERER_FX_H_
#define RESONANCE_AUDIO_PLATFORM_WWISE_WWISE_RENDERER_FX_H_

#include "AK/SoundEngine/Common/AkTypes.h"
#include "AK/SoundEngine/Common/IAkPlugin.h"
#include "api/resonance_audio_api.h"

namespace vraudio {
namespace wwise {

// Class that implements an out of place effect plugin for the binaural renderer
// plugin of the Wwise integration.
class WwiseRendererFx : public AK::IAkOutOfPlaceEffectPlugin {
 public:
  WwiseRendererFx();

  ~WwiseRendererFx() override = default;

  // Initializes the effect plugin.
  AKRESULT Init(AK::IAkPluginMemAlloc* memory_allocator,
                AK::IAkEffectPluginContext* plugin_context,
                AK::IAkPluginParam* plugin_params,
                AkAudioFormat& audio_format) override;

  // Terminates the effect plugin.
  AKRESULT Term(AK::IAkPluginMemAlloc* memory_allocator) override;

  // Resets the effect plugin.
  AKRESULT Reset() override;

  // Returns the plugin info.
  AKRESULT GetPluginInfo(AkPluginInfo& plugin_info) override;

  // Processes the next audio buffer.
  void Execute(AkAudioBuffer* input, AkUInt32 frame_offset,
               AkAudioBuffer* output) override;

  // Processes execution when the voice is virtual.
  AKRESULT TimeSkip(AkUInt32& num_frames) override;

 private:
  // Number of input ambisonic channels.
  size_t num_ambisonic_channels_;

  // Id of ambisonic source to be accumulated by the ambisonic bus.
  int ambisonic_source_id_;

  // Plugin instance id.
  int instance_id_;
};

}  // namespace wwise
}  // namespace vraudio

#endif  // RESONANCE_AUDIO_PLATFORM_WWISE_WWISE_RENDERER_FX_H_
