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

#include "geometrical_acoustics/estimating_rt60.h"

#include <cmath>
#include <memory>
#include <vector>

#include "third_party/googletest/googletest/include/gtest/gtest.h"
#include "base/constants_and_types.h"
#include "geometrical_acoustics/impulse_response_computer.h"
#include "geometrical_acoustics/test_util.h"
#include "platforms/common/room_effects_utils.h"

namespace vraudio {

namespace {

class EstimateRT60Test : public testing::Test {
 public:
  EstimateRT60Test() {
    Vertex min_corner = {cube_center_[0] - 0.5f * cube_dimensions_[0],
                         cube_center_[1] - 0.5f * cube_dimensions_[1],
                         cube_center_[2] - 0.5f * cube_dimensions_[2]};
    Vertex max_corner = {cube_center_[0] + 0.5f * cube_dimensions_[0],
                         cube_center_[1] + 0.5f * cube_dimensions_[1],
                         cube_center_[2] + 0.5f * cube_dimensions_[2]};

    BuildTestBoxScene(min_corner, max_corner, &cube_vertices_, &cube_triangles_,
                      &cube_wall_triangles_);
  }

 protected:
  std::vector<float> CollectImpulseResponsesAndEstimateRT60(
      const std::vector<Path>& paths, SceneManager* scene_manager) {
    // Listener and impulse response computer.
    std::unique_ptr<std::vector<AcousticListener>> listeners(
        new std::vector<AcousticListener>);
    listeners->emplace_back(Eigen::Vector3f(cube_center_),
                            impulse_response_num_samples_);
    ImpulseResponseComputer impulse_response_computer(
        listener_sphere_radius_, sampling_rate_, std::move(listeners),
        scene_manager);

    // Collect impulse responses.
    impulse_response_computer.CollectContributions(paths);
    const std::array<std::vector<float>, kNumReverbOctaveBands>&
        energy_impulse_responses =
            impulse_response_computer.GetFinalizedListeners()
                .at(0)
                .energy_impulse_responses;

    // Estimate RT60 values.
    std::vector<float> output_rt60_values(kNumReverbOctaveBands, 0.0f);
    for (size_t band = 0; band < kNumReverbOctaveBands; ++band) {
      output_rt60_values[band] =
          EstimateRT60(energy_impulse_responses[band], sampling_rate_);
    }
    return output_rt60_values;
  }

  // Ray-tracing related fields.
  const float sampling_rate_ = 48000.0f;
  const float listener_sphere_radius_ = 0.1f;
  const size_t impulse_response_num_samples_ = 96000;
  Eigen::Vector3f listener_position_;

  // Data describing a cube scene.
  std::vector<Vertex> cube_vertices_;
  std::vector<Triangle> cube_triangles_;
  const float cube_center_[3] = {0.5f, 0.5f, 0.5f};
  const float cube_dimensions_[3] = {1.0f, 1.0f, 1.0f};

  std::vector<MaterialName> wall_materials_;

  // Triangles for six walls of a cube. Useful for assigning surface materials.
  std::vector<std::unordered_set<unsigned int>> cube_wall_triangles_;
};

// Tests that estimating from an empty impulse responses vector fails.
TEST_F(EstimateRT60Test, EstimateFromEmptyImpulseResponsesFails) {
  std::vector<float> empty_energy_impulse_responses;

  // Expect that the estimation function returns 0.
  EXPECT_EQ(0.0f, EstimateRT60(empty_energy_impulse_responses, sampling_rate_));
}

// Tests that if the impulse responses are increasing in energy and therefore
// RT60 is not defined, the estimation returns 0.
TEST_F(EstimateRT60Test, EstimateFromIncreasingEnergyFails) {
  std::vector<float> increasing_energy_impulse_responses(1000, 0.0f);
  for (size_t i = 0; i < 1000; ++i) {
    increasing_energy_impulse_responses[i] = static_cast<float>(i) * 1e-3f;
  }

  // Expect that the estimation function returns 0.
  EXPECT_EQ(0.0f,
            EstimateRT60(increasing_energy_impulse_responses, sampling_rate_));
}

// Tests that if estimating from perfectly-constructed, exponentially-decaying
// impulse responses, then the known RT60 is returned.
TEST_F(EstimateRT60Test, EstimateFromExponentiallyDecayingResponses) {
  std::vector<float> energy_impulse_responses(1000, 0.0f);
  const std::vector<float> expected_RT60s = {0.05f, 0.1f, 0.2f,
                                             0.5f,  1.0f, 2.0f};

  for (const float expected_RT60 : expected_RT60s) {
    // Construct an exponentially decaying reverb tail with a known RT60, whose
    // energy at index i is 10^(6 * i / sampling_rate / RT60).
    for (size_t i = 1; i < energy_impulse_responses.size(); ++i) {
      energy_impulse_responses[i] = std::pow(
          10.0f, -(static_cast<float>(6 * i) / sampling_rate_ / expected_RT60));
    }

    // Expect that the estimated RT60 is close to the expected one.
    EXPECT_NEAR(expected_RT60,
                EstimateRT60(energy_impulse_responses, sampling_rate_), 0.01f);
  }
}

// Tests that RT60s estimated from a cube scene agrees with those computed
// using RoomEffectsUtils::ComputeReverbProperties() (which uses Eyring's
// equation under the hood).
TEST_F(EstimateRT60Test, EstimateFromCubeSceneAgreesWithHeuristics) {
  wall_materials_ = std::vector<MaterialName>{
      MaterialName::kPlasterSmooth,
      MaterialName::kLinoleumOnConcrete,
      MaterialName::kConcreteBlockPainted,
      MaterialName::kGlassThin,
      MaterialName::kBrickBare,
      MaterialName::kPlywoodPanel,
  };

  // Trace rays in a cube scene and estimate RT60s.
  SceneManager scene_manager;
  std::vector<Path> paths = TracePathsInTestcene(
      100 /* min_num_rays */, 100 /* max_depth */,
      1e-12f /* energy_threshold */, cube_center_, cube_vertices_,
      cube_triangles_, cube_wall_triangles_, wall_materials_, &scene_manager);
  std::vector<float> estimated_rt60_values =
      CollectImpulseResponsesAndEstimateRT60(paths, &scene_manager);

  // A default room properties with some fields set to non-default values.
  RoomProperties room_properties;
  room_properties.position[0] = cube_center_[0];
  room_properties.position[1] = cube_center_[1];
  room_properties.position[2] = cube_center_[2];
  room_properties.dimensions[0] = cube_dimensions_[0];
  room_properties.dimensions[1] = cube_dimensions_[1];
  room_properties.dimensions[2] = cube_dimensions_[2];
  for (size_t wall = 0; wall < kNumRoomSurfaces; ++wall) {
    room_properties.material_names[wall] = wall_materials_[wall];
  }

  // Reverb properties computed using heuristics.
  const ReverbProperties reverb_properties =
      ComputeReverbProperties(room_properties);

  // Compare the two sets of RT60s.
  const float rt60_error_tolerance = 0.05f;
  for (size_t band = 0; band < kNumReverbOctaveBands; ++band) {
    const float estimated_rt60 = estimated_rt60_values[band];
    const float expected_rt60 = reverb_properties.rt60_values[band];
    EXPECT_NEAR(estimated_rt60, expected_rt60, rt60_error_tolerance);
  }
}

}  // namespace

}  // namespace vraudio
