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

#include "geometrical_acoustics/impulse_response_computer.h"

#include <array>
#include <cmath>
#include <random>
#include <utility>
#include <vector>

#include "third_party/googletest/googletest/include/gtest/gtest.h"
#include "base/constants_and_types.h"
#include "geometrical_acoustics/acoustic_listener.h"
#include "geometrical_acoustics/acoustic_ray.h"
#include "geometrical_acoustics/acoustic_source.h"
#include "geometrical_acoustics/path.h"
#include "geometrical_acoustics/scene_manager.h"
#include "geometrical_acoustics/test_util.h"

namespace vraudio {

namespace {

using Eigen::Vector3f;

const float kSamplingRateHz = 1000.0f;
const float kListenerSphereRadiusMeter = 0.1f;
const size_t kImpulseResponseNumSamples = 1000;

// The energy collected from a ray.
// kCollectedEnergyPerRay
//   = 1.0 * CollectionKernel::sphere_size_energy_factor_
//   = 1.0 * 4.0 / kListenerSphereRadius^2 = 1.0 * 4.0 / 0.01
//   = 400.0.
// See CollectionKernel::sphere_size_energy_factor_ for details.
const float kCollectedEnergyPerRay = 400.0f;

class ImpulseResponseComputerTest : public testing::Test {
 public:
  void SetUp() override { paths_.clear(); }

 protected:
  void AddListenersAtPositions(const std::vector<Vector3f>& positions,
                               std::vector<AcousticListener>* listeners) {
    for (const Vector3f& position : positions) {
      listeners->emplace_back(position, kImpulseResponseNumSamples);
    }
  }

  // Validate indices and values of the impulse responses for all listeners.
  //
  // param@ listeners Listeners whose impulse responses are to be validated.
  // param@ expected_indices_for_listeners Expected indices of the non-zero
  //     elements for all listeners' impulse responses. So that
  //     expected_indices_for_listeners[i][j] is the index of the j-th non-zero
  //     element in listener i's impulse response.
  // param@ expected_energies_for_listeners Expected energy values of the
  //     non-zero elements for all listeners' impulse responses. So that
  //     the value stored in expected_energies_for_listeners[i][j] is the j-th
  //     non-zero element in listener i's impulse response.
  // param@ relative_error_tolerance_for_listeners Tolerances of relative errors
  //     when comparing energy impulse responses for all listeners.
  void ValidateImpulseResponses(
      const std::vector<AcousticListener>& listeners,
      const std::vector<std::vector<size_t>>& expected_indices_for_listeners,
      const std::vector<std::vector<float>>& expected_energies_for_listeners,
      const std::vector<float> relative_error_tolerances_for_listeners) {
    ASSERT_EQ(expected_indices_for_listeners.size(), listeners.size());
    ASSERT_EQ(expected_energies_for_listeners.size(), listeners.size());
    ASSERT_EQ(relative_error_tolerances_for_listeners.size(), listeners.size());
    for (size_t i = 0; i < listeners.size(); ++i) {
      for (size_t j = 0; j < kNumReverbOctaveBands; ++j) {
        ValidateSparseFloatArray(listeners[i].energy_impulse_responses.at(j),
                                 expected_indices_for_listeners[i],
                                 expected_energies_for_listeners[i],
                                 relative_error_tolerances_for_listeners[i]);
      }
    }
  }

  const std::array<float, kNumReverbOctaveBands> kUnitEnergies{
      {1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f}};

