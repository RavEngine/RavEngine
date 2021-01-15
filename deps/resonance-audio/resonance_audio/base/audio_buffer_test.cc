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

#include "base/audio_buffer.h"

#include "third_party/googletest/googletest/include/gtest/gtest.h"
#include "base/constants_and_types.h"

namespace vraudio {

namespace {

// Tests default constructor.
TEST(AudioBuffer, AudioBufferDefaultConstructor) {
  AudioBuffer audio_buffer;
  EXPECT_EQ(audio_buffer.num_channels(), 0U);
  EXPECT_EQ(audio_buffer.num_frames(), 0U);
}

// Tests initialization of |AudioBuffer|.
TEST(AudioBuffer, AudioBufferInitializationTest) {
  static const size_t kNumChannels = 2;
  static const size_t kFramesPerBuffer = 16;
  AudioBuffer audio_buffer(kNumChannels, kFramesPerBuffer);

  EXPECT_EQ(audio_buffer.num_channels(), kNumChannels);
  EXPECT_EQ(audio_buffer.num_frames(), kFramesPerBuffer);
  EXPECT_EQ(static_cast<size_t>(audio_buffer.end() - audio_buffer.begin()),
            kNumChannels);

  // Test range-based for-loop.
  size_t channel_idx = 0;
  for (const AudioBuffer::Channel& channel : audio_buffer) {
    EXPECT_EQ(channel.begin(), audio_buffer[channel_idx].begin());
    EXPECT_EQ(channel.end(), audio_buffer[channel_idx].end());
    ++channel_idx;
  }
}

// Tests assignment operator from std::vector<std::vector<float>>.
TEST(AudioBuffer, AudioBufferAssignmentOperator) {
  const std::vector<std::vector<float>> kTestVector = {
      {0.0f, 1.0f, 2.0f}, {3.0f, 4.0f, 5.0f}, {6.0f, 7.0f, 8.0f}};

  AudioBuffer audio_buffer(kTestVector.size(), kTestVector[0].size());
  audio_buffer = kTestVector;

  for (size_t channel = 0; channel < kTestVector.size(); ++channel) {
    for (size_t frame = 0; frame < kTestVector[0].size(); ++frame) {
      EXPECT_EQ(audio_buffer[channel][frame], kTestVector[channel][frame]);
    }
  }
}

// Tests move constructor.
TEST(AudioBuffer, AudioBufferMoveConstructor) {
  const std::vector<std::vector<float>> kTestVector = {
      {0.0f, 1.0f, 2.0f}, {3.0f, 4.0f, 5.0f}, {6.0f, 7.0f, 8.0f}};

  AudioBuffer audio_buffer(kTestVector.size(), kTestVector[0].size());
  audio_buffer = kTestVector;
  const size_t num_channels = audio_buffer.num_channels();
  const size_t num_frames = audio_buffer.num_frames();

  AudioBuffer moved_audio_buffer(std::move(audio_buffer));
  EXPECT_EQ(audio_buffer.num_channels(), 0U);
  EXPECT_EQ(audio_buffer.num_frames(), 0U);
  EXPECT_EQ(moved_audio_buffer.num_channels(), num_channels);
  EXPECT_EQ(moved_audio_buffer.num_frames(), num_frames);

  for (size_t channel = 0; channel < kTestVector.size(); ++channel) {
    for (size_t frame = 0; frame < kTestVector[0].size(); ++frame) {
      EXPECT_EQ(moved_audio_buffer[channel][frame],
                kTestVector[channel][frame]);
    }
  }
}

// Tests memory alignment of each channel buffer. The address if the first
// element of each channel should be memory aligned.
TEST(AudioBuffer, TestBufferAlignment) {
  static const size_t kNumRuns = 100;
  static const size_t kNumChannels = 16;

  for (size_t run = 0; run < kNumRuns; ++run) {
    const size_t frames_per_buffer = run + 1;
    AudioBuffer audio_buffer(kNumChannels, frames_per_buffer);
    for (size_t channel = 0; channel < kNumChannels; ++channel) {
      const AudioBuffer::Channel& channel_view = audio_buffer[channel];
      const bool is_aligned =
          ((reinterpret_cast<size_t>(&(*channel_view.begin())) &
            (kMemoryAlignmentBytes - 1)) == 0);
      EXPECT_TRUE(is_aligned);
    }
  }
}

// Tests Clear method.
TEST(AudioBuffer, AudioBufferClear) {
  const std::vector<std::vector<float>> kTestVector = {
      {0.0f, 1.0f, 2.0f}, {3.0f, 4.0f, 5.0f}, {6.0f, 7.0f, 8.0f}};

  AudioBuffer audio_buffer(kTestVector.size(), kTestVector[0].size());
  audio_buffer = kTestVector;

  audio_buffer.Clear();

  for (size_t channel = 0; channel < kTestVector.size(); ++channel) {
    for (size_t frame = 0; frame < kTestVector[0].size(); ++frame) {
      EXPECT_EQ(0.0f, audio_buffer[channel][frame]);
    }
  }
}

// Tests GetChannelStride method.
TEST(AudioBuffer, GetChannelStride) {
  const size_t num_frames_per_alignment = kMemoryAlignmentBytes / sizeof(float);
  for (size_t num_frames = 1; num_frames < num_frames_per_alignment * 5;
       ++num_frames) {
    AudioBuffer buffer(1, num_frames);
    // Fast way to ceil(frame/num_frames_per_alignment).
    const size_t expected_num_alignment_blocks =
        (num_frames + num_frames_per_alignment - 1) / num_frames_per_alignment;
    EXPECT_EQ(expected_num_alignment_blocks * num_frames_per_alignment,
              buffer.GetChannelStride());
  }
}

}  // namespace

}  // namespace vraudio
