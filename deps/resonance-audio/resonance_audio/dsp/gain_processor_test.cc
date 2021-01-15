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

#include "dsp/gain_processor.h"

#include <algorithm>
#include <cmath>
#include <vector>

#include "third_party/googletest/googletest/include/gtest/gtest.h"
#include "base/audio_buffer.h"
#include "base/constants_and_types.h"

// A set of simple tests to confirm expected behavior when a linear gain ramp is
// applied to a vector of samples.
namespace vraudio {

namespace {

// Initial gain value.
const float kInitialGain = 0.1f;

// Target gain value.
const float kTargetGain = 0.6f;

// Test input data length.
const size_t kInputLength = 10;

// Test value of each sample of the output buffer prior to processing.
const float kInitialOutputValue = 2.0f;

class GainProcessorTest : public ::testing::TestWithParam<bool> {};

// This test checks that gain value is applied to the input buffer in a linear
// ramp when the buffer length is less than the ramp length.
TEST_P(GainProcessorTest, ApplyGainOverBufferLengthTest) {
  // Expected output data for ramped gain application.
  const float kExpectedOutput[kInputLength] = {
      0.1f,       0.1004883f, 0.1009766f, 0.1014648f, 0.1019531f,
      0.1024414f, 0.1029297f, 0.1034180f, 0.1039063f, 0.1043945f};
  const bool accumulate_output = GetParam();
  // Initialize input buffer.
  AudioBuffer input(kNumMonoChannels, kInputLength);
  std::fill(input[0].begin(), input[0].end(), 1.0f);
  // Initialize output buffer.
  AudioBuffer output(kNumMonoChannels, kInputLength);
  std::fill(output[0].begin(), output[0].end(), kInitialOutputValue);

  // Initialize gain processor with a gain value.
  GainProcessor gain_processor(kInitialGain);
  // Process buffer samples with the test gain value.
  gain_processor.ApplyGain(kTargetGain, input[0], &output[0],
                           accumulate_output);

  // Check that gain values have been applied correctly to the buffer.
  for (size_t i = 0; i < kInputLength; ++i) {
    const float expected_value = accumulate_output
                                     ? kInitialOutputValue + kExpectedOutput[i]
                                     : kExpectedOutput[i];
    EXPECT_NEAR(expected_value, output[0][i], kEpsilonFloat);
  }
}

// This test checks that gain value is applied to the buffer samples in a linear
// ramp. The test also confirms that the gain ramping halts at the correct index
// for buffer lengths greater than the ramp length.
TEST_P(GainProcessorTest, ApplyGainLongerThanRampTest) {
  const bool accumulate_output = GetParam();
  // Initialize input buffer.
  AudioBuffer input(kNumMonoChannels, kUnitRampLength);
  std::fill(input[0].begin(), input[0].end(), 1.0f);
  // Initialize output buffer.
  AudioBuffer output(kNumMonoChannels, kUnitRampLength);
  std::fill(output[0].begin(), output[0].end(), kInitialOutputValue);

  // Initialize gain processor with a gain value.
  GainProcessor gain_processor(kInitialGain);
  // Process an input buffer with the test gain value.
  gain_processor.ApplyGain(kTargetGain, input[0], &output[0],
                           accumulate_output);

  // Generate expected gain ramp. The output should consist of the linearly
  // interpolated gain values over the ramp length, and the final (constant)
  // gain for the rest of the buffer.
  std::vector<float> expected_output(kUnitRampLength);
  const size_t ramp_length =
      static_cast<size_t>(std::abs(kTargetGain - kInitialGain) *
                          static_cast<float>(kUnitRampLength));
  const float increment =
      (kTargetGain - kInitialGain) / static_cast<float>(ramp_length);
  float expected_value =
      accumulate_output ? kInitialOutputValue + kInitialGain : kInitialGain;
  for (size_t i = 0; i < kUnitRampLength; ++i) {
    expected_output[i] = expected_value;
    if (i < ramp_length) {
      expected_value += increment;
    }
  }
  // Check that gain values have been applied correctly to the buffer.
  for (size_t i = 0; i < kUnitRampLength; ++i) {
    // Check that ramp was applied.
    EXPECT_NEAR(expected_output[i], output[0][i], kEpsilonFloat);
  }
}

// This test checks that gain values are reset to initial gain after a call to
// the |Reset()| method.
TEST_P(GainProcessorTest, ResetGainProcessorTest) {
  const bool accumulate_output = GetParam();
  // Initialize input buffer.
  AudioBuffer input(kNumMonoChannels, kUnitRampLength);
  std::fill(input[0].begin(), input[0].end(), 1.0f);
  // Initialize output buffer.
  AudioBuffer output(kNumMonoChannels, kUnitRampLength);
  std::fill(output[0].begin(), output[0].end(), 0.0f);

  // Initialize gain processor with a gain of 0.0f.
  GainProcessor gain_processor(0.0f);
  // Reset gain to |kInitialGain|.
  gain_processor.Reset(kInitialGain);
  // Apply newly-reset gains (no ramp expected).
  gain_processor.ApplyGain(kInitialGain, input[0], &output[0],
                           accumulate_output);

  // Check that uniform gain has been applied correctly to the buffer.
  for (size_t i = 0; i < kInputLength; ++i) {
    EXPECT_NEAR(kInitialGain, output[0][i], kEpsilonFloat);
  }
}

// Checks that the initial gain is assigned during the first call of |ApplyGain|
// in case the |GainProcessor| instance is constructed via the default
// constructor.
TEST_P(GainProcessorTest, DefaultConstructorTest) {
  const bool accumulate_output = GetParam();
  // Initialize input buffer.
  AudioBuffer input(kNumMonoChannels, kUnitRampLength);
  std::fill(input[0].begin(), input[0].end(), kInitialGain);
  // Initialize output buffer.
  AudioBuffer output(kNumMonoChannels, kUnitRampLength);
  std::fill(output[0].begin(), output[0].end(), 0.0f);

  // Declare gain processor without specifiying a gain value.
  GainProcessor gain_processor;
  // Apply some new gain value.
  gain_processor.ApplyGain(kTargetGain, input[0], &output[0],
                           accumulate_output);

  // Check that uniform gain has been applied correctly to the buffer.
  for (size_t i = 0; i < kInputLength; ++i) {
    EXPECT_NEAR(kInitialGain * kTargetGain, output[0][i], kEpsilonFloat);
  }
}

// Tests the |GetGain| method.
TEST(GainProcessorTest, GetGainTest) {
  // Test |GainProcessor| with arbitrary gain.
  const float kGainValue = -1.5f;
  GainProcessor gain_processor(kGainValue);
  EXPECT_EQ(gain_processor.GetGain(), kGainValue);
}

INSTANTIATE_TEST_CASE_P(AccumulateOutput, GainProcessorTest,
                        ::testing::Values(false, true));

}  // namespace

}  // namespace vraudio