  std::vector<Path> paths_;
  SceneManager scene_manager_;
};

TEST_F(ImpulseResponseComputerTest, OnePathMultipleRaysOneListenerTest) {
  // A listener at (1, 0, 0).
  std::unique_ptr<std::vector<AcousticListener>> listeners(
      new std::vector<AcousticListener>);
  const Vector3f listener_position(1.0f, 0.0f, 0.0f);
  AddListenersAtPositions({listener_position}, listeners.get());

  scene_manager_.BuildScene({}, {});
  ImpulseResponseComputer impulse_response_computer(
      kListenerSphereRadiusMeter, kSamplingRateHz, std::move(listeners),
      &scene_manager_);

  // One path with multiple ray segments.
  Path path;

  // Imagine a scene with two specular walls parallel to the y-z plane.
  // One (called the right wall) is at (2, 0, 0) and the other (called the
  // left wall) at (-2, 0, 0). A ray shooting at the +x direction will reflect
  // back-and-forth between these parallel walls.
  //
  // First ray segment: from source at (0, 0, 0), shooting at (1, 0, 0).
  const Vector3f source_position(0.0f, 0.0f, 0.0f);
  const Vector3f left_to_right_direction(1.0f, 0.0f, 0.0f);
  const float wall_position_x = 2.0f;
  path.rays.emplace_back(source_position.data(), left_to_right_direction.data(),
                         0.0f, wall_position_x, kUnitEnergies,
                         AcousticRay::RayType::kSpecular, 0.0f);

  // Also record the distances traveled each time the ray passes through the
  // listener sphere. This will be used later to verify the indices of non-zero
  // elements in the impulse response.
  std::vector<float> source_listener_distances = {
      (source_position - listener_position).norm()};  // First segment.

  // The rest of the reflections. Half of these rays start from the left wall
  // and stop at the right wall, and the other half start from the right wall
  // and stop at the left wall. We compute up to order |max_order| = 10.
  const Vector3f right_wall_origin(wall_position_x, 0.0f, 0.0f);
  const Vector3f left_wall_origin(-wall_position_x, 0.0f, 0.0f);
  const float distance_between_walls =
      (right_wall_origin - left_wall_origin).norm();
  const float listener_left_wall_distance =
      (listener_position - left_wall_origin).norm();
  const float listener_right_wall_distance =
      (listener_position - right_wall_origin).norm();
  const Vector3f right_to_left_direction = -left_to_right_direction;
  const size_t max_order = 10;

  // Prior distance corresponds to the first ray segment.
  float prior_distance = (right_wall_origin - source_position).norm();
  for (size_t order = 1; order < max_order; ++order) {
    float source_listener_distance = source_listener_distances.back();
    if (order % 2 == 0) {
      path.rays.emplace_back(left_wall_origin.data(),
                             left_to_right_direction.data(), 0.0f,
                             distance_between_walls, kUnitEnergies,
                             AcousticRay::RayType::kSpecular, prior_distance);

      // Add the distance of {listener -> left wall -> listener} to the
      // accumulated source-listener distance.
      source_listener_distance += 2.0f * listener_left_wall_distance;
    } else {
      path.rays.emplace_back(right_wall_origin.data(),
                             right_to_left_direction.data(), 0.0f,
                             distance_between_walls, kUnitEnergies,
                             AcousticRay::RayType::kSpecular, prior_distance);

      // Add the distance of {listener -> right wall -> listener} to the
      // accumulated source-listener distance.
      source_listener_distance += 2.0f * listener_right_wall_distance;
    }
    source_listener_distances.push_back(source_listener_distance);

    // Each reflection adds |distance_between_walls| to |prior_distance|.
    prior_distance += distance_between_walls;
  }
  paths_.push_back(path);

  // Compute impulse response.
  impulse_response_computer.CollectContributions(paths_);

  // Validate the impulse responses.
  // The expected indices are
  // floor(distance / kSpeedOfSound (m/s) * kSamplingRateHz (1/s)).
  // The theoretical energy values are all kCollectedEnergyPerRay.
  std::vector<std::vector<size_t>> expected_indices_for_listeners(1);
  std::vector<std::vector<float>> expected_energies_for_listeners(1);
  for (size_t order = 0; order < max_order; ++order) {
    expected_indices_for_listeners.back().push_back(
        static_cast<size_t>(std::floor(source_listener_distances[order] /
                                       kSpeedOfSound * kSamplingRateHz)));
    expected_energies_for_listeners.back().push_back(kCollectedEnergyPerRay);
  }
  ValidateImpulseResponses(impulse_response_computer.GetFinalizedListeners(),
                           expected_indices_for_listeners,
                           expected_energies_for_listeners, {kEpsilonFloat});
}

TEST_F(ImpulseResponseComputerTest, OnePathMultipleListenersTest) {
  // A series of listeners along the z-axis. Some of them overlapping.
  std::unique_ptr<std::vector<AcousticListener>> listeners(
      new std::vector<AcousticListener>);
  AddListenersAtPositions({{0.0f, 0.0f, 1.0f},
                           {0.0f, 0.0f, 2.0f},
                           {0.0f, 0.0f, 3.0f},
                           {0.0f, 0.0f, 3.05f},
                           {0.0f, 0.0f, 3.10f}},
                          listeners.get());

  scene_manager_.BuildScene({}, {});
  ImpulseResponseComputer impulse_response_computer(
      kListenerSphereRadiusMeter, kSamplingRateHz, std::move(listeners),
      &scene_manager_);

  // One path with only one ray. This ray should pass through all listener
  // spheres.
  const Vector3f origin(0.0f, 0.0f, 0.0f);
  const Vector3f direction(0.0f, 0.0f, 1.0f);
  Path path;
  path.rays.emplace_back(origin.data(), direction.data(), 0.0f, 100.0f,
                         kUnitEnergies, AcousticRay::RayType::kSpecular, 0.0f);
  paths_.push_back(path);

  // Compute impulse response.
  impulse_response_computer.CollectContributions(paths_);

  // Validate the impulse responses.
  // The theoretical indices are
  // floor(distance / kSpeedOfSound (m/s) * kSamplingRateHz (1/s)).
  // The theoretical energy values are all kCollectedEnergyPerRay.
  std::vector<std::vector<size_t>> expected_indices_for_listeners;
  std::vector<std::vector<float>> expected_energies_for_listeners;
  std::vector<float> relative_error_tolerances_for_listeners;
  for (const AcousticListener& listener :
       impulse_response_computer.GetFinalizedListeners()) {
    const float distance = (listener.position - origin).norm();
    expected_indices_for_listeners.push_back({static_cast<size_t>(
        std::floor(distance / kSpeedOfSound * kSamplingRateHz))});
    expected_energies_for_listeners.push_back({kCollectedEnergyPerRay});
    relative_error_tolerances_for_listeners.push_back(kEpsilonFloat);
  }
  ValidateImpulseResponses(impulse_response_computer.GetFinalizedListeners(),
                           expected_indices_for_listeners,
                           expected_energies_for_listeners,
                           relative_error_tolerances_for_listeners);
}

// Test that the energies collected at different positions is proportional to
// 1/D^2, where D is the source-listener distance.
TEST_F(ImpulseResponseComputerTest,
       EnergyInverselyProportionalDistanceSquaredTest) {
  // A series of listeners along the y-axis.
  std::unique_ptr<std::vector<AcousticListener>> listeners(
      new std::vector<AcousticListener>);
  AddListenersAtPositions(
      {
          {0.0f, 1.0f, 0.0f},
          {0.0f, 1.2f, 0.0f},
          {0.0f, 1.4f, 0.0f},
          {0.0f, 1.6f, 0.0f},
          {0.0f, 1.8f, 0.0f},
          {0.0f, 2.0f, 0.0f},
          {0.0f, 2.5f, 0.0f},
          {0.0f, 3.0f, 0.0f},
      },
      listeners.get());

  scene_manager_.BuildScene({}, {});
  ImpulseResponseComputer impulse_response_computer(
      kListenerSphereRadiusMeter, kSamplingRateHz, std::move(listeners),
      &scene_manager_);

  // Add 10,000 paths from rays with uniformly distributed directions.
  const size_t min_num_paths = 10000;
  const Vector3f source_position(0.0f, 0.0f, 0.0f);
  paths_ = GenerateUniformlyDistributedRayPaths(source_position.data(),
                                                min_num_paths);

  // Compute impulse response.
  impulse_response_computer.CollectContributions(paths_);

  // Check the index and value of the single non-zero element for each listener.
  std::vector<std::vector<size_t>> expected_indices_for_listeners;
  std::vector<std::vector<float>> expected_energies_for_listeners;
  std::vector<float> relative_error_tolerances_for_listeners;
  for (const AcousticListener& listener :
       impulse_response_computer.GetFinalizedListeners()) {
    // The theoretical index is
    // floor(distance / kSpeedOfSound (m/s) * kSamplingRateHz (1/s)).
    const float distance = (listener.position - source_position).norm();
    expected_indices_for_listeners.push_back({static_cast<size_t>(
        std::floor(distance / kSpeedOfSound * kSamplingRateHz))});

    // The theoretical energy value is 1.0 / distance^2.
    expected_energies_for_listeners.push_back({1.0f / (distance * distance)});

    // The expected relative error of a Monte Carlo integration is O(1/sqrt(M)),
    // where M is the expected number of samples. A listener sphere of radius R
    // at a distance D away from the source is expected to intersect
    // M = N * (PI * R^2) / (4 * PI * D^2) = 0.25 * N * R^2 /D^2 rays.
    // We use 2 / sqrt(M) as the tolerance for relative errors.
    const float expected_num_intersecting_rays =
        0.25f * static_cast<float>(paths_.size()) * kListenerSphereRadiusMeter *
        kListenerSphereRadiusMeter / (distance * distance);
    relative_error_tolerances_for_listeners.push_back(
        2.0f / std::sqrt(expected_num_intersecting_rays));
  }
  ValidateImpulseResponses(impulse_response_computer.GetFinalizedListeners(),
                           expected_indices_for_listeners,
                           expected_energies_for_listeners,
                           relative_error_tolerances_for_listeners);
}

// Tests that collecting after GetFinalizedListeners() is called has no effect.
TEST_F(ImpulseResponseComputerTest, CollectingAfterFinalizeHasNoEffect) {
  // A listener at (1, 0, 0).
  std::unique_ptr<std::vector<AcousticListener>> listeners(
      new std::vector<AcousticListener>);
  const Vector3f listener_position(1.0f, 0.0f, 0.0f);
  AddListenersAtPositions({listener_position}, listeners.get());

  scene_manager_.BuildScene({}, {});
  ImpulseResponseComputer impulse_response_computer(
      kListenerSphereRadiusMeter, kSamplingRateHz, std::move(listeners),
      &scene_manager_);

  // One path with only one ray. This ray should pass through the listener
  // sphere.
  const Vector3f origin(0.0f, 0.0f, 0.0f);
  const Vector3f direction(1.0f, 0.0f, 0.0f);
  Path path;
  path.rays.emplace_back(origin.data(), direction.data(), 0.0f, 100.0f,
                         kUnitEnergies, AcousticRay::RayType::kSpecular, 0.0f);
  paths_.push_back(path);
  impulse_response_computer.CollectContributions(paths_);

  // Finalize the listeners and make a copy of the energy impulse responses.
  const std::array<std::vector<float>, kNumReverbOctaveBands>
      old_energy_impulse_responses =
          impulse_response_computer.GetFinalizedListeners()
              .at(0)
              .energy_impulse_responses;

  // Try to collect another set of paths. This would change the energies if
  // they were collected, but they are not because the collection is finalized.
  paths_.clear();
  const Vector3f another_origin(1.0f, -1.0f, 0.0f);
  const Vector3f another_direction(0.0f, 1.0f, 0.0f);
  Path another_path;
  another_path.rays.emplace_back(
      another_origin.data(), another_direction.data(), 0.0f, 100.0f,
      kUnitEnergies, AcousticRay::RayType::kSpecular, 0.0f);
  paths_.push_back(another_path);
  impulse_response_computer.CollectContributions(paths_);

  // Verify that the energy impulse responses are the same as the copy.
  const std::array<std::vector<float>, kNumReverbOctaveBands>&
      new_energy_impulse_responses =
          impulse_response_computer.GetFinalizedListeners()
              .at(0)
              .energy_impulse_responses;
  for (size_t band = 0; band < kNumReverbOctaveBands; ++band) {
    const std::vector<float>& old_responses_in_band =
        old_energy_impulse_responses[band];
    const std::vector<float>& new_responses_in_band =
        new_energy_impulse_responses[band];
    for (size_t index = 0; index < old_responses_in_band.size(); ++index) {
      EXPECT_FLOAT_EQ(old_responses_in_band[index],
                      new_responses_in_band[index]);
    }
  }
}

}  // namespace

}  // namespace vraudio
