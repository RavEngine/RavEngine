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

#include "geometrical_acoustics/sphere.h"

#include <cmath>

#include "third_party/googletest/googletest/include/gtest/gtest.h"
#include "base/aligned_allocator.h"
#include "base/logging.h"
#include "geometrical_acoustics/acoustic_ray.h"
#include "geometrical_acoustics/test_util.h"

namespace vraudio {

namespace {

// A function adapter from SphereBounds() to RTCBoundsFunc in order to be
// passed to rtcSetBoundsFunction().
// The signature of RTCBoundsFunc does not comply with Google's C++ style,

static void EmbreeSphereBoundsFunction(void* user_data, size_t index,
                                       RTCBounds& output_bounds
                                       ) {
  Sphere* spheres = static_cast<Sphere*>(user_data);
  const Sphere& sphere = spheres[index];
  SphereBounds(sphere, &output_bounds);
}

// A function adapter from SphereIntersections() to RTCIntersectFunc in order
// to be passed to rtcSetIntersectFunction().
// The signature of RTCIntersectFunc does not comply with Google's C++ style,

static void EmbreeSphereIntersectFunction(void* user_data,
                                          RTCRay& ray,
                                          size_t index) {
  Sphere* spheres = static_cast<Sphere*>(user_data);
  const Sphere& sphere = spheres[index];
  SphereIntersection(sphere, &ray);
}

void SetSphereData(const float center[3], float radius,
                   unsigned int geometry_id, Sphere* sphere) {
  sphere->center[0] = center[0];
  sphere->center[1] = center[1];
  sphere->center[2] = center[2];
  sphere->radius = radius;
  sphere->geometry_id = geometry_id;
}

TEST(Sphere, SphereBoundsTest) {
  const float center[3] = {1.0f, 2.0f, 3.0f};
  const float radius = 3.0f;
  Sphere sphere;
  SetSphereData(center, radius, 0u, &sphere);

  RTCBounds bounds;
  SphereBounds(sphere, &bounds);
  EXPECT_FLOAT_EQ(bounds.lower_x, -2.0f);
  EXPECT_FLOAT_EQ(bounds.lower_y, -1.0f);
  EXPECT_FLOAT_EQ(bounds.lower_z, 0.0f);
  EXPECT_FLOAT_EQ(bounds.upper_x, 4.0f);
  EXPECT_FLOAT_EQ(bounds.upper_y, 5.0f);
  EXPECT_FLOAT_EQ(bounds.upper_z, 6.0f);
}

TEST(Sphere, RayIntersectSphereTest) {
  const float center[3] = {0.0f, 0.0f, 0.0f};
  const float radius = 1.0f;
  const unsigned sphere_geometry_id = 1u;
  Sphere sphere;
  SetSphereData(center, radius, sphere_geometry_id, &sphere);

  // This ray passes through the sphere.
  const float origin[3] = {0.5f, 0.0f, -2.0f};
  const float direction[3] = {0.0f, 0.0f, 1.0f};
  AcousticRay ray(origin, direction, 0.0f, AcousticRay::kInfinity, {},
                  AcousticRay::RayType::kSpecular, 0.0f);

  SphereIntersection(sphere, &ray);

  // Expect an intersection, so that ray.intersected_geometry_id() returns
  // |sphere_geometry_id|.
  EXPECT_EQ(ray.intersected_geometry_id(), sphere_geometry_id);

  // Expect that the intersection point is at (0.5, 0, -sqrt(3)/2).
  const float expected_intersection_point[3] = {0.5f, 0.0f,
                                                -0.5f * kSqrtThree};
  float intersection_point[3];
  intersection_point[0] = ray.origin()[0] + ray.t_far() * ray.direction()[0];
  intersection_point[1] = ray.origin()[1] + ray.t_far() * ray.direction()[1];
  intersection_point[2] = ray.origin()[2] + ray.t_far() * ray.direction()[2];
  ExpectFloat3Close(intersection_point, expected_intersection_point);

  // Expect that the normal at the intersection point is (0.5, 0, -sqrt(3)/2).
  const float expected_normal[3] = {0.5f, 0.0f, -0.5f * kSqrtThree};
  ExpectFloat3Close(ray.intersected_geometry_normal(), expected_normal);
}

TEST(Sphere, RayOutsideSphereNotIntersectTest) {
  const float center[3] = {0.0f, 0.0f, 0.0f};
  const float radius = 1.0f;
  Sphere sphere;
  SetSphereData(center, radius, 1u, &sphere);

  // This ray completely lies outside of the sphere.
  const float origin[3] = {2.0f, 2.0f, 2.0f};
  const float direction[3] = {0.0f, 0.0f, 1.0f};
  AcousticRay ray(origin, direction, 0.0f, AcousticRay::kInfinity, {},
                  AcousticRay::RayType::kSpecular, 0.0f);

  SphereIntersection(sphere, &ray);

  // Expect no intersection, i.e. so that ray.intersected_geometry_id() returns
  // RTC_INVALID_GEOMETRY_ID.
  EXPECT_EQ(ray.intersected_geometry_id(), RTC_INVALID_GEOMETRY_ID);
}

TEST(Sphere, RayStartingInsideSphereNotIntersectTest) {
  const float center[3] = {0.0f, 0.0f, 0.0f};
  const float radius = 1.0f;
  Sphere sphere;
  SetSphereData(center, radius, 1u, &sphere);

  // This ray theoretically intersects with the sphere, but as its starting
  // point is inside the sphere, our intersection test regards it as
  // non-intersecting.
  const float origin[3] = {0.5f, 0.5f, 0.0f};
  const float direction[3] = {0.0f, 0.0f, 1.0f};
  AcousticRay ray(origin, direction, 0.0f, AcousticRay::kInfinity, {},
                  AcousticRay::RayType::kSpecular, 0.0f);

  SphereIntersection(sphere, &ray);

  // Expect no intersection, i.e. so that ray.intersected_geometry_id() returns
  // RTC_INVALID_GEOMETRY_ID.
  EXPECT_EQ(ray.intersected_geometry_id(), RTC_INVALID_GEOMETRY_ID);
}

TEST(Sphere, RayEndingInsideSphereNotIntersectTest) {
  const float center[3] = {0.0f, 0.0f, 0.0f};
  const float radius = 1.0f;
  Sphere sphere;
  SetSphereData(center, radius, 1u, &sphere);

  // This ray theoretically intersects with the sphere, but as its end point
  // (controlled by |ray.tfar| is inside the sphere, our intersection test
  // regards it as non-intersecting.
  const float origin[3] = {0.5f, 0.0f, -2.0f};
  const float direction[3] = {0.0f, 0.0f, 1.0f};
  AcousticRay ray(origin, direction, 0.0f, 2.0f, {},
                  AcousticRay::RayType::kSpecular, 0.0f);

  SphereIntersection(sphere, &ray);

  // Expect no intersection, i.e. so that ray.intersected_geometry_id() returns
  // RTC_INVALID_GEOMETRY_ID.
  EXPECT_EQ(ray.intersected_geometry_id(), RTC_INVALID_GEOMETRY_ID);
}

// The following test fixture and tests mimic real use cases of ray-sphere
// intersections in our acoustics computation:
//
// 1) A sphere is constructed.
// 2) The sphere is registered to the scene as user data. Two callback
//    functions of types RTCBoundsFunc and RTCIntersectFunc are registered
//    to the scene as well.
// 2) An AcousticRay is constructed.
// 3) The intersection is found by AcousticRay.Intersect().
//
// This is different from previous tests in that we do not directly invoke
// SphereBounds() and SphereIntersection(); Embree does it for us.

class SphereTest : public testing::Test {
 protected:
  void SetUp() override {
    // Use a single RTCDevice for all tests.
    static RTCDevice device = rtcNewDevice(nullptr);
    CHECK_NOTNULL(device);
    scene_ = rtcDeviceNewScene(
        device, RTC_SCENE_STATIC | RTC_SCENE_HIGH_QUALITY, RTC_INTERSECT1);
  }

