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

#include "geometrical_acoustics/scene_manager.h"

#include <functional>
#include <random>
#include <vector>

#include "third_party/googletest/googletest/include/gtest/gtest.h"
#include "Eigen/Core"
#include "geometrical_acoustics/acoustic_listener.h"
#include "geometrical_acoustics/acoustic_ray.h"
#include "geometrical_acoustics/reflection_kernel.h"
#include "geometrical_acoustics/test_util.h"

namespace vraudio {

namespace {

using Eigen::Vector3f;

class SceneManagerTest : public testing::Test {
 public:
  SceneManagerTest() : random_engine_(0), distribution_(0.0f, 1.0f) {}

  void SetUp() override {
    ground_vertices_ = {{0.0f, 0.0f, 0.0f},
                        {0.0f, 1.0f, 0.0f},
                        {1.0f, 0.0f, 0.0f},
                        {1.0f, 1.0f, 0.0f}};
    ground_triangles_ = {{0, 2, 1}, {1, 2, 3}};

    // Reseed the random number generator for every test to be deterministic
    // and remove flakiness in tests.
    random_engine_.seed(0);
    random_number_generator_ = [this] { return distribution_(random_engine_); };
  }

 protected:
  const std::array<float, kNumReverbOctaveBands> kUnitEnergies{
      {1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f}};
  const std::array<float, kNumReverbOctaveBands> kZeroAbsorptionCoefficients{
      {0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f}};
  std::vector<Vertex> ground_vertices_;
  std::vector<Triangle> ground_triangles_;
  SceneManager scene_manager_;
  std::default_random_engine random_engine_;
  std::uniform_real_distribution<float> distribution_;
  std::function<float()> random_number_generator_;
};

TEST_F(SceneManagerTest, UncommittedSceneTest) {
  EXPECT_FALSE(scene_manager_.is_scene_committed());
  EXPECT_FALSE(scene_manager_.is_listener_scene_committed());
}

TEST_F(SceneManagerTest, BuildEmptySceneTest) {
  scene_manager_.BuildScene({}, {});
  EXPECT_TRUE(scene_manager_.is_scene_committed());
  EXPECT_EQ(scene_manager_.num_vertices(), 0U);
  EXPECT_EQ(scene_manager_.num_triangles(), 0U);
}

TEST_F(SceneManagerTest, BuildSceneWithTwoTrianglesTest) {
  scene_manager_.BuildScene(ground_vertices_, ground_triangles_);
  EXPECT_TRUE(scene_manager_.is_scene_committed());
  EXPECT_EQ(scene_manager_.num_vertices(), 4U);
  EXPECT_EQ(scene_manager_.num_triangles(), 2U);
}

TEST_F(SceneManagerTest, ReturnDefaultReflectionIfNotAssociatedTest) {
  scene_manager_.BuildScene(ground_vertices_, ground_triangles_);

  // Get a reflection of triangle 0; since nothing has been associated to it,
  // a default reflection is returned.
  const ReflectionKernel& reflection =
      scene_manager_.GetAssociatedReflectionKernel(0);

  // Test that this default reflection absorbs all energy.
  AcousticRay ray(Vector3f(0.0f, 0.0f, 0.0f).data(),
                  Vector3f(1.0f, 0.0f, 0.0f).data(), 0.0f, 1.0f, kUnitEnergies,
                  AcousticRay::RayType::kSpecular, 0.0f);
  AcousticRay reflected_ray = reflection.Reflect(ray);
  for (size_t i = 0; i < kNumReverbOctaveBands; ++i) {
    EXPECT_FLOAT_EQ(reflected_ray.energies().at(i), 0.0f);
  }
}

TEST_F(SceneManagerTest, ReturnFalseAssociatingToNonExistentTriangleTest) {
  scene_manager_.BuildScene(ground_vertices_, ground_triangles_);

  // A non-absorptive, purely specular reflection.
  ReflectionKernel reflection(kZeroAbsorptionCoefficients, 0.0f,
                              random_number_generator_);

  // Associate the reflection to triangle 2, which does not exist.
  EXPECT_FALSE(
      scene_manager_.AssociateReflectionKernelToTriangles(reflection, {2}));
}

TEST_F(SceneManagerTest, ReturnAssociatedReflectionsTest) {
  scene_manager_.BuildScene(ground_vertices_, ground_triangles_);

  // A half-absorptive, purely diffuse reflection.
  std::array<float, kNumReverbOctaveBands> absorption_coefficients;
  absorption_coefficients.fill(0.5f);
  ReflectionKernel reflection_1(absorption_coefficients, 1.0f,
                                random_number_generator_);

  // A non-absorptive, purely specular reflection.
  ReflectionKernel reflection_2(kZeroAbsorptionCoefficients, 0.0f,
                                random_number_generator_);

  // Associate them to triangle 1 and 0 respectively.
  EXPECT_TRUE(
      scene_manager_.AssociateReflectionKernelToTriangles(reflection_1, {1}));
  EXPECT_TRUE(
      scene_manager_.AssociateReflectionKernelToTriangles(reflection_2, {0}));

  AcousticRay ray(Vector3f(0.0f, 0.0f, 0.0f).data(),
                  Vector3f(1.0f, 0.0f, 0.0f).data(), 0.0f, 1.0f, kUnitEnergies,
                  AcousticRay::RayType::kSpecular, 0.0f);
  {
    // Test that the reflection from triangle 0, which should be |reflection_2|,
    // absorbs none of the energy and reflects specularly.

    const ReflectionKernel& reflection =
        scene_manager_.GetAssociatedReflectionKernel(0);
    AcousticRay reflected_ray = reflection.Reflect(ray);
    for (size_t i = 0; i < kNumReverbOctaveBands; ++i) {
      EXPECT_FLOAT_EQ(reflected_ray.energies().at(i), 1.0f);
    }
    EXPECT_EQ(reflected_ray.type(), AcousticRay::RayType::kSpecular);
  }
  {
    // Test that the reflection from triangle 1, which should be |reflection_1|,
    // absorbs half of the energy and reflects diffusely.
    const ReflectionKernel& reflection =
        scene_manager_.GetAssociatedReflectionKernel(1);
    AcousticRay reflected_ray = reflection.Reflect(ray);
    for (size_t i = 0; i < kNumReverbOctaveBands; ++i) {
      EXPECT_FLOAT_EQ(reflected_ray.energies().at(i), 0.5f);
    }
    EXPECT_EQ(reflected_ray.type(), AcousticRay::RayType::kDiffuse);
  }
}

TEST_F(SceneManagerTest, BuildEmptyListenerSceneTest) {
  const float listener_sphere_radius = 0.1f;
  scene_manager_.BuildListenerScene({}, listener_sphere_radius);
  EXPECT_TRUE(scene_manager_.is_listener_scene_committed());
}

TEST_F(SceneManagerTest, RayIntersectListenerSceneTest) {
  // Five AcousticListeners, lining up on the x-axis, with positions (i, 0, 0).
  std::vector<AcousticListener> listeners;
  const size_t impulse_response_length = 1000;
  for (size_t i = 0; i < 5; ++i) {
    const Vector3f listener_position(static_cast<float>(i), 0.0f, 0.0f);
    listeners.emplace_back(listener_position, impulse_response_length);
  }

  // Build the scene.
  const float listener_sphere_radius = 0.1f;
  scene_manager_.BuildListenerScene(listeners, listener_sphere_radius);
  EXPECT_TRUE(scene_manager_.is_listener_scene_committed());

  // Shoot 5 rays from (i, -1, 0), all with direction (0, 1, 0). Ray i is
  // expected to intersect with listener i's sphere.
  const float direction[3] = {0.0f, 1.0f, 0.0f};
  for (size_t i = 0; i < 5; ++i) {
    const float origin[3] = {static_cast<float>(i), -1.0f, 0.0f};
    AcousticRay ray_i(origin, direction, 0.0f /* t_near */,
                      AcousticRay::kInfinity /* t_far */, kUnitEnergies,
                      AcousticRay::RayType::kDiffuse,
                      0.0f /* prior_distance */);

    // Verify that |ray_i| intersects with listener i's sphere.
    EXPECT_TRUE(ray_i.Intersect(scene_manager_.listener_scene()));
    const unsigned int intersected_sphere_id = ray_i.intersected_geometry_id();
    EXPECT_EQ(
        scene_manager_.GetListenerIndexFromSphereId(intersected_sphere_id), i);
  }
}

}  // namespace

}  // namespace vraudio
