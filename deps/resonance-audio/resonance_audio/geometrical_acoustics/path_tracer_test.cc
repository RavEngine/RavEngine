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

#include "geometrical_acoustics/path_tracer.h"

#include <memory>
#include <random>
#include <unordered_set>
#include <vector>

#include "third_party/googletest/googletest/include/gtest/gtest.h"
#include "geometrical_acoustics/mesh.h"
#include "geometrical_acoustics/test_util.h"

namespace vraudio {

namespace {

using Eigen::Vector3f;

class PathTracerTest : public testing::Test {
 public:
  PathTracerTest() : random_engine_(), distribution_(0.0f, 1.0f) {
    random_number_generator_ = [this] { return distribution_(random_engine_); };
  }

  void BuildEmptyScene() {
    scene_manager_.reset(new SceneManager);
    scene_manager_->BuildScene(std::vector<Vertex>(), std::vector<Triangle>());
    all_triangle_indices_.clear();
  }

  void BuildGroundScene() {
    scene_manager_.reset(new SceneManager);
    std::vector<Vertex> ground_vertices{{0.0f, 0.0f, 0.0f},
                                        {0.0f, 1.0f, 0.0f},
                                        {1.0f, 0.0f, 0.0f},
                                        {1.0f, 1.0f, 0.0f}};
    std::vector<Triangle> ground_triangles{{0, 3, 1}, {0, 2, 3}};
    scene_manager_->BuildScene(ground_vertices, ground_triangles);
    all_triangle_indices_ = {0, 1};
  }

  void BuildBoxScene() {
    scene_manager_.reset(new SceneManager);
    std::vector<Vertex> box_vertices;
    std::vector<Triangle> box_triangles;
    BuildTestBoxScene({0.0f, 0.0f, 0.0f}, {1.0f, 1.0f, 1.0f}, &box_vertices,
                      &box_triangles, nullptr);
    scene_manager_->BuildScene(box_vertices, box_triangles);
    all_triangle_indices_.clear();
    for (unsigned int i = 0; i < 12; ++i) {
      all_triangle_indices_.insert(i);
    }
  }

 protected:
  const std::array<float, kNumReverbOctaveBands> kZeroAbsorptionCoefficients = {
      {0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f}};
  std::unique_ptr<SceneManager> scene_manager_;
  std::unordered_set<unsigned int> all_triangle_indices_;
  std::default_random_engine random_engine_;
  std::uniform_real_distribution<float> distribution_;
  std::function<float()> random_number_generator_;
};

TEST_F(PathTracerTest, AllPathsHaveLengthOneInEmptySceneTest) {
  BuildEmptyScene();
  PathTracer path_tracer(*scene_manager_);

  AcousticSource source({0.0f, 0.0f, 0.0f}, kUnitEnergies,
                        random_number_generator_);
  const size_t min_num_rays = 100;
  const size_t max_depth = 10;
  const float energy_thresold = 1e-6f;
  std::vector<Path> paths =
      path_tracer.TracePaths(source, min_num_rays, max_depth, energy_thresold);

  EXPECT_LE(paths.size(), min_num_rays);
  for (size_t i = 0; i < paths.size(); ++i) {
    EXPECT_EQ(static_cast<int>(paths[i].rays.size()), 1);
  }
}

TEST_F(PathTracerTest, GroundSceneTest) {
  BuildGroundScene();

  // Associate a perfect reflection to all triangles.
  ReflectionKernel reflection_kernel(kZeroAbsorptionCoefficients, 0.0f,
                                     random_number_generator_);
  scene_manager_->AssociateReflectionKernelToTriangles(reflection_kernel,
                                                       all_triangle_indices_);

  PathTracer path_tracer(*scene_manager_);

  AcousticSource source({0.5f, 0.5f, 0.1f}, kUnitEnergies,
                        random_number_generator_);
  const size_t min_num_rays = 100;
  const size_t max_depth = 10;
  const float energy_thresold = 1e-6f;
  std::vector<Path> paths =
      path_tracer.TracePaths(source, min_num_rays, max_depth, energy_thresold);

  // All paths have at least one ray (for those that did not hit the ground)
  // and at most two rays (for those hitting the ground).
  for (size_t i = 0; i < paths.size(); ++i) {
    EXPECT_GE(static_cast<int>(paths[i].rays.size()), 1);
    EXPECT_LE(static_cast<int>(paths[i].rays.size()), 2);

    // For those with length = 2, validate that the first ray ends on the
    // ground plane.
    if (paths[i].rays.size() == 2) {
      const AcousticRay& first_ray = paths[i].rays.front();
      const Vector3f& end_point =
          Vector3f(first_ray.origin()) +
          first_ray.t_far() * Vector3f(first_ray.direction());
      EXPECT_NEAR(end_point.z(), 0.0f, 1e-7f);
    }
  }
}

TEST_F(PathTracerTest, TerminateAtMaxDepthTest) {
  // A box scene within which rays never escape.
  BuildBoxScene();

  // Associate a perfect reflection to all triangles. Theoretically there should
  // be infinite reflections, but the path tracer stops once each path reaches
  // length |max_depth|.
  ReflectionKernel reflection_kernel(kZeroAbsorptionCoefficients, 0.0f,
                                     random_number_generator_);
  scene_manager_->AssociateReflectionKernelToTriangles(reflection_kernel,
                                                       all_triangle_indices_);
  PathTracer path_tracer(*scene_manager_);

  AcousticSource source({0.5f, 0.5f, 0.5f}, kUnitEnergies,
                        random_number_generator_);
  const size_t min_num_rays = 100;
  const size_t max_depth = 10;
  const float energy_thresold = 1e-6f;
  std::vector<Path> paths =
      path_tracer.TracePaths(source, min_num_rays, max_depth, energy_thresold);

  for (size_t i = 0; i < paths.size(); ++i) {
    EXPECT_EQ(paths[i].rays.size(), max_depth);
  }
}

TEST_F(PathTracerTest, TerminateWhenEnergyBelowThresholdTest) {
  // A box scene within which rays never escape.
  BuildBoxScene();

  // Associate an absorptive reflection with an absorption coefficient slightly
  // over 90% to all triangles.
  std::array<float, kNumReverbOctaveBands> absorption_coefficients;
  absorption_coefficients.fill(0.9001f);
  ReflectionKernel reflection_kernel(absorption_coefficients, 0.0f,
                                     random_number_generator_);
  scene_manager_->AssociateReflectionKernelToTriangles(reflection_kernel,
                                                       all_triangle_indices_);
  PathTracer path_tracer(*scene_manager_);

  // Since the reflected energy is slightly below 1e-1 after each reflection,
  // we expect the path tracer to stop after X reflections for an
  // energy threshold of 1e-X, and that each path is of length X.
  AcousticSource source({0.5f, 0.5f, 0.5f}, kUnitEnergies,
                        random_number_generator_);
  const size_t min_num_rays = 100;
  const size_t max_depth = 10;
  const float energy_thresold = 1e-6f;
  std::vector<Path> paths =
      path_tracer.TracePaths(source, min_num_rays, max_depth, energy_thresold);

  for (size_t i = 0; i < paths.size(); ++i) {
    EXPECT_EQ(static_cast<int>(paths[i].rays.size()), 6);
  }
}

}  // namespace

}  // namespace vraudio
