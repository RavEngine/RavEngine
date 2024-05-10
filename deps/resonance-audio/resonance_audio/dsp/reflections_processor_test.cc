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

#include "dsp/reflections_processor.h"

#include <algorithm>
#include <utility>
#include <vector>

#include "third_party/googletest/googletest/include/gtest/gtest.h"
#include "api/resonance_audio_api.h"
#include "base/constants_and_types.h"
#include "utils/test_util.h"

namespace vraudio {

namespace {

const size_t kFramesPerBuffer = 512;
const int kSampleRate = 48000;
const float kRoomDimensions[3] = {2.0f, 2.0f, 2.0f};
const size_t kExpectedDelaySamples = 279;

}  // namespace

class ReflectionsProcessorTest : public ::testing::Test {
 protected:
  void SetUp() override {
    const float kReflectionCoefficients[kNumRoomSurfaces] = {0.5f, 0.5f, 0.5f,
                                                             0.5f, 0.5f, 0.5f};
    const WorldPosition kListenerPosition(0.0f, 0.0f, 0.0f);
    std::copy(std::begin(kReflectionCoefficients),
              std::end(kReflectionCoefficients),
              std::begin(reflection_properties_.coefficients));
    std::copy(std::begin(kRoomDimensions), std::end(kRoomDimensions),
              std::begin(reflection_properties_.room_dimensions));
    listener_position_ = kListenerPosition;
  }

  ReflectionProperties reflection_properties_;
  WorldPosition listener_position_;
};  // namespace vraudio

// Tests that the processed output is delayed, filtered, and scaled as expected.
TEST_F(ReflectionsProcessorTest, ProcessTest) {
  ReflectionsProcessor reflections_processor(kSampleRate, kFramesPerBuffer);
  reflections_processor.Update(reflection_properties_, listener_position_);

  AudioBuffer input(kNumMonoChannels, kFramesPerBuffer);
  AudioBuffer output(kNumFirstOrderAmbisonicChannels, kFramesPerBuffer);
  // Process until we have transitioned to the specified Gain.
  for (size_t i = 0; i < kUnitRampLength; i += kFramesPerBuffer) {
    input.Clear();
    output.Clear();
    GenerateDiracImpulseFilter(0, &input[0]);
    reflections_processor.Process(input, &output);
  }

  // The reflections calculated should be as follows:
  // delay = 280 samples, magnitude = 0.25, direction = {azim 90, elev 0}.
  // delay = 280 samples, magnitude = 0.25, direction = {azim -90, elev 0}.
  // delay = 280 samples, magnitude = 0.25, direction = {azim 0, elev -90}.
  // delay = 280 samples, magnitude = 0.25, direction = {azim 0, elev 90}.
  // delay = 280 samples, magnitude = 0.25, direction = {azim 0, elev 0}.
  // delay = 280 samples, magnitude = 0.25, direction = {azim 180, elev 0}.
  const float kExpectedMagnitude = 0.25f;
  // We expect the following ambisonic encoding coefficients:
  // {azim 90, elev 0} = {1 1 0 0}.
  // {azim -90, elev 0} = {1 -1 0 0}.
  // {azim 0, elev -90} = {1 0 -1 0}.
  // {azim 0, elev 90} = {1 0 1 0}.
  // {azim 0, elev 0} = {1 0 0 1}.
  // {azim 180, elev 0} = {1 0 0 -1}.
  const std::vector<std::vector<float>> kEncodingCoefficients = {
      {1.0f, 1.0f, 0.0f, 0.0f},  {1.0f, -1.0f, 0.0f, 0.0f},
      {1.0f, 0.0f, -1.0f, 0.0f}, {1.0f, 0.0f, 1.0f, 0.0f},
      {1.0f, 0.0f, 0.0f, 1.0f},  {1.0f, 0.0f, 0.0f, -1.0f}};

  for (size_t j = kExpectedDelaySamples; j < kFramesPerBuffer; ++j) {
    for (size_t k = 0; k < kNumFirstOrderAmbisonicChannels; ++k) {
      float coefficient_sum = 0.0f;
      for (size_t i = 0; i < kEncodingCoefficients.size(); ++i) {
        coefficient_sum += kEncodingCoefficients[i][k];
      }
      const float expected_sample = input[0][j - kExpectedDelaySamples] *
                                    kExpectedMagnitude * coefficient_sum;
      EXPECT_NEAR(expected_sample, output[k][j], kEpsilonFloat);
    }
  }
}

// Tests that when transitioning from a reflection with zero magnitude to one
// with non-zero magnitude, there will be a steady incremental increase in
// the "fade in".
TEST_F(ReflectionsProcessorTest, CrossFadeTest) {
  ReflectionsProcessor reflections_processor(kSampleRate, kFramesPerBuffer);

  AudioBuffer input(kNumMonoChannels, kFramesPerBuffer);
  AudioBuffer output(kNumFirstOrderAmbisonicChannels, kFramesPerBuffer);

  input.Clear();
  output.Clear();
  for (size_t i = 0; i < kFramesPerBuffer; ++i) {
    input[0][i] = 1.0f;
  }

  reflections_processor.Process(input, &output);
  reflections_processor.Update(reflection_properties_, listener_position_);
  reflections_processor.Process(input, &output);

  // All the reflections are expected to arrive at the listener at the same
  // time. That is why their directional contributions are expected to cancel
  // out. Only in first channel we will see a steady increase.
  for (size_t channel = 0; channel < kNumFirstOrderAmbisonicChannels;
       ++channel) {
    if (channel < kNumMonoChannels) {
      for (size_t frame = kExpectedDelaySamples; frame < kFramesPerBuffer;
           ++frame) {
        EXPECT_TRUE(output[channel][frame] > output[channel][frame - 1]);
      }
    } else {
      for (size_t frame = 0; frame < kFramesPerBuffer; ++frame) {
        EXPECT_NEAR(output[channel][frame], 0.0f, kEpsilonFloat);
      }
    }
  }
}

}  // namespace vraudio
