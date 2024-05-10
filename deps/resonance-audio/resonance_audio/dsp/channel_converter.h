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

#ifndef RESONANCE_AUDIO_DSP_CHANNEL_CONVERTER_H_
#define RESONANCE_AUDIO_DSP_CHANNEL_CONVERTER_H_

#include "base/audio_buffer.h"

namespace vraudio {

// Converts a mono |input| buffer to a stereo |output| buffer by preserving the
// signal energy.
//
// @param input Mono input buffer.
// @param output Pointer to a stereo output buffer.
void ConvertStereoFromMono(const AudioBuffer& input, AudioBuffer* output);

// Converts a stereo |input| buffer to a mono |output| buffer by preserving the
// signal energy.
//
// @param input Stereo input buffer.
// @param output Pointer to a mono output buffer.
void ConvertMonoFromStereo(const AudioBuffer& input, AudioBuffer* output);

}  // namespace vraudio

#endif  // RESONANCE_AUDIO_DSP_CHANNEL_CONVERTER_H_
