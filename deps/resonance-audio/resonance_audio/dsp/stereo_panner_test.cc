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

#include "dsp/stereo_panner.h"

#include <cmath>
#include <vector>

#include "third_party/googletest/googletest/include/gtest/gtest.h"
#include "base/constants_and_types.h"

namespace vraudio {

namespace {

const float kMinusThreeDecibels = kInverseSqrtTwo;

// Tests that the |CalculateStereoPanGains| method will generate correct stereo
// pan gains.
TEST(StereoPannerTest, StereoTest) {
  std::vector<float> speaker_gains;

  SphericalAngle source_direction = SphericalAngle::FromDegrees(-90.0f, 0.0f);
  CalculateStereoPanGains(source_direction, &speaker_gains);
  EXPECT_NEAR(0.0f, speaker_gains[0], kEpsilonFloat);
  EXPECT_NEAR(1.0f, speaker_gains[1], kEpsilonFloat);

  source_direction = SphericalAngle::FromDegrees(-45.0f, 0.0f);
  CalculateStereoPanGains(source_direction, &speaker_gains);
  EXPECT_NEAR(kMinusThreeDecibels, speaker_gains[1] - speaker_gains[0],
              kEpsilonFloat);

  source_direction = SphericalAngle::FromDegrees(0.0f, 0.0f);
  CalculateStereoPanGains(source_direction, &speaker_gains);
  EXPECT_NEAR(speaker_gains[0], speaker_gains[1], kEpsilonFloat);

  source_direction = SphericalAngle::FromDegrees(45.0f, 0.0f);
  CalculateStereoPanGains(source_direction, &speaker_gains);
  EXPECT_NEAR(kMinusThreeDecibels, speaker_gains[0] - speaker_gains[1],
              kEpsilonFloat);

  source_direction = SphericalAngle::FromDegrees(90.0f, 0.0f);
  CalculateStereoPanGains(source_direction, &speaker_gains);
  EXPECT_NEAR(1.0f, speaker_gains[0], kEpsilonFloat);
  EXPECT_NEAR(0.0f, speaker_gains[1], kEpsilonFloat);

  source_direction = SphericalAngle::FromDegrees(0.0f, 45.0f);
  CalculateStereoPanGains(source_direction, &speaker_gains);
  EXPECT_NEAR(0.5f, speaker_gains[0], kEpsilonFloat);
  EXPECT_NEAR(0.5f, speaker_gains[1], kEpsilonFloat);

  source_direction = SphericalAngle::FromDegrees(0.0f, -60.0f);
  CalculateStereoPanGains(source_direction, &speaker_gains);
  EXPECT_NEAR(0.5f, speaker_gains[0], kEpsilonFloat);
  EXPECT_NEAR(0.5f, speaker_gains[1], kEpsilonFloat);

  source_direction = SphericalAngle::FromDegrees(0.0f, 90.0f);
  CalculateStereoPanGains(source_direction, &speaker_gains);
  EXPECT_NEAR(0.5f, speaker_gains[0], kEpsilonFloat);
  EXPECT_NEAR(0.5f, speaker_gains[1], kEpsilonFloat);

  source_direction = SphericalAngle::FromDegrees(0.0f, -90.0f);
  CalculateStereoPanGains(source_direction, &speaker_gains);
  EXPECT_NEAR(0.5f, speaker_gains[0], kEpsilonFloat);
  EXPECT_NEAR(0.5f, speaker_gains[1], kEpsilonFloat);

  source_direction = SphericalAngle::FromDegrees(45.0f, 45.0f);
  CalculateStereoPanGains(source_direction, &speaker_gains);
  EXPECT_NEAR(0.75f, speaker_gains[0], kEpsilonFloat);
  EXPECT_NEAR(0.25f, speaker_gains[1], kEpsilonFloat);
}

}  // namespace

}  // namespace vraudio
