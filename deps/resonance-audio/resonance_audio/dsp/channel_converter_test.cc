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

#include "dsp/channel_converter.h"

#include <vector>

#include "third_party/googletest/googletest/include/gtest/gtest.h"
#include "base/constants_and_types.h"

namespace vraudio {

namespace {

// Number of frames for test buffers.
const size_t kNumFrames = 5;

// Tests that the correct stereo output buffer is obtained from the converter
// given an arbitrary mono input buffer.
TEST(ChannelConverterTest, ConvertStereoFromMonoTest) {
  const std::vector<std::vector<float>> kMonoInput = {
      {1.0f, 2.0f, 3.0f, 4.0f, 5.0f}};
  const std::vector<std::vector<float>> kExpectedStereoOutput = {
      {0.707107f, 1.414214f, 2.121320f, 2.828427f, 3.535534f},
      {0.707107f, 1.414214f, 2.121320f, 2.828427f, 3.535534f}};

  // Initialize the test buffers.
  AudioBuffer mono_input(kNumMonoChannels, kNumFrames);
  mono_input = kMonoInput;
  AudioBuffer stereo_output(kNumStereoChannels, kNumFrames);
  // Process the input buffer.
  ConvertStereoFromMono(mono_input, &stereo_output);
  // Compare the output buffer against the expected output.
  for (size_t channel = 0; channel < kNumStereoChannels; ++channel) {
    for (size_t frame = 0; frame < kNumFrames; ++frame) {
      EXPECT_NEAR(stereo_output[channel][frame],
                  kExpectedStereoOutput[channel][frame], kEpsilonFloat);
    }
  }
}

// Tests that the correct mono output buffer is obtained from the converter
// given an arbitrary stereo input buffer.
TEST(ChannelConverterTest, ConvertMonoFromStereoTest) {
  const std::vector<std::vector<float>> kStereoInput = {
      {0.707107f, 1.414214f, 2.121320f, 2.828427f, 3.535534f},
      {0.707107f, 1.414214f, 2.121320f, 2.828427f, 3.535534f}};

  const std::vector<std::vector<float>> kExpectedMonoOutput = {
      {1.0f, 2.0f, 3.0f, 4.0f, 5.0f}};

  // Initialize the test buffers.
  AudioBuffer stereo_input(kNumStereoChannels, kNumFrames);
  stereo_input = kStereoInput;
  AudioBuffer mono_output(kNumMonoChannels, kNumFrames);
  // Process the input buffer.
  ConvertMonoFromStereo(stereo_input, &mono_output);
  // Compare the output buffer against the expected output.
  for (size_t frame = 0; frame < kNumFrames; ++frame) {
    EXPECT_NEAR(mono_output[0][frame], kExpectedMonoOutput[0][frame],
                kEpsilonFloat);
  }
}

}  // namespace

}  // namespace vraudio
