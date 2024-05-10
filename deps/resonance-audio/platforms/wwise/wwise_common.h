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

#ifndef RESONANCE_AUDIO_PLATFORM_WWISE_WWISE_COMMON_H_
#define RESONANCE_AUDIO_PLATFORM_WWISE_WWISE_COMMON_H_

#include <list>
#include <memory>
#include <vector>

#include "AK/SoundEngine/Common/AkCommonDefs.h"
#include "AK/SoundEngine/Common/AkTypes.h"
#include "AK/Tools/Common/AkAssert.h"
#include "api/resonance_audio_api.h"

namespace vraudio {
namespace wwise {

// Number of channels in stereo output configuration.
const size_t kNumStereoChannels = 2;

// Company ID reserved for Google third-party plugins.
const AkUInt32 kCompanyId = 272;

// Plugin ID for Google VR binaural renderer.
const AkUInt32 kRendererPluginId = 100;

// Plugin ID for Google VR room effects.
const AkUInt32 kRoomEffectsPluginId = 200;

// Plugin ID for Google VR room effects attachment.
const AkUInt32 kRoomEffectsAttachmentPluginId = 201;

// Invalid plugin instance id.
const int kInvalidInstanceId = -1;

// Default dialog id for a plugin UI window in the Wwise Authoring app. This
// should be passed as the dialog id if no custom dialog window is implemented.
const unsigned int kDefaultPluginDialogId = 0;

// Stores a singleton ResonanceAudio system with a reference count.
struct ResonanceAudioSystem {
  ResonanceAudioSystem()
      : api(nullptr), num_instances(0), instance_id_counter(0) {}

  // Singleton ResonanceAudio API instance to communicate with the internal
  // system.
  std::unique_ptr<ResonanceAudioApi> api;

  // Number of global instances attempted to initialize.

  int num_instances;

  // List of currently active plugin instance ids.
  std::list<int> instance_ids;

  // Incremental plugin instance id counter.
  int instance_id_counter;
};

// Returns a vector of raw pointers to beginning of each channel in given audio
// buffer.
//
// @param buffer Pointer to Audikinetic planar audio buffer.
// @return Vector of raw pointers per each buffer channel.
template <typename T>
inline std::vector<T> GetChannelPointerArrayFromAkAudioBuffer(
    AkAudioBuffer* buffer) {
  const AkUInt32 num_channels = buffer->NumChannels();
  AKASSERT(num_channels >= 0U);

  std::vector<T> channel_ptrs(static_cast<size_t>(num_channels));
  for (AkUInt32 channel = 0; channel < num_channels; ++channel) {
    channel_ptrs[channel] = static_cast<T>(buffer->GetChannel(channel));
  }
  return channel_ptrs;
}

}  // namespace wwise
}  // namespace vraudio

#endif  // RESONANCE_AUDIO_PLATFORM_WWISE_WWISE_COMMON_H_
