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

#include "dsp/gain_mixer.h"

#include <iterator>
#include <vector>

#include "third_party/googletest/googletest/include/gtest/gtest.h"
#include "base/constants_and_types.h"
#include "utils/planar_interleaved_conversion.h"

namespace vraudio {

namespace {

const int kId1 = 0;
const int kId2 = 1;

// Number of frames per output buffer.
const size_t kFramesPerBuffer = 3;

// Sample mono input data, gains, and the corresponding output data.
const float kGain1 = 0.5f;
const float kGain2 = 2.0f;
const float kInput1[kFramesPerBuffer] = {0.2f, 0.4f, 0.6f};
const float kInput2[kFramesPerBuffer] = {0.5f, 1.0f, 1.5f};
const float kOutput[kFramesPerBuffer] = {1.1f, 2.2f, 3.3f};

// Tests that the gain mixer returns null output if no input has been added.
TEST(MixerTest, EmptyInputTest) {
  GainMixer gain_mixer(kNumMonoChannels, kFramesPerBuffer);

  const AudioBuffer* output = gain_mixer.GetOutput();
  EXPECT_TRUE(output == nullptr);
}

// Mono processing test with two inputs. Tests the mixed output buffer against
// the manually computed output data.
TEST(GainMixerTest, ProcessTest) {
  GainMixer gain_mixer(kNumMonoChannels, kFramesPerBuffer);

  // Initialize input buffers.
  AudioBuffer input1(kNumMonoChannels, kFramesPerBuffer);
  input1.set_source_id(kId1);
  FillAudioBuffer(std::begin(kInput1), kFramesPerBuffer, kNumMonoChannels,
                  &input1);
  AudioBuffer input2(kNumMonoChannels, kFramesPerBuffer);
  input2.set_source_id(kId2);
  FillAudioBuffer(std::begin(kInput2), kFramesPerBuffer, kNumMonoChannels,
                  &input2);
  // Initialize gain vectors.
  const std::vector<float> kGainVector1(1, kGain1);
  const std::vector<float> kGainVector2(1, kGain2);

  // Add the input buffers to the |GainMixer| (process multiple times so the
  // gains have reached steady state.
  const AudioBuffer* output = nullptr;
  for (size_t iterations = 0; iterations < kUnitRampLength; ++iterations) {
    gain_mixer.Reset();
    gain_mixer.AddInput(input1, kGainVector1);
    gain_mixer.AddInput(input2, kGainVector2);
    // Get the processed output (should contain the pre-computed output data).
    output = gain_mixer.GetOutput();
  }

  EXPECT_FALSE(output == nullptr);
  EXPECT_EQ(output->num_channels(), kNumMonoChannels);
  EXPECT_EQ(output->num_frames(), kFramesPerBuffer);
  for (size_t i = 0; i < kFramesPerBuffer; ++i) {
    EXPECT_NEAR((*output)[0][i], kOutput[i], kEpsilonFloat);
  }
}

// Reset test for the |GainMixer| with single input at a time. First, adds an
// input buffer, resets the |GainMixer|. Then, adds another input buffer and
// tests the final output buffer against the data of the last added input buffer
// to verify if the system has been successfully reset.
TEST(GainMixerTest, ResetTest) {
  GainMixer gain_mixer(kNumMonoChannels, kFramesPerBuffer);

  // Initialize input buffers.
  AudioBuffer input1(kNumMonoChannels, kFramesPerBuffer);
  input1.set_source_id(kId1);
  FillAudioBuffer(std::begin(kInput1), kFramesPerBuffer, kNumMonoChannels,
                  &input1);
  AudioBuffer input2(kNumMonoChannels, kFramesPerBuffer);
  input2.set_source_id(kId2);
  FillAudioBuffer(std::begin(kInput2), kFramesPerBuffer, kNumMonoChannels,
                  &input2);
  // Initialize gain vectors.
  const std::vector<float> kGainVector1(1, kGain1);
  const std::vector<float> kGainVector2(1, kGain2);

  // Add the first input buffer to the |GainMixer|.
  gain_mixer.AddInput(input1, kGainVector1);
  // Reset the accumulator (and release the buffer).
  gain_mixer.Reset();
  // Add the second input buffers to the |GainMixer|.
  gain_mixer.AddInput(input2, kGainVector2);

  // Get the output (should only contain the second input).
  const AudioBuffer* output = gain_mixer.GetOutput();

  EXPECT_FALSE(output == nullptr);
  EXPECT_EQ(output->num_channels(), kNumMonoChannels);
  EXPECT_EQ(output->num_frames(), kFramesPerBuffer);
  for (size_t i = 0; i < kFramesPerBuffer; ++i) {
    EXPECT_NEAR((*output)[0][i], kInput2[i] * kGain2, kEpsilonFloat);
  }
}

// Tests that gains are correctly applied for multiple channels.
TEST(GainMixerTest, MultiChannelTest) {
  const std::vector<float> kGains = {0.1f, -0.2f};
  const std::vector<float> kInputChannel = {10.0f, 10.0f, 10.0f};
  GainMixer gain_mixer(kGains.size(), kFramesPerBuffer);

  // Initialize input buffer.
  AudioBuffer input(kGains.size(), kInputChannel.size());
  for (size_t i = 0; i < kGains.size(); ++i) {
    input[i] = kInputChannel;
  }

  const AudioBuffer* output = nullptr;
  for (size_t iterations = 0; iterations < kUnitRampLength; ++iterations) {
    gain_mixer.Reset();
    gain_mixer.AddInput(input, kGains);
    output = gain_mixer.GetOutput();
  }

  EXPECT_FALSE(output == nullptr);
  EXPECT_EQ(output->num_channels(), kGains.size());
  EXPECT_EQ(output->num_frames(), kFramesPerBuffer);
  for (size_t channel = 0; channel < kGains.size(); ++channel) {
    for (size_t i = 0; i < kInputChannel.size(); ++i) {
      EXPECT_NEAR((*output)[channel][i], kInputChannel[i] * kGains[channel],
                  kEpsilonFloat);
    }
  }
}

// Tests that gains are correctly applied for a mono input buffer across a
// multichannel output buffer.
TEST(GainMixerTest, MonoChannelInputTest) {
  const std::vector<float> kGains = {2.0f, -3.0f};
  const std::vector<float> kInputChannel = {10.0f, 10.0f, 10.0f};
  GainMixer gain_mixer(kGains.size(), kInputChannel.size());

  // Create a mono input buffer.
  AudioBuffer input(kNumMonoChannels, kInputChannel.size());
  AudioBuffer::Channel* input_channel = &input[0];
  *input_channel = kInputChannel;

  const AudioBuffer* output = nullptr;
  for (size_t iterations = 0; iterations < kUnitRampLength; ++iterations) {
    gain_mixer.Reset();
    gain_mixer.AddInputChannel(*input_channel, kId1, kGains);
    output = gain_mixer.GetOutput();
  }

  EXPECT_FALSE(output == nullptr);
  EXPECT_EQ(output->num_channels(), kGains.size());
  EXPECT_EQ(output->num_frames(), kFramesPerBuffer);
  for (size_t channel = 0; channel < kGains.size(); ++channel) {
    for (size_t i = 0; i < kInputChannel.size(); ++i) {
      EXPECT_NEAR((*output)[channel][i], kInputChannel[i] * kGains[channel],
                  kEpsilonFloat);
    }
  }
}

}  // namespace

}  // namespace vraudio
