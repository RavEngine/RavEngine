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

#ifndef RESONANCE_AUDIO_GEOMETRICAL_ACOUSTICS_ACOUSTIC_SOURCE_H_
#define RESONANCE_AUDIO_GEOMETRICAL_ACOUSTICS_ACOUSTIC_SOURCE_H_

#include <array>
#include <functional>
#include <utility>
#include <vector>

#include "Eigen/Core"
#include "base/constants_and_types.h"
#include "geometrical_acoustics/acoustic_ray.h"
#include "geometrical_acoustics/sampling.h"

namespace vraudio {

// A class modeling a sound source. Currently we only support point source
// that emits rays uniformly in all directions.
class AcousticSource {
 public:
  // Constructor.
  //
  // @param position Source position.
  // @param energies Source energies. This will be the initial energies for
  //     all frequency bands shared by all rays generated from this source.
  // @param random_number_generator Random number generator used to sample
  //     ray directions. It should implement operator() that returns a
  //     random value in [0.0, 1.0).
  AcousticSource(const Eigen::Vector3f& position,
                 const std::array<float, kNumReverbOctaveBands>& energies,
                 std::function<float()> random_number_generator)
      : position_(position),
        energies_(energies),
        random_number_generator_(std::move(random_number_generator)) {}

  // Generates one ray.
  //
  // @return AcousticRay starting from this source.
  AcousticRay GenerateRay() const {
    const Eigen::Vector3f& direction = UniformSampleSphere(
        random_number_generator_(), random_number_generator_());
    return AcousticRay(position_.data(), direction.data(), 0.0f,
                       AcousticRay::kInfinity, energies_,
                       AcousticRay::RayType::kSpecular, 0.0f);
  }

  // Generates a vector of rays at once, using stratified sampling. The rays
  // generated this way are more "evenly spaced", with fewer holes and clusters.
  //
  // One caveat: only the whole set of the returned rays is uniformly
  // distributed (the expected number of rays found in a finite solid angle
  // is the same in any direction), while any subset with fewer than
  // |num_rays| rays is not.
  //
  // In contrast, the GenerateRay() above guarantees any subset is a uniformly
  // distributed set of rays. This is why the function is designed to return
  // a vector of rays, which are meant to be used as a whole and not partially.
  //
  // @param num_rays Number of rays; must be equal to |sqrt_num_rays|^2.
  // @param sqrt_num_rays The square root of number of rays to emit.
  // @return A vector of |sqrt_num_rays|^2 AcousticRays starting from this
  //     source.
  std::vector<AcousticRay> GenerateStratifiedRays(size_t num_rays,
                                                  size_t sqrt_num_rays) const {
    // To save computation time, it is the caller's responsibility to make sure
    // that |num_rays| is equal to |sqrt_num_rays|^2.
    DCHECK_EQ(sqrt_num_rays * sqrt_num_rays, num_rays);

    std::vector<AcousticRay> rays;
    rays.reserve(num_rays);
    for (size_t ray_index = 0; ray_index < num_rays; ++ray_index) {
      const Eigen::Vector3f& direction = StratifiedSampleSphere(
          random_number_generator_(), random_number_generator_(), sqrt_num_rays,
          ray_index);
      rays.push_back(AcousticRay(position_.data(), direction.data(),
                                 0.0f /* t_near */, AcousticRay::kInfinity,
                                 energies_, AcousticRay::RayType::kSpecular,
                                 0.0f /* prior_distance */));
    }
    return rays;
  }

 private:
  // The position of this point source.
  const Eigen::Vector3f position_;

  // The energy of this source.
  const std::array<float, kNumReverbOctaveBands> energies_;

  // Randon number generator for sampling ray directions.
  std::function<float()> random_number_generator_;
};

}  // namespace vraudio

#endif  // RESONANCE_AUDIO_GEOMETRICAL_ACOUSTICS_ACOUSTIC_SOURCE_H_
