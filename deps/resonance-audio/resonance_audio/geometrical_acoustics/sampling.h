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

// Collection of sampling functions useful for in stochastic ray tracing.

#ifndef RESONANCE_AUDIO_GEOMETRICAL_ACOUSTICS_SAMPLING_H_
#define RESONANCE_AUDIO_GEOMETRICAL_ACOUSTICS_SAMPLING_H_

#include <cmath>

#include "Eigen/Dense"
#include "base/constants_and_types.h"
#include "base/logging.h"

namespace vraudio {

// Samples a vector uniformly from a unit sphere given two random variables
// whose values are in [0, 1).
//
// @param u The first random variable.
// @param v The second random variable.
// @return The sampled vector.
inline Eigen::Vector3f UniformSampleSphere(float u, float v) {
  DCHECK(u >= 0.0f && u <= 1.0f);
  DCHECK(v >= 0.0f && v <= 1.0f);
  const float cos_theta = 1.0f - 2.0f * v;
  const float sin_theta = 2.0f * std::sqrt(v * (1.0f - v));
  const float phi = kTwoPi * u;
  return Eigen::Vector3f(std::cos(phi) * sin_theta, std::sin(phi) * sin_theta,
                         cos_theta);
}

// Samples a vector from a unit sphere in a stratified fashion given two
// random variables whose values are in [0, 1). The collective results from
// |sample_index| iterating from 0 to |sqrt_num_samples|^2 are uniformly
// distributed on a sphere.
//
// @param u The first random variable.
// @param v The second random variable.
// @param sqrt_num_samples The square root of the total number of samples.
// @param sample_index The index of the currently sampled vector.
// @return The sampled vector.
inline Eigen::Vector3f StratifiedSampleSphere(float u, float v,
                                              size_t sqrt_num_samples,
                                              size_t sample_index) {
  DCHECK(u >= 0.0f && u <= 1.0f);
  DCHECK(v >= 0.0f && v <= 1.0f);

  // Make a domain that have |sqrt_num_samples| by |sqrt_num_samples| cells.
  // First decide which cell that this sample is in, denoted by the coordinates
  // of the cell's top-left of corner.
  const float cell_x = static_cast<float>(sample_index / sqrt_num_samples);
  const float cell_y = static_cast<float>(sample_index % sqrt_num_samples);

  // Then pick a point inside the cell using the two random variables u and v.
  // Normalize the point to the [0, 1) x [0, 1) domain and send it to
  // UniformSampleSphere().
  const float cell_width = 1.0f / static_cast<float>(sqrt_num_samples);
  return UniformSampleSphere((cell_x + u) * cell_width,
                             (cell_y + v) * cell_width);
}

// Samples a vector from a unit hemisphere according to the cosine-weighted
// distribution given two random variables whose values are in [0, 1). The
// hemisphere lies on a plane whose normal is assumed to be on the +z direction.
//

static Eigen::Vector3f CosineSampleHemisphere(float u, float v) {
  DCHECK(u >= 0.0f && u <= 1.0f);
  DCHECK(v >= 0.0f && v <= 1.0f);
  const float cos_theta = std::sqrt(1.0f - v);
  const float sin_theta = std::sqrt(v);
  const float phi = kTwoPi * u;
  return Eigen::Vector3f(std::cos(phi) * sin_theta, std::sin(phi) * sin_theta,
                         cos_theta);
}

// Same as above but the hemisphere lies on a plane whose normal is specified
// by the user (instead of the +z direction).
//
// @param u The first random variable.
// @param v The second random variable.
// @param unit_normal The normal of the plane on which the hemisphere resides.
// @return The sampled vector.
static Eigen::Vector3f CosineSampleHemisphere(
    float u, float v, const Eigen::Vector3f& unit_normal) {
  Eigen::Vector3f local_vector = CosineSampleHemisphere(u, v);

  return Eigen::Quaternionf::FromTwoVectors(Eigen::Vector3f::UnitZ(),
                                            unit_normal) *
         local_vector;
}

// The probability density function (PDF) that a vector is in a particular
// direction if the vector is sampled from a cosine-weigted distribution over
// a unit hemisphere (i.e. it is sampled from the CosineSampleHemisphere()
// above).
//
// @param unit_normal The normal of the plane on which the hemisphere resides.
// @param unit_direction The direction of the sampled vector.
// @return Probability density that a sampled vector is in the |unit_direction|.
static float CosineSampleHemispherePdf(const Eigen::Vector3f& unit_normal,
                                       const Eigen::Vector3f& unit_direction) {
  const float cos_theta = unit_normal.dot(unit_direction);

  // Returns zero probability if the |unit_normal| and |unit_direction| lie on
  // different sides of the plane, i.e., their inner-product is negative.
  return cos_theta >= 0.0f ? cos_theta / kPi : 0.0f;
}

}  // namespace vraudio

#endif  // RESONANCE_AUDIO_GEOMETRICAL_ACOUSTICS_SAMPLING_H_
