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

#include "base/spherical_angle.h"

#include "third_party/googletest/googletest/include/gtest/gtest.h"
#include "base/constants_and_types.h"

namespace vraudio {

namespace {

// Spherical angle to be used in the rotation tests.
const float kAzimuth = 0.0f;
const float kElevation = 0.0f;
const SphericalAngle kSphericalAngle(0.0f, 0.0f);

// Arbitrary rotation angle to be used in the rotation tests.
const float kRotationAngle = 10.0f * kRadiansFromDegrees;

// Tests that the GetWorldPositionOnUnitSphere() and FromWorldPosition()
// functions act as perfect inverses of one another for angles defined on the
// unit sphere (in this case the vraudio cube speaker layout).
TEST(SphericalAngleTest, CartesianToSphericalAndBackTest) {
  // Azimuth and elevation angles of the cubic spherical loudspeaker array.
  const std::vector<SphericalAngle> kCubeAngles = {
      SphericalAngle::FromDegrees(45.0f, 35.26f),
      SphericalAngle::FromDegrees(-45.0f, 35.26f),
      SphericalAngle::FromDegrees(-135.0f, 35.26f),
      SphericalAngle::FromDegrees(135.0f, 35.26f),
      SphericalAngle::FromDegrees(45.0f, -35.26f),
      SphericalAngle::FromDegrees(-45.0f, -35.26f),
      SphericalAngle::FromDegrees(-135.0f, -35.26f),
      SphericalAngle::FromDegrees(135.0f, -35.26f)};

  for (size_t i = 0; i < kCubeAngles.size(); ++i) {
    const WorldPosition position =
        kCubeAngles[i].GetWorldPositionOnUnitSphere();
    const SphericalAngle angle = SphericalAngle::FromWorldPosition(position);
    EXPECT_EQ(kCubeAngles[i].azimuth(), angle.azimuth());
    EXPECT_EQ(kCubeAngles[i].elevation(), angle.elevation());
  }
}

// Tests the horizontal angle flip across the median plane.
TEST(SphericalAngleTest, FlipTest) {
  const std::vector<SphericalAngle> kTestAngles = {
      SphericalAngle::FromDegrees(45.0f, 35.26f),
      SphericalAngle::FromDegrees(-15.0f, -10.0f)};

  for (size_t i = 0; i < kTestAngles.size(); ++i) {
    SphericalAngle flipped_spherical_angle = kTestAngles[i].FlipAzimuth();

    // Check if the flipped spherical anglee is correct.
    EXPECT_NEAR(kTestAngles[i].azimuth(), -flipped_spherical_angle.azimuth(),
                kEpsilonFloat);
    EXPECT_NEAR((kTestAngles[i].elevation()),
                flipped_spherical_angle.elevation(), kEpsilonFloat);
  }
}

// Tests that the Rotate() function correctly rotates the spherical angle
// against the x axis (right facing).
TEST(SphericalAngleTest, RotateXTest) {
  const WorldPosition kAxis = {1.0f, 0.0f, 0.0f};
  const WorldRotation kRotation(AngleAxisf(kRotationAngle, kAxis));
  // Rotate against the x axis (right facing).

  const SphericalAngle kXrotatedSphericalAngle =
      kSphericalAngle.Rotate(kRotation);

  // Check if the rotated spherical angle is correct.
  EXPECT_NEAR(kAzimuth, kXrotatedSphericalAngle.azimuth(), kEpsilonFloat);
  EXPECT_NEAR((kElevation + kRotationAngle),
              kXrotatedSphericalAngle.elevation(), kEpsilonFloat);
}

// Tests that the Rotate() function correctly rotates the spherical angle
// against the y axis (upward facing).
TEST(SphericalAngleTest, RotateYTest) {
  const WorldPosition kAxis(0.0f, 1.0f, 0.0f);
  const WorldRotation kRotation(AngleAxisf(kRotationAngle, kAxis));
  // Rotate against the y axis (upward facing).
  const SphericalAngle kYrotatedSphericalAngle =
      kSphericalAngle.Rotate(kRotation);

  // Check if the rotated spherical angle is correct.
  EXPECT_NEAR((kAzimuth + kRotationAngle), kYrotatedSphericalAngle.azimuth(),
              kEpsilonFloat);
  EXPECT_NEAR(kElevation, kYrotatedSphericalAngle.elevation(), kEpsilonFloat);
}

// Tests that the Rotate() function correctly rotates the spherical angle
// against the Z axis (forward facing).
TEST(SphericalAngleTest, RotateZTest) {
  const WorldPosition kAxis = {0.0f, 0.0f, 1.0f};
  const WorldRotation kRotation(AngleAxisf(kRotationAngle, kAxis));
  // Rotate against the z axis (forward facing).
  const SphericalAngle kZrotatedSphericalAngle =
      kSphericalAngle.Rotate(kRotation);

  // Check if the rotated spherical angle is correct.
  EXPECT_NEAR(kAzimuth, kZrotatedSphericalAngle.azimuth(), kEpsilonFloat);
  EXPECT_NEAR(kElevation, kZrotatedSphericalAngle.elevation(), kEpsilonFloat);
}

}  // namespace

}  // namespace vraudio
