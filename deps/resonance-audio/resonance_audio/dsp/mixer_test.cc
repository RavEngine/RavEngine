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

#include "dsp/mixer.h"

#include <iterator>
#include <vector>

#include "third_party/googletest/googletest/include/gtest/gtest.h"
#include "base/constants_and_types.h"
#include "utils/planar_interleaved_conversion.h"

namespace vraudio {

namespace {

// Number of frames per output buffer.
const size_t kFramesPerBuffer = 3;

// Sample two-channel interleaved input data.
const float kStereoInput1[kNumStereoChannels * kFramesPerBuffer] = {
    0.5f, 0.0f, -0.25f, 0.0f, 0.25f, 0.0f};
const float kStereoInput2[kNumStereoChannels * kFramesPerBuffer] = {
    0.5f, 1.0f, 0.5f, -1.0f, 0.5f, 1.0f};

// Tests that the mixer returns null output if no input has been added.
TEST(MixerTest, EmptyInputTest) {
  Mixer mixer(kNumStereoChannels, kFramesPerBuffer);

  const AudioBuffer* output = mixer.GetOutput();
  EXPECT_TRUE(output == nullptr);
}

// This is a simple two-channel process test with two inputs. Tests the
// mixed output buffer against the manually computed output data.
TEST(MixerTest, ProcessUniformInputChannelsTest) {
  Mixer mixer(kNumStereoChannels, kFramesPerBuffer);

  // Initialize the input buffers.
  AudioBuffer input1(kNumStereoChannels, kFramesPerBuffer);
  FillAudioBuffer(std::begin(kStereoInput1), kFramesPerBuffer,
                  kNumStereoChannels, &input1);
  AudioBuffer input2(kNumStereoChannels, kFramesPerBuffer);
  FillAudioBuffer(std::begin(kStereoInput2), kFramesPerBuffer,
                  kNumStereoChannels, &input2);

  // Add the input buffers to the mixer.
  mixer.AddInput(input1);
  mixer.AddInput(input2);
  // Get the processed output.
  const AudioBuffer* output = mixer.GetOutput();

  // Test that the output channels was accumulated correctly.
  EXPECT_FALSE(output == nullptr);
  EXPECT_EQ(output->num_channels(), kNumStereoChannels);
  EXPECT_EQ(output->num_frames(), kFramesPerBuffer);
  for (size_t n = 0; n < kNumStereoChannels; ++n) {
    for (size_t i = 0; i < kFramesPerBuffer; ++i) {
      EXPECT_NEAR((*output)[n][i], input1[n][i] + input2[n][i], kEpsilonFloat);
    }
  }
}

// This is a non-uniform process test with two inputs with arbitrary number of
// channels. Tests the mixed output buffer with different target number of
// channels against the manually computed output data.
TEST(MixerTest, ProcessVaryingInputChannelsTest) {
  // Initialize the input buffers.
  AudioBuffer mono_input(kNumMonoChannels, kFramesPerBuffer);
  FillAudioBuffer(std::begin(kStereoInput1), kFramesPerBuffer, kNumMonoChannels,
                  &mono_input);
  AudioBuffer stereo_input(kNumStereoChannels, kFramesPerBuffer);
  FillAudioBuffer(std::begin(kStereoInput2), kFramesPerBuffer,
                  kNumStereoChannels, &stereo_input);

  // Initialize a mono mixer.
  Mixer mono_mixer(kNumMonoChannels, kFramesPerBuffer);
  // Add the input buffers to the mixer.
  mono_mixer.AddInput(mono_input);
  mono_mixer.AddInput(stereo_input);
  // Get the processed output.
  const AudioBuffer* mono_output = mono_mixer.GetOutput();
  // Test that the mono output channel was accumulated correctly.
  EXPECT_FALSE(mono_output == nullptr);
  EXPECT_EQ(mono_output->num_channels(), kNumMonoChannels);
  EXPECT_EQ(mono_output->num_frames(), kFramesPerBuffer);
  for (size_t i = 0; i < kFramesPerBuffer; ++i) {
    EXPECT_NEAR((*mono_output)[0][i], mono_input[0][i] + stereo_input[0][i],
                kEpsilonFloat);
  }

  // Initialize a stereo mixer.
  Mixer stereo_mixer(kNumStereoChannels, kFramesPerBuffer);
  // Add the input buffers to the mixer.
  stereo_mixer.AddInput(mono_input);
  stereo_mixer.AddInput(stereo_input);
  // Get the processed output.
  const AudioBuffer* stereo_output = stereo_mixer.GetOutput();
  // Test that the stereo output channels were accumulated correctly.
  EXPECT_FALSE(stereo_output == nullptr);
  EXPECT_EQ(stereo_output->num_channels(), kNumStereoChannels);
  EXPECT_EQ(stereo_output->num_frames(), kFramesPerBuffer);
  for (size_t n = 0; n < kNumStereoChannels; ++n) {
    for (size_t i = 0; i < kFramesPerBuffer; ++i) {
      // The second channel should only contain the samples from |stereo_input|.
      EXPECT_NEAR(
          (*stereo_output)[n][i],
          (n == 0) ? mono_input[n][i] + stereo_input[n][i] : stereo_input[n][i],
          kEpsilonFloat);
    }
  }
}

// This is a two-channel reset test for the mixer with single input at a time.
// First, adds an input buffer, resets the mixer. Then, adds another input
// buffer and tests the final output buffer against the data of the last added
// input buffer to verify if the system has been successfully reset.
TEST(MixerTest, ResetTest) {
  Mixer mixer(kNumStereoChannels, kFramesPerBuffer);

  // Initialize the input buffers.
  AudioBuffer input1(kNumStereoChannels, kFramesPerBuffer);
  FillAudioBuffer(std::begin(kStereoInput1), kFramesPerBuffer,
                  kNumStereoChannels, &input1);
  AudioBuffer input2(kNumStereoChannels, kFramesPerBuffer);
  FillAudioBuffer(std::begin(kStereoInput2), kFramesPerBuffer,
                  kNumStereoChannels, &input2);

  // Add the first input buffer to the mixer.
  mixer.AddInput(input1);
  // Reset the accumulator.
  mixer.Reset();
  // Add the second input buffers to the mixer.
  mixer.AddInput(input2);
  // Get the processed output.
  const AudioBuffer* output = mixer.GetOutput();
  // Test that the output channels contains only the samples from |input2|.
  EXPECT_FALSE(output == nullptr);
  EXPECT_EQ(output->num_channels(), kNumStereoChannels);
  EXPECT_EQ(output->num_frames(), kFramesPerBuffer);
  for (size_t n = 0; n < kNumStereoChannels; ++n) {
    for (size_t i = 0; i < kFramesPerBuffer; ++i) {
      EXPECT_NEAR((*output)[n][i], input2[n][i], kEpsilonFloat);
    }
  }
}

}  // namespace

}  // namespace vraudio
