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

#include "geometrical_acoustics/collection_kernel.h"

#include <cmath>
#include <random>
#include <vector>

#include "third_party/googletest/googletest/include/gtest/gtest.h"
#include "Eigen/Core"
#include "base/constants_and_types.h"
#include "geometrical_acoustics/acoustic_listener.h"
#include "geometrical_acoustics/acoustic_ray.h"
#include "geometrical_acoustics/acoustic_source.h"
#include "geometrical_acoustics/sphere.h"
#include "geometrical_acoustics/test_util.h"

namespace vraudio {

namespace {

using Eigen::Vector3f;

const float kSamplingRateHz = 1000.0f;
const size_t kImpulseResponseNumSamples = 1000;

class CollectionKernelTest : public testing::Test {
 protected:
  const std::array<float, kNumReverbOctaveBands> kUnitEnergies{
      {1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f}};
};

TEST_F(CollectionKernelTest, CollectOneRayTest) {
  // A listener at (5, 0, 0).
  AcousticListener listener({5.0f, 0.0f, 0.0f}, kImpulseResponseNumSamples);

  // One ray.
  const Vector3f origin(0.0f, 0.0f, 0.0f);
  const Vector3f direction(1.0f, 0.0f, 0.0f);
  const float t_near = 0.0f;
  const float t_far = 100.0f;
  const float prior_distance = 0.0f;
  const AcousticRay ray(origin.data(), direction.data(), t_near, t_far,
                        kUnitEnergies, AcousticRay::RayType::kSpecular,
                        prior_distance);

  // Collect impulse response.
  const float listener_sphere_radius = 0.1f;
  CollectionKernel collection_kernel(
      listener_sphere_radius, kSamplingRateHz);
  collection_kernel.Collect(ray, 1.0f, &listener);

  // Validate the impulse response.
  // The theoretical index of the single non-zero element is:
  //   floor(distance (m) / kSpeedOfSound (m/s) * kSamplingRate (1/s)).
  const float distance = (listener.position - origin).norm();
  const size_t expected_index = static_cast<size_t>(
      std::floor(distance / kSpeedOfSound * kSamplingRateHz));

  // The theoretical energy value is energy * sphere_size_energy_factor.
  const float expected_sphere_size_energy_factor =
      4.0f / listener_sphere_radius / listener_sphere_radius;
  for (size_t i = 0; i < kNumReverbOctaveBands; ++i) {
    ValidateSparseFloatArray(
        listener.energy_impulse_responses.at(i), {expected_index},
        {kUnitEnergies.at(i) * expected_sphere_size_energy_factor},
        kEpsilonFloat);
  }
}

TEST_F(CollectionKernelTest, DiscardContributionTest) {
  // A listener far away that it takes 2 seconds for the ray to arrive, which
  // is longer than the impulse response of interest, i.e.
  // kImpulseResponseNumSamples / kSamplingRateHz = 1 second.
  // The contribution should be discarded.
  AcousticListener listener({kSpeedOfSound * 2.0f, 0.0f, 0.0f},
                            kImpulseResponseNumSamples);

  // One ray.
  const Vector3f origin(0.0f, 0.0f, 0.0f);
  const Vector3f direction(1.0f, 0.0f, 0.0f);
  const float t_near = 0.0f;
  const float t_far = 100.0f;
  const float prior_distance = 0.0f;
  const AcousticRay ray(origin.data(), direction.data(), t_near, t_far,
                        kUnitEnergies, AcousticRay::RayType::kSpecular,
                        prior_distance);

  // Collect impulse response.
  const float listener_sphere_radius = 0.1f;
  CollectionKernel collection_kernel(
      listener_sphere_radius, kSamplingRateHz);
  collection_kernel.Collect(ray, 1.0f, &listener);

  // Validate that the impulse responses are all zero.
  for (const std::vector<float>& energy_impulse_response :
       listener.energy_impulse_responses) {
    ValidateSparseFloatArray(energy_impulse_response, {}, {}, kEpsilonFloat);
  }
}

TEST_F(CollectionKernelTest, CollectOneRayWithPriorDistanceTest) {
  // A listener at (5, 0, 0).
  AcousticListener listener({5.0f, 0.0f, 0.0f}, kImpulseResponseNumSamples);

  // One ray.
  const Vector3f origin(0.0f, 0.0f, 0.0f);
  const Vector3f direction(1.0f, 0.0f, 0.0f);
  const float t_near = 0.0f;
  const float t_far = 100.0f;
  const float prior_distance = 10.0f;
  const AcousticRay ray(origin.data(), direction.data(), t_near, t_far,
                        kUnitEnergies, AcousticRay::RayType::kSpecular,
                        prior_distance);

  // Collect impulse response.
  const float listener_sphere_radius = 0.1f;
  CollectionKernel collection_kernel(
      listener_sphere_radius, kSamplingRateHz);
  collection_kernel.Collect(ray, 1.0f, &listener);

  // Validate the impulse response.
  // The theoretical index of the single non-zero element is:
  //   floor(distance (m) / kSpeedOfSound (m/s) * kSamplingRate (1/s)).
  // In this test distance = prior_distance + <distance on this ray>.
  const float distance = prior_distance + (listener.position - origin).norm();
  const size_t expected_index = static_cast<size_t>(
      std::floor(distance / kSpeedOfSound * kSamplingRateHz));

  // The theoretical energy value is energy * sphere_size_energy_factor.
  const float expected_sphere_size_energy_factor =
      4.0f / listener_sphere_radius / listener_sphere_radius;
  for (size_t i = 0; i < kNumReverbOctaveBands; ++i) {
    ValidateSparseFloatArray(
        listener.energy_impulse_responses.at(i), {expected_index},
        {kUnitEnergies.at(i) * expected_sphere_size_energy_factor},
        kEpsilonFloat);
  }
}

TEST_F(CollectionKernelTest, SphereSizeEnergyFactorTest) {
  // All listeners are at (0, 1, 0).
  const Vector3f listener_position(0.0f, 1.0f, 0.0f);

  // One ray.
  const Vector3f origin(0.0f, 0.0f, 0.0f);
  const Vector3f direction(0.0f, 1.0f, 0.0f);
  const float t_near = 0.0f;
  const float t_far = 100.0f;
  const float prior_distance = 0.0f;
  const AcousticRay ray(origin.data(), direction.data(), t_near, t_far,
                        kUnitEnergies, AcousticRay::RayType::kSpecular,
                        prior_distance);

  // All listeners' impulse responses should have a non-zero element at index
  // floor(distance (m) / kSpeedOfSound (m/s) * kSamplingRate (1/s)).
  const float distance = (listener_position - origin).norm();
  const size_t expected_index = static_cast<size_t>(
      std::floor(distance / kSpeedOfSound * kSamplingRateHz));

  // Collect impulse response using spheres with different radii.
  // Expect that the energy values are energy * (4 / radius^2).
  for (const float listener_sphere_radius : {0.1f, 0.2f, 0.3f, 0.4f, 0.5f}) {
    AcousticListener listener(listener_position, kImpulseResponseNumSamples);
    CollectionKernel collection_kernel(
        listener_sphere_radius, kSamplingRateHz);
    collection_kernel.Collect(ray, 1.0f, &listener);
    const float expected_sphere_size_energy_factor =
        4.0f / listener_sphere_radius / listener_sphere_radius;
    for (size_t i = 0; i < kNumReverbOctaveBands; ++i) {
      ValidateSparseFloatArray(
          listener.energy_impulse_responses.at(i), {expected_index},
          {kUnitEnergies.at(i) * expected_sphere_size_energy_factor},
          kEpsilonFloat);
    }
  }
}

TEST_F(CollectionKernelTest, CollectMultipleRaysWithWeightsTest) {
  // A listener at (0, 0, 1).
  AcousticListener listener({0.0f, 0.0f, 1.0f}, kImpulseResponseNumSamples);

  // Add many rays with different energies.
  const Vector3f origin(0.0f, 0.0f, 0.0f);
  const Vector3f direction(0.0f, 0.0f, 1.0f);
  const float t_near = 0.0f;
  const float t_far = 100.0f;
  const float prior_distance = 2.0f;
  std::vector<AcousticRay> rays;
  std::array<float, kNumReverbOctaveBands> energies = {};
  for (const float energy : {1.0f, 2.0f, 3.0f, 4.0f, 5.0f}) {
    energies.fill(energy);
    rays.emplace_back(origin.data(), direction.data(), t_near, t_far, energies,
                      AcousticRay::RayType::kSpecular, prior_distance);
  }

  // Collect impulse responses with different weights.
  const float listener_sphere_radius = 0.1f;
  CollectionKernel collection_kernel(
      listener_sphere_radius, kSamplingRateHz);
  const std::vector<float> weights = {8.0f, 4.0f, 2.0f, 1.0f, 0.5f};
  for (size_t i = 0; i < rays.size(); ++i) {
    collection_kernel.Collect(rays[i], weights[i], &listener);
  }

  // Validate the impulse responses.
  // The theoretical index of the single non-zero element is:
  //   floor(distance (m) / kSpeedOfSound (m/s) * kSamplingRate (1/s)).
  const float distance = prior_distance + (listener.position - origin).norm();
  const size_t expected_index = static_cast<size_t>(
      std::floor(distance / kSpeedOfSound * kSamplingRateHz));

  // The theoretical energy value is the sum of
  //   energy * sphere_size_energy_factor * weight for all rays.
  const float expected_sphere_size_energy_factor =
      4.0f / listener_sphere_radius / listener_sphere_radius;
  for (size_t i = 0; i < kNumReverbOctaveBands; ++i) {
    float expected_energy = 0.0f;
    for (size_t j = 0; j < rays.size(); ++j) {
      expected_energy += rays[j].energies().at(i) *
                         expected_sphere_size_energy_factor * weights[j];
    }
    ValidateSparseFloatArray(listener.energy_impulse_responses.at(i),
                             {expected_index}, {expected_energy},
                             kEpsilonFloat);
  }
}

// This test emulates a physically meaningful setting:
// 1. N rays are shot in uniformly distributed directions from a source.
// 2. Some rays intersect with a listener sphere and their contributions are
//    collected, weighted by 1/N; others miss and do not contribute.
// 3. The total effect of partial collections and the sphere size cancel out,
//    so that the energy collected is actually proportional to 1/distance^2.
TEST_F(CollectionKernelTest, CollectRaysInMonteCarloIntegrationTest) {
  // A listener at (0, 0, 2).
  AcousticListener listener({0.0f, 0.0f, 2.0f}, kImpulseResponseNumSamples);

  // Use AcousticSource to generate N = 100,000 rays with uniformly distributed
  // directions.
  const Vector3f source_position(0.0f, 0.0f, 0.0f);
  const size_t total_num_rays = 100000;
  std::vector<AcousticRay> rays;
  std::default_random_engine engine(0);
  std::uniform_real_distribution<float> distribution(0.0f, 1.0f);
  AcousticSource source(
      source_position, kUnitEnergies,
      [&engine, &distribution] { return distribution(engine); });
  for (size_t i = 0; i < total_num_rays; ++i) {
    rays.push_back(source.GenerateRay());
  }

  // Collect impulse responses using 1/N as weights. Only those rays
  // intersecting with the listener sphere is collected.
  const float listener_sphere_radius = 0.1f;
  Sphere listener_sphere;
  listener_sphere.center[0] = listener.position[0];
  listener_sphere.center[1] = listener.position[1];
  listener_sphere.center[2] = listener.position[2];
  listener_sphere.radius = listener_sphere_radius;
  listener_sphere.geometry_id = 1;
  CollectionKernel collection_kernel(
      listener_sphere_radius, kSamplingRateHz);
  const float monte_carlo_weight = 1.0f / static_cast<float>(total_num_rays);
  for (AcousticRay& ray : rays) {
    SphereIntersection(listener_sphere, &ray);
    if (ray.intersected_geometry_id() == listener_sphere.geometry_id) {
      collection_kernel.Collect(ray, monte_carlo_weight, &listener);
    }
  }

  // Validate the impulse response.
  // The theoretical index of the single non-zero element is:
  //   floor(distance (m) / kSpeedOfSound (m/s) * kSamplingRate (1/s)).
  const float distance = (listener.position - source_position).norm();
  const size_t expected_index = static_cast<size_t>(
      std::floor(distance / kSpeedOfSound * kSamplingRateHz));

  // The expected relative error of a Monte Carlo integration is O(1/sqrt(M)),
  // where M is the expected number of samples. A listener sphere of radius R
  // at a distance D away from the source is expected to intersect
  // M = N * (PI * R^2) / (4 * PI * D^2) = 0.25 * N * R^2 /D^2 rays.
  // We use 2 / sqrt(M) as the tolerance for relative errors.
  const float expected_num_intersecting_rays =
      0.25f * static_cast<float>(total_num_rays) * listener_sphere_radius *
      listener_sphere_radius / (distance * distance);
  const float relative_error_tolerance =
      2.0f / std::sqrt(expected_num_intersecting_rays);

  for (size_t i = 0; i < kNumReverbOctaveBands; ++i) {
    // The theoretical energy value is energy / distance^2.
    const float expected_energy = kUnitEnergies.at(i) / (distance * distance);
    ValidateSparseFloatArray(listener.energy_impulse_responses.at(i),
                             {expected_index}, {expected_energy},
                             relative_error_tolerance);
  }
}

TEST_F(CollectionKernelTest, CollectOneDiffuseRainRayTest) {
  // A listener at (5, 0, 0).
  AcousticListener listener({5.0f, 0.0f, 0.0f}, kImpulseResponseNumSamples);

  // A diffuse-rain ray.
  const Vector3f origin(0.0f, 0.0f, 0.0f);
  const Vector3f direction(1.0f, 0.0f, 0.0f);
  const float t_near = 0.0f;
  const float t_far = 5.0f;
  const float prior_distance = 0.0f;
  const AcousticRay diffuse_rain_ray(
      origin.data(), direction.data(), t_near, t_far, kUnitEnergies,
      AcousticRay::RayType::kDiffuse, prior_distance);

  // Collect impulse response, assuming the ray is reflected with a PDF of 0.1.
  const float direction_pdf = 0.1f;
  const float listener_sphere_radius = 0.1f;
  CollectionKernel collection_kernel(listener_sphere_radius, kSamplingRateHz);
  collection_kernel.CollectDiffuseRain(diffuse_rain_ray, direction_pdf, 1.0f,
                                       &listener);

  // Validate the impulse response.
  // The theoretical index of the single non-zero element is:
  //   floor(distance (m) / kSpeedOfSound (m/s) * kSamplingRate (1/s)).
  const float distance = (listener.position - origin).norm();
  const size_t expected_index =
      static_cast<size_t>(distance / kSpeedOfSound * kSamplingRateHz);

  // The theoretical energy value is energy * energy_factor, where
  // energy_factor is PDF / distance^2.
  const float expected_energy_factor = direction_pdf / distance / distance;
  for (size_t i = 0; i < kNumReverbOctaveBands; ++i) {
    ValidateSparseFloatArray(
        listener.energy_impulse_responses.at(i), {expected_index},
        {kUnitEnergies.at(i) * expected_energy_factor}, kEpsilonFloat);
  }
}

}  // namespace

}  // namespace vraudio
