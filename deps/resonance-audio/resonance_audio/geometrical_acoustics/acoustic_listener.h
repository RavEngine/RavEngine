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

#ifndef RESONANCE_AUDIO_GEOMETRICAL_ACOUSTICS_ACOUSTIC_LISTENER_H_
#define RESONANCE_AUDIO_GEOMETRICAL_ACOUSTICS_ACOUSTIC_LISTENER_H_

#include <array>
#include <cstddef>
#include <vector>

#include "Eigen/Core"
#include "base/constants_and_types.h"

namespace vraudio {
struct AcousticListener {
  // Constructor.
  //
  // @param listener_position Position of the listener.
  // @param impulse_response_length Number of samples in the energy impulse
  //     response for each frequency band.
  AcousticListener(const Eigen::Vector3f& listener_position,
                   size_t impulse_response_length)
      : position(listener_position) {
    for (size_t i = 0; i < kNumReverbOctaveBands; ++i) {
      energy_impulse_responses[i] =
          std::vector<float>(impulse_response_length, 0.0f);
    }
  }

  const Eigen::Vector3f position;

  // Impulse responses in terms of energies for all frequency bands.
  // Need to take square roots before applying it to an input signal, which
  // models pressure. The reason we store response in energy instead of in
  // pressure is because the geometrical acoustics computation is energy-based,
  // and the following kind of operations are performed extensively:
  //   energy_impulse_responses[i][t] += <energy contribution from a ray>.
  std::array<std::vector<float>, kNumReverbOctaveBands>
      energy_impulse_responses;
};

}  // namespace vraudio

#endif  // RESONANCE_AUDIO_GEOMETRICAL_ACOUSTICS_ACOUSTIC_LISTENER_H_
