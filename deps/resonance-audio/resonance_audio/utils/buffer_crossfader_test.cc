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

#include "utils/buffer_crossfader.h"

#include "third_party/googletest/googletest/include/gtest/gtest.h"
#include "base/audio_buffer.h"
#include "base/constants_and_types.h"
#include "base/logging.h"

namespace vraudio {

namespace {

// Tests that the linear crossfade is correctly applied to an input pair.
TEST(BufferCrossfaderTest, ApplyLinearCrossfadeTest) {
  const size_t kNumFrames = 5;
  const size_t kTestValue = 1.0f;
  // Initialize input buffers.
  AudioBuffer input_fade_in(kNumMonoChannels, kNumFrames);
  AudioBuffer input_fade_out(kNumMonoChannels, kNumFrames);
  for (size_t i = 0; i < kNumFrames; ++i) {
    input_fade_in[0][i] = kTestValue;
    input_fade_out[0][i] = kTestValue;
  }
  // Initialize output buffer.
  AudioBuffer output(kNumMonoChannels, kNumFrames);
  output.Clear();
  // Initialize a new crossfader and apply linear crossfade.
  BufferCrossfader crossfader(kNumFrames);
  crossfader.ApplyLinearCrossfade(input_fade_in, input_fade_out, &output);
  // Verify that the output buffer is correctly filled in as expected.
  for (size_t i = 0; i < kNumFrames; ++i) {
    EXPECT_FLOAT_EQ(kTestValue, output[0][i]);
  }
}

}  // namespace

}  // namespace vraudio
