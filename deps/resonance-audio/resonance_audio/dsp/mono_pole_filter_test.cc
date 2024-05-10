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

#include "dsp/mono_pole_filter.h"

#include <cmath>

#include "third_party/googletest/googletest/include/gtest/gtest.h"
#include "base/constants_and_types.h"
#include "base/misc_math.h"

namespace vraudio {

namespace {

const float kCoefficient = 0.5f;
const size_t kFramesPerBuffer = 10;

// Tests that the filter correctly implements the difference equation.
TEST(MonoPoleFilterTest, ImpulseResponseTest) {
  MonoPoleFilter filter(kCoefficient);
  AudioBuffer buffer(1U, kFramesPerBuffer);
  buffer.Clear();
  buffer[0][0] = 1.0f;
  EXPECT_TRUE(filter.Filter(buffer[0], &buffer[0]));
  for (size_t i = 0; i < kFramesPerBuffer; ++i) {
    // The impulse response of this filter (for coefficient a), should be:
    // d[n] = (1 - a) * a^n
    const float expected =
        (1.0f - kCoefficient) * IntegerPow(kCoefficient, static_cast<int>(i));
    EXPECT_NEAR(buffer[0][i], expected, kEpsilonFloat);
  }
}

// Tests that no processing is performed when the filter is allpass.
TEST(MonoPoleFilterTest, AllPassNoProcessingTest) {
  MonoPoleFilter filter(0.0f);
  AudioBuffer buffer(1U, kFramesPerBuffer);
  buffer.Clear();
  buffer[0][0] = 1.0f;
  EXPECT_FALSE(filter.Filter(buffer[0], &buffer[0]));
}

}  // namespace

}  // namespace vraudio
