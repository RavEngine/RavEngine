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

#include "dsp/gain.h"

#include <algorithm>
#include <vector>

#include "third_party/googletest/googletest/include/gtest/gtest.h"
#include "base/audio_buffer.h"
#include "base/constants_and_types.h"
#include "dsp/utils.h"

namespace vraudio {

namespace {

// Tolerated error margin.
const float kEpsilon = 1e-4f;

class GainTest : public ::testing::TestWithParam<bool> {};

// This test checks that the gain applied using ConstantGain is correct.
TEST_P(GainTest, ConstantGainTest) {
  const size_t kMaxInputLength = 32;
  const float kGain = 1;
  const bool accumulate_output = GetParam();

  for (size_t length = 1; length < kMaxInputLength; ++length) {
    AudioBuffer input_samples(kNumMonoChannels, length);
    GenerateUniformNoise(
        /*min=*/-1.0f, /*min=*/1.0f, static_cast<unsigned>(length),
        &input_samples[0]);
    for (size_t offset_index = 0; offset_index < length; ++offset_index) {
      // Initialize input buffer.
      AudioBuffer input(kNumMonoChannels, length);
      input[0] = input_samples[0];
      // Initialize output buffer with the same values.
      AudioBuffer output(kNumMonoChannels, length);
      output[0] = input_samples[0];

      // Apply constant gain.
      ConstantGain(offset_index, kGain, input[0], &output[0],
                   accumulate_output);

      // Compute expected values.
      AudioBuffer expected_samples_buffer(kNumMonoChannels, length);
      expected_samples_buffer[0] = input_samples[0];
      for (size_t i = offset_index; i < length; ++i) {
        const float processed_input = kGain * input[0][i];
        if (accumulate_output) {
          expected_samples_buffer[0][i] += processed_input;
        } else {
          expected_samples_buffer[0][i] = processed_input;
        }
      }
      // Check that the output buffer has the expected values per each sample.
      for (size_t i = 0; i < length; ++i) {
        EXPECT_NEAR(expected_samples_buffer[0][i], output[0][i], kEpsilon)
            << " at index=" << i << " with input_length=" << length
            << " and offset_index=" << offset_index;
      }
    }
  }
}

// Test that checks that the gain ramp applied is correct.
TEST_P(GainTest, LinearGainRampTest) {
  const float kInitialOutputValue = 2.0f;
  const float kStartGain = 0.0f;
  const float kEndGain = 1.0f;
  const size_t kNumSamples = 10;
  const float kPerSampleIncrease =
      (kEndGain - kStartGain) / static_cast<float>(kNumSamples);
  const bool accumulate_output = GetParam();

  // Create an input buffer with unity samples.
  AudioBuffer input(kNumMonoChannels, kNumSamples);
  std::fill(input[0].begin(), input[0].end(), 1.0f);
  // Create an output buffer with all samples the same.
  AudioBuffer output(kNumMonoChannels, kNumSamples);
  std::fill(output[0].begin(), output[0].end(), kInitialOutputValue);

  // Apply linear gain ramp.
  LinearGainRamp(kNumSamples, kStartGain, kEndGain, input[0], &(output[0]),
                 accumulate_output);

  // Check that the output buffer has the expected values per each sample.
  float expected_value = accumulate_output ? kInitialOutputValue : 0.0f;
  for (size_t i = 0; i < kNumSamples; ++i) {
    EXPECT_NEAR(expected_value, output[0][i], kEpsilon);
    expected_value += kPerSampleIncrease;
  }
}

TEST(GainTest, GainUtilsTest) {
  const float kGainUnity = 1.0f;
  const float kGainZero = 0.0f;
  const float kGainOther = -1.7f;

  // Test the cases where each should return true.
  EXPECT_TRUE(IsGainNearZero(kGainZero));
  EXPECT_TRUE(IsGainNearUnity(kGainUnity));

  // Test the cases where gain value is non zero, positive and negative.
  EXPECT_FALSE(IsGainNearZero(kGainOther));
  EXPECT_FALSE(IsGainNearZero(kGainUnity));

  // Test the case where gain value is not unity, with alternate value and zero.
  EXPECT_FALSE(IsGainNearUnity(kGainOther));
  EXPECT_FALSE(IsGainNearUnity(kGainZero));
}

INSTANTIATE_TEST_CASE_P(AccumulateOutput, GainTest,
                        ::testing::Values(false, true));

}  // namespace

}  // namespace vraudio
