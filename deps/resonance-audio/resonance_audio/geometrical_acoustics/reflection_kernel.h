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

#ifndef RESONANCE_AUDIO_GEOMETRICAL_ACOUSTICS_REFLECTION_KERNEL_H_
#define RESONANCE_AUDIO_GEOMETRICAL_ACOUSTICS_REFLECTION_KERNEL_H_

#include <array>
#include <functional>
#include <utility>

#include "Eigen/Core"
#include "base/constants_and_types.h"
#include "base/logging.h"
#include "geometrical_acoustics/acoustic_ray.h"

namespace vraudio {

// A class modeling reflections of acoustic rays. It takes in an incident ray
// and computes the corresponding reflected ray. NOTE: not to be confused with
// vraudio:Reflection, which models the reflection properties of a room.
//
// This class handles specular and diffuse reflections, randomly chosen based
// on the scattering coefficient. Besides, a certain portion of the energy is
// absorbed based on the absorption coefficient.
//

class ReflectionKernel {
 public:
  // Constructor.
  //
  // @param absorption_coefficients Fraction of energy being absorbed for each
  //     frequency band. All values must be in [0.0, 1.0].
  // @param scattering_coefficient Fraction of energy being scattered
  //     (diffusely reflected). Must be in [0.0, 1.0].
  // @param random_number_generator Random number generator to randomly select
  //     reflection types as well as ray directions if needed. It should
  //     implement operator() that returns a random value in [0.0, 1.0).
  ReflectionKernel(
      const std::array<float, kNumReverbOctaveBands>& absorption_coefficients,
      float scattering_coefficient,
      std::function<float()> random_number_generator)
      : scattering_coefficient_(scattering_coefficient),
        random_number_generator_(std::move(random_number_generator)) {
    for (size_t i = 0; i < kNumReverbOctaveBands; ++i) {
      reflection_coefficients_[i] = 1.0f - absorption_coefficients[i];
      CHECK(reflection_coefficients_[i] >= 0.0f &&
            reflection_coefficients_[i] <= 1.0f);
    }
    CHECK(scattering_coefficient_ >= 0.0f && scattering_coefficient_ <= 1.0f);
  }

  // Reflects a ray.
  //
  // @param incident_ray Incident (incoming) ray.
  // @return Exitant (outgoing) ray, whose origin is on the intersection point,
  //     with direction and energy calculated based on the type of reflection
  //     and the incident ray.
  AcousticRay Reflect(const AcousticRay& incident_ray) const;

  // Reflects an incident ray to create a diffuse-rain ray, whose |direction|
  // is from the reflection point to the listener position, and whose |t_far|
  // is the distance between the reflection point and the listener position.
  // All the other data are the copied from the provided
  // |reference_reflected_ray|.
  //
  // @param incident_ray Incident ray.
  // @param reflected_ray Reference reflected ray, from which the output
  //     diffuse-rain ray copies all the data except for |direction| and
  //     |t_far|.
  // @param listener_position Listener position.
  // @param direction_pdf Output probability density function that a reflected
  //     ray is in the direction of the diffuse-rain ray. Will be used to
  //     modulate the energy contribution from the diffuse-rain ray.
  // @param diffuse_rain_ray Output diffuse-rain ray.
  void ReflectDiffuseRain(const AcousticRay& incident_ray,
                          const AcousticRay& reference_reflected_ray,
                          const Eigen::Vector3f& listener_position,
                          float* direction_pdf,
                          AcousticRay* diffuse_rain_ray) const;

 private:
  std::array<float, kNumReverbOctaveBands> reflection_coefficients_;
  const float scattering_coefficient_;

  // Randon number generator for sampling reflection types and ray directions.
  std::function<float()> random_number_generator_;
};

}  // namespace vraudio

#endif  // RESONANCE_AUDIO_GEOMETRICAL_ACOUSTICS_REFLECTION_KERNEL_H_