  void TearDown() override { rtcDeleteScene(scene_); }

  unsigned int AddSphereToScene() {
    const unsigned int geometry_id = rtcNewUserGeometry(scene_, 1);

    const float center[3] = {0.0f, 0.0f, 0.0f};
    const float radius = 1.0f;
    Sphere* const sphere =
        AllignedMalloc<Sphere, size_t, Sphere*>(sizeof(Sphere),
                                                /*alignment=*/64);

    SetSphereData(center, radius, geometry_id, sphere);

    // rtcSetUserData() takes ownership of |sphere|.
    rtcSetUserData(scene_, geometry_id, sphere);
    rtcSetBoundsFunction(scene_, geometry_id, &EmbreeSphereBoundsFunction);
    rtcSetIntersectFunction(scene_, geometry_id,
                            &EmbreeSphereIntersectFunction);
    return geometry_id;
  }

  const std::array<float, kNumReverbOctaveBands> kUnitEnergies{
      {1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f}};
  RTCScene scene_ = nullptr;
};

TEST_F(SphereTest, AcousticRayIntersectSphereTest) {
  const unsigned int sphere_geometry_id = AddSphereToScene();
  rtcCommit(scene_);

  // Construct an AcousticRay that passes through the sphere.
  const float origin[3] = {0.5f, 0.0f, -2.0f};
  const float direction[3] = {0.0f, 0.0f, 1.0f};
  AcousticRay ray(origin, direction, 0.0f, AcousticRay::kInfinity,
                  kUnitEnergies, AcousticRay::RayType::kSpecular, 0.0f);

  // Expect intersection happens and the intersected geometry id set to
  // |sphere_geometry_id|.
  EXPECT_TRUE(ray.Intersect(scene_));
  EXPECT_EQ(ray.intersected_geometry_id(), sphere_geometry_id);

  // Expect that the intersection point is at (0.5, 0, -sqrt(3)/2).
  const float expected_intersection_point[3] = {0.5f, 0.0f,
                                                -0.5f * kSqrtThree};
  float intersection_point[3];
  intersection_point[0] = ray.origin()[0] + ray.t_far() * ray.direction()[0];
  intersection_point[1] = ray.origin()[1] + ray.t_far() * ray.direction()[1];
  intersection_point[2] = ray.origin()[2] + ray.t_far() * ray.direction()[2];
  ExpectFloat3Close(intersection_point, expected_intersection_point);

  // Expect that the normal at the intersection point is (0.5, 0, -sqrt(3)/2).
  const float expected_normal[3] = {0.5f, 0.0f, -0.5f * kSqrtThree};
  ExpectFloat3Close(ray.intersected_geometry_normal(), expected_normal);
}

TEST_F(SphereTest, AcousticRayIntersectNothingTest) {
  rtcCommit(scene_);

  // Construct an AcousticRay completely outside of the sphere.
  const float origin[3] = {2.0f, 2.0f, 2.0f};
  const float direction[3] = {0.0f, 0.0f, 1.0f};
  AcousticRay ray(origin, direction, 0.0f, AcousticRay::kInfinity,
                  kUnitEnergies, AcousticRay::RayType::kSpecular, 0.0f);

  // Expect no intersection.
  EXPECT_FALSE(ray.Intersect(scene_));
}

}  // namespace

}  // namespace vraudio
