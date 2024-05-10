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

#include "geometrical_acoustics/acoustic_ray.h"

#include "third_party/googletest/googletest/include/gtest/gtest.h"
#include "base/logging.h"
#include "geometrical_acoustics/test_util.h"

namespace vraudio {

namespace {
const float kFloatErrorTolerance = 5e-7f;

class AcousticRayTest : public testing::Test {
 protected:
  void SetUp() override {
    // Use a single RTCDevice for all tests.
    static RTCDevice device = rtcNewDevice(nullptr);
    CHECK_NOTNULL(device);
    scene_ = rtcDeviceNewScene(
        device, RTC_SCENE_STATIC | RTC_SCENE_HIGH_QUALITY, RTC_INTERSECT1);
  }

  void TearDown() override { rtcDeleteScene(scene_); }

  // Normalizes the vector in place.
  void NormalizeVector(float* vector) {
    const float norm = std::sqrt(vector[0] * vector[0] + vector[1] * vector[1] +
                                 vector[2] * vector[2]);
    ASSERT_GT(norm, 0.0f);
    vector[0] /= norm;
    vector[1] /= norm;
    vector[2] /= norm;
  }

  const std::array<float, kNumReverbOctaveBands> kZeroEnergies{
      {0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f}};
  RTCScene scene_ = nullptr;
};

TEST_F(AcousticRayTest, DefaultConstructorTest) {
  AcousticRay ray;
  const float default_origin[3] = {0.0f, 0.0f, 0.0f};
  const float default_direction[3] = {1.0f, 0.0f, 0.0f};
  ExpectFloat3Close(ray.origin(), default_origin);
  ExpectFloat3Close(ray.direction(), default_direction);
  EXPECT_FLOAT_EQ(ray.t_near(), 0.0f);
  EXPECT_FLOAT_EQ(ray.t_far(), AcousticRay::kInfinity);
  for (const float energy : ray.energies()) {
    EXPECT_FLOAT_EQ(energy, 0.0f);
  }
  EXPECT_EQ(ray.type(), AcousticRay::RayType::kSpecular);
  EXPECT_FLOAT_EQ(ray.prior_distance(), 0.0f);
}

TEST_F(AcousticRayTest, AccessorsTest) {
  const float origin[3] = {0.0f, 0.0f, 0.25f};
  const float direction[3] = {0.0f, 0.0f, 1.0f};

  const float t_near = 0.0f;
  const float t_far = AcousticRay::kInfinity;
  const std::array<float, kNumReverbOctaveBands> energies = {
      {1.0f, 2.0f, 3.0f, 4.0f, 5.0f, 6.0f, 7.0f, 8.0f, 9.0f}};
  const AcousticRay::RayType ray_type = AcousticRay::RayType::kDiffuse;
  const float prior_distance = 0.0f;
  AcousticRay ray(origin, direction, t_near, t_far, energies, ray_type,
                  prior_distance);

  // Validate fields passed to the constructor.
  ExpectFloat3Close(ray.origin(), origin);
  ExpectFloat3Close(ray.direction(), direction);
  EXPECT_FLOAT_EQ(ray.t_near(), t_near);
  EXPECT_FLOAT_EQ(ray.t_far(), t_far);
  for (size_t i = 0; i < energies.size(); ++i) {
    EXPECT_FLOAT_EQ(ray.energies().at(i), energies.at(i));
  }
  EXPECT_EQ(ray.type(), ray_type);
  EXPECT_FLOAT_EQ(ray.prior_distance(), prior_distance);

  // Validate explicit setters.
  const float new_origin[3] = {1.0f, 2.0f, 3.0f};
  ray.set_origin(new_origin);
  ExpectFloat3Close(ray.origin(), new_origin);

  const float new_direction[3] = {0.0f, 1.0f, 0.0f};
  ray.set_direction(new_direction);
  ExpectFloat3Close(ray.direction(), new_direction);

  const float new_t_near = 2.0f;
  ray.set_t_near(new_t_near);
  EXPECT_FLOAT_EQ(ray.t_near(), new_t_near);

  const float new_t_far = 4.0f;
  ray.set_t_far(new_t_far);
  EXPECT_FLOAT_EQ(ray.t_far(), new_t_far);

  const float intersected_geometry_normal[3] = {0.0f, 1.0f, 2.0f};
  ray.set_intersected_geometry_normal(intersected_geometry_normal);
  ExpectFloat3Close(ray.intersected_geometry_normal(),
                    intersected_geometry_normal);

  const std::array<float, kNumReverbOctaveBands> new_energies = {
      {2.0f, 4.0f, 6.0f, 8.0f, 10.0f, 12.0f, 14.0f, 16.0f, 18.0f}};
  ray.set_energies(new_energies);
  for (size_t i = 0; i < energies.size(); ++i) {
    EXPECT_FLOAT_EQ(ray.energies().at(i), new_energies.at(i));
  }

  const AcousticRay::RayType new_ray_type = AcousticRay::RayType::kSpecular;
  ray.set_type(new_ray_type);
  EXPECT_EQ(ray.type(), new_ray_type);

  const float new_prior_distance = 7.0f;
  ray.set_prior_distance(new_prior_distance);
  EXPECT_FLOAT_EQ(ray.prior_distance(), new_prior_distance);
}

TEST_F(AcousticRayTest, IntersectTriangleInGroundSceneTest) {
  // Add a ground to the scene and commit.
  AddTestGround(scene_);
  rtcCommit(scene_);

  const float t_near = 0.0f;
  const float t_far = AcousticRay::kInfinity;
  const float prior_distance = 0.0f;
  {
    // This ray should intersect the ground geometry (id: 0) and the first
    // triangle (id: 0).
    const float origin[3] = {0.0f, 0.0f, 0.25f};
    float direction[3] = {1.0f, 1.0f, -1.0f};
    NormalizeVector(direction);
    AcousticRay ray(origin, direction, t_near, t_far, kZeroEnergies,
                    AcousticRay::RayType::kSpecular, prior_distance);
    EXPECT_TRUE(ray.Intersect(scene_));
    EXPECT_EQ(ray.intersected_geometry_id(), 0u);
    EXPECT_EQ(ray.intersected_primitive_id(), 0u);

    // Check the intersection point.
    EXPECT_NEAR(0.4330127f, ray.t_far(), 1e-7);
    float intersection_point[3];
    intersection_point[0] = ray.origin()[0] + ray.t_far() * ray.direction()[0];
    intersection_point[1] = ray.origin()[1] + ray.t_far() * ray.direction()[1];
    intersection_point[2] = ray.origin()[2] + ray.t_far() * ray.direction()[2];

    const float expected_intersection_point[3] = {0.25f, 0.25f, 0.0f};
    ExpectFloat3Close(intersection_point, expected_intersection_point,
                      kFloatErrorTolerance);

    // Normal at the intersection point.
    float normal[3];
    normal[0] = ray.intersected_geometry_normal()[0];
    normal[1] = ray.intersected_geometry_normal()[1];
    normal[2] = ray.intersected_geometry_normal()[2];
    NormalizeVector(normal);
    const float expected_normal[3] = {0.0f, 0.0f, 1.0f};
    ExpectFloat3Close(normal, expected_normal, kFloatErrorTolerance);
  }
  {
    // This ray should intersect the ground geometry (id: 0) and the second
    // triangle (id: 1).
    const float origin[3] = {0.0f, 0.0f, 0.75f};
    const float direction[3] = {1.0f, 1.0f, -1.0f};
    AcousticRay ray(origin, direction, t_near, t_far, kZeroEnergies,
                    AcousticRay::RayType::kSpecular, prior_distance);
    EXPECT_TRUE(ray.Intersect(scene_));
    EXPECT_EQ(ray.intersected_geometry_id(), 0u);
    EXPECT_EQ(ray.intersected_primitive_id(), 1u);
  }
  {
    // This ray shoots upward (away from the ground) and therefore should not
    // intersect anything.
    const float origin[3] = {0.0f, 0.0f, 0.25};
    const float direction[3] = {1.0f, 1.0f, 1.0f};
    AcousticRay ray(origin, direction, t_near, t_far, kZeroEnergies,
                    AcousticRay::RayType::kSpecular, prior_distance);
    EXPECT_FALSE(ray.Intersect(scene_));
  }
}

TEST_F(AcousticRayTest, IntersectNothingBackfaceTest) {
  // Add a ground to the scene and commit.
  AddTestGround(scene_);
  rtcCommit(scene_);

  // This ray is on the "back side" of the ground. So even if the ray passes
  // through a triangle, the intersection should not be reported.
  const float origin[3] = {0.0f, 0.0f, -0.25f};
  const float direction[3] = {1.0f, 1.0f, 1.0f};
  const float t_near = 0.0f;
  const float t_far = AcousticRay::kInfinity;
  const float prior_distance = 0.0f;
  AcousticRay ray(origin, direction, t_near, t_far, kZeroEnergies,
                  AcousticRay::RayType::kSpecular, prior_distance);
  EXPECT_FALSE(ray.Intersect(scene_));
}

TEST_F(AcousticRayTest, IntersectNothingInEmptySceneTest) {
  // Commit without adding any geometry.
  rtcCommit(scene_);

  // This ray should not intersect anything.
  const float origin[3] = {0.0f, 0.0f, 0.25f};
  const float direction[3] = {1.0f, 1.0f, -1.0f};
  const float t_near = 0.0f;
  const float t_far = AcousticRay::kInfinity;
  const float prior_distance = 0.0f;
  AcousticRay ray(origin, direction, t_near, t_far, kZeroEnergies,
                  AcousticRay::RayType::kSpecular, prior_distance);
  EXPECT_FALSE(ray.Intersect(scene_));
}

TEST_F(AcousticRayTest, IntersectNothingInUnCommitedSceneTest) {
  // Add a ground to the scene but forget to commit.
  AddTestGround(scene_);

  // This ray should not intersect anything.
  const float origin[3] = {0.0f, 0.0f, 0.25f};
  const float direction[3] = {1.0f, 1.0f, -1.0f};
  const float t_near = 0.0f;
  const float t_far = AcousticRay::kInfinity;
  const float prior_distance = 0.0f;
  AcousticRay ray(origin, direction, t_near, t_far, kZeroEnergies,
                  AcousticRay::RayType::kSpecular, prior_distance);
  EXPECT_FALSE(ray.Intersect(scene_));
}

}  // namespace

}  // namespace vraudio
