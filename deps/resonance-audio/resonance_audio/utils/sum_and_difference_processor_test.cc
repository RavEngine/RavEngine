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

#include "utils/sum_and_difference_processor.h"

#include "third_party/googletest/googletest/include/gtest/gtest.h"

namespace vraudio {

// Tests Process method.
TEST(SumAndDifferenceProcessor, TestProcessMethod) {
  static const std::vector<std::vector<float>> kTestVector = {
      {0.0f, 1.0f, 2.0f}, {3.0f, 4.0f, 5.0f}};

  AudioBuffer audio_buffer(kTestVector.size(), kTestVector[0].size());
  audio_buffer = kTestVector;

  SumAndDifferenceProcessor processor(audio_buffer.num_frames());
  processor.Process(&audio_buffer);

  for (size_t frame = 0; frame < kTestVector[0].size(); ++frame) {
    EXPECT_EQ(kTestVector[0][frame] + kTestVector[1][frame],
              audio_buffer[0][frame]);
    EXPECT_EQ(kTestVector[0][frame] - kTestVector[1][frame],
              audio_buffer[1][frame]);
  }
}

}  // namespace vraudio
