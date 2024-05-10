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

#ifndef RESONANCE_AUDIO_GEOMETRICAL_ACOUSTICS_COLLECTION_KERNEL_H_
#define RESONANCE_AUDIO_GEOMETRICAL_ACOUSTICS_COLLECTION_KERNEL_H_

#include "geometrical_acoustics/acoustic_listener.h"
#include "geometrical_acoustics/acoustic_ray.h"

namespace vraudio {

// A class modeling the collection of energy contributions from an acoustic ray.
class CollectionKernel {
 public:
  // Constructor.
  //
  // @param listener_sphere_radius Radius of listener spheres (m).
  // @param sampling_rate Sampling rate (Hz). Together with speed of sound we
  //     can convert a physical time to an index in an impulse response array.
  CollectionKernel(float listener_sphere_radius, float sampling_rate);

  // Collects contributions from a ray to a listener's impulse response.
  //
  // @param ray AcousticRay whose contribution is to be collected.
  // @param weight Weight of the contribution on |ray|.
  // @param listener AcousticListener to which the contribution from |ray| is
  //     added in-place.
  void Collect(const AcousticRay& ray, float weight,
               AcousticListener* listener) const;

  // Collects contributions from a diffuse-rain ray to a listener's impulse
  // response.
  //
  // @param diffuse_rain_ray Diffuse-rain ray whose contribution is to be
  //     collected.
  // @param weight Weight of the contribution on |diffuse_rain_ray|.
  // @param direction_pdf The probability density function that a ray is in
  //     the direction of |diffuse_rain_ray|, with density taken over the solid
  //     angle. Used to scale the energy contributions. The unit is
  //     1 / steradian.
  // @param listener AcousticListener to which the contribution from |ray| is
  //     added in-place.
  void CollectDiffuseRain(const AcousticRay& diffuse_rain_ray, float weight,
                          float direction_pdf,
                          AcousticListener* listener) const;

 private:
  // Because we use a sphere to collect rays, the number of rays (and therefore
  // the amount of energy) collected is proportional to the cross section area
  // of the sphere, i.e. energy ~ R^2, where R is the sphere radius. But
  // because the energy at a listener position should be independent of the
  // sphere radius (which is an artificial construct), we need to factor out
  // the radius-dependency, by multiplying the sum of energy with a constant,
  // |sphere_size_energy_factor_|.
  //
  // In our implementation, this constant is defined such that a listener
  // 1.0 meter away from the source would give an attenuation of 1.0.
  const float sphere_size_energy_factor_;

  // Converting traveled distance in meter to an index in the impulse response
  // array: index = distance (m) / speed_of_sound (m/s) * sampling_rate (1/s).
  const float distance_to_impulse_response_index_;
};

}  // namespace vraudio
#endif  // RESONANCE_AUDIO_GEOMETRICAL_ACOUSTICS_COLLECTION_KERNEL_H_
