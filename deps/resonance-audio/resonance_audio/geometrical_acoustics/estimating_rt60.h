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

// Function to estimate RT60s.

#ifndef RESONANCE_AUDIO_GEOMETRICAL_ACOUSTICS_ESTIMATING_RT60_H_
#define RESONANCE_AUDIO_GEOMETRICAL_ACOUSTICS_ESTIMATING_RT60_H_

#include <vector>

namespace vraudio {

// Estimates the RT60 value from collected energy impulse responses.
//
// @param energy_impulse_responses Energy impulse responses.
// @param sampling_rate Sampling rate in Hz. Used to convert indices of the
//     impulse response vector to time in seconds.
// @return Estimated RT60 value in seconds. Returns 0.0f (meaning no reverb
//     effect at all) when the estimation fails.
float EstimateRT60(const std::vector<float>& energy_impulse_responses,
                   float sampling_rate);

}  // namespace vraudio

#endif  // RESONANCE_AUDIO_GEOMETRICAL_ACOUSTICS_ESTIMATING_RT60_H_
