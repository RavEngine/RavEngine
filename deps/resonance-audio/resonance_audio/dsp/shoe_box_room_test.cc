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

#include "dsp/shoe_box_room.h"

#include <algorithm>
#include <vector>

#include "third_party/googletest/googletest/include/gtest/gtest.h"
#include "base/constants_and_types.h"

namespace vraudio {

namespace {

// Expected reflection delay times in seconds.
const float kExpectedDelays[kNumRoomSurfaces] = {
    0.011661f, 0.005830f, 0.016035f, 0.004373f, 0.020408f, 0.002915f};

// Expected reflection magnitudes.
const float kExpectedMagnitudes[kNumRoomSurfaces] = {
    0.25f, 0.5f, 0.181818f, 0.666666f, 0.142857f, 1.0f};

// Tests that a set of single reflections arrive in from the correct directions.
TEST(ShoeBoxRoomTest, SingleReflectionsTest) {
  const WorldPosition kListenerPosition(1.0f, 2.0f, 3.0f);
  const WorldPosition kRoomDimensions(4.0f, 5.0f, 6.0f);

  // Perform the simplified image source method with the wall at the given index
  // having a reflection coefficient of 1 and all of the other walls having a
  // reflection coefficient of 0. Thus we expect one reflection in the output
  // and we should be able to predict its magnitude, delay and direction of
  // arrival in terms of azimuth and elevation.
  float reflection_coefficients[kNumRoomSurfaces];
  std::fill(std::begin(reflection_coefficients),
            std::end(reflection_coefficients), 0.0f);
  std::vector<Reflection> reflections(kNumRoomSurfaces);
  for (size_t index = 0; index < kNumRoomSurfaces; ++index) {
    reflection_coefficients[index] = 1.0f;

    ComputeReflections(kListenerPosition, kRoomDimensions,
                       reflection_coefficients, &reflections);
    EXPECT_EQ(kNumRoomSurfaces, reflections.size());

    // Check that the correct reflection is returned for the given call.
    size_t num_returned = 0;
    for (size_t i = 0; i < kNumRoomSurfaces; ++i) {
      if (reflections[i].magnitude > 0.0f) {
        EXPECT_NEAR(kExpectedDelays[index], reflections[i].delay_time_seconds,
                    kEpsilonFloat);
        EXPECT_NEAR(kExpectedMagnitudes[index], reflections[i].magnitude,
                    kEpsilonFloat);
        ++num_returned;
      }
    }
    EXPECT_EQ(1U, num_returned);

    // Reset so that all reflection reflection_coefficients are 0.0f again.
    reflection_coefficients[index] = 0.0f;
  }
}

// Tests that no reflections arrive when the listener is outside the room.
TEST(ShoeBoxRoomTest, ReflectionsOutsideRoomTest) {
  const WorldPosition kListenerPosition(4.0f, 5.0f, 6.0f);
  const WorldPosition kRoomDimensions(1.0f, 2.0f, 3.0f);
  const float kReflectionCoefficients[] = {1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f};

  std::vector<Reflection> reflections(kNumRoomSurfaces);
  ComputeReflections(kListenerPosition, kRoomDimensions,
                     kReflectionCoefficients, &reflections);
  EXPECT_EQ(kNumRoomSurfaces, reflections.size());

  // Check that all the reflections have zeros.
  for (size_t i = 0; i < kNumRoomSurfaces; ++i) {
    EXPECT_NEAR(0.0f, reflections[i].delay_time_seconds, kEpsilonFloat);
    EXPECT_NEAR(0.0f, reflections[i].magnitude, kEpsilonFloat);
  }
}

}  // namespace

}  // namespace vraudio
