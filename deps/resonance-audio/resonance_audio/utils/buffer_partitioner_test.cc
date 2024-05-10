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

#include "utils/buffer_partitioner.h"

#include <memory>

#include "third_party/googletest/googletest/include/gtest/gtest.h"
#include "base/audio_buffer.h"
#include "base/constants_and_types.h"
#include "base/misc_math.h"
#include "utils/planar_interleaved_conversion.h"
#include "utils/test_util.h"

namespace vraudio {

const size_t kNumFramesPerBuffer = 64;

using vraudio::AudioBuffer;

class BufferPartitionerTest : public ::testing::Test {
 public:
  // Public validation method. This is called for every generated |AudioBuffer|.
  AudioBuffer* ValidateAudioBufferCallback(AudioBuffer* audio_buffer) {
    if (audio_buffer != nullptr) {
      std::vector<float> received_buffer_interleaved;
      FillExternalBuffer(*audio_buffer, &received_buffer_interleaved);
      total_received_buffer_interleaved_.insert(
          total_received_buffer_interleaved_.end(),
          received_buffer_interleaved.begin(),
          received_buffer_interleaved.end());

      ++num_total_buffers_received_;
      num_total_frames_received_ += audio_buffer->num_frames();
    }
    ++num_total_callbacks_triggered_;
    return &partitioned_buffer_;
  }

 protected:
  BufferPartitionerTest()
      : partitioned_buffer_(kNumMonoChannels, kNumFramesPerBuffer),
        partitioner_(
            kNumMonoChannels, kNumFramesPerBuffer,
            std::bind(&BufferPartitionerTest::ValidateAudioBufferCallback, this,
                      std::placeholders::_1)),
        num_total_frames_received_(0),
        num_total_buffers_received_(0),
        num_total_callbacks_triggered_(0) {}
  ~BufferPartitionerTest() override {}

  // Returns a section of an |AudioBuffer| in interleaved format.
  template <typename T>
  std::vector<T> GetInterleavedVectorFromAudioBufferSection(
      const AudioBuffer& planar_buffer, size_t frame_offset,
      size_t num_frames) {
    CHECK_GE(planar_buffer.num_frames(), frame_offset + num_frames);
    std::vector<T> return_vector(planar_buffer.num_channels() * num_frames);

    FillExternalBufferWithOffset(
        planar_buffer, frame_offset, return_vector.data(), num_frames,
        planar_buffer.num_channels(), 0 /* output_offset_frames */, num_frames);

    return return_vector;
  }

  template <typename T>
  void SmallBufferInputTest(float buffer_epsilon) {
    ResetTest();

    const size_t kNumInputBuffers = 8;
    const size_t kNumFramesPerTestBuffer =
        kNumFramesPerBuffer / kNumInputBuffers;

    AudioBuffer test_buffer(vraudio::kNumMonoChannels, kNumFramesPerBuffer);
    GenerateSawToothSignal(kNumFramesPerBuffer, &test_buffer[0]);

    for (size_t i = 0; i < kNumInputBuffers; ++i) {
      const std::vector<T> input_interleaved =
          GetInterleavedVectorFromAudioBufferSection<T>(
              test_buffer, kNumFramesPerTestBuffer * i,
              kNumFramesPerTestBuffer);

      partitioner_.AddBuffer(input_interleaved.data(), kNumMonoChannels,
                             kNumFramesPerTestBuffer);
    }
    EXPECT_EQ(num_total_buffers_received_, 1U);
    EXPECT_EQ(num_total_callbacks_triggered_, num_total_buffers_received_ + 1);
    EXPECT_EQ(num_total_frames_received_, kNumFramesPerBuffer);

    AudioBuffer reconstructed_buffer(kNumMonoChannels, kNumFramesPerBuffer);
    FillAudioBuffer(total_received_buffer_interleaved_, kNumMonoChannels,
                    &reconstructed_buffer);

    EXPECT_TRUE(CompareAudioBuffers(test_buffer[0], reconstructed_buffer[0],
                                    buffer_epsilon));
  }

  template <typename T>
  void LargeBufferTest(float buffer_epsilon) {
    ResetTest();
    const size_t kNumOutputBuffers = 8;
    const size_t kNumFramesPerTestBuffer =
        kNumFramesPerBuffer * kNumOutputBuffers;

    AudioBuffer test_buffer(vraudio::kNumMonoChannels, kNumFramesPerTestBuffer);
    GenerateSawToothSignal(kNumFramesPerTestBuffer, &test_buffer[0]);

    const std::vector<T> input_interleaved =
        GetInterleavedVectorFromAudioBufferSection<T>(test_buffer, 0,
                                                      kNumFramesPerTestBuffer);

    partitioner_.AddBuffer(input_interleaved.data(), kNumMonoChannels,
                           kNumFramesPerTestBuffer);

    EXPECT_EQ(num_total_buffers_received_, kNumOutputBuffers);
    EXPECT_EQ(num_total_callbacks_triggered_, num_total_buffers_received_ + 1);
    EXPECT_EQ(num_total_frames_received_, kNumFramesPerTestBuffer);

    AudioBuffer reconstructed_buffer(kNumMonoChannels, kNumFramesPerTestBuffer);
    FillAudioBuffer(total_received_buffer_interleaved_, kNumMonoChannels,
                    &reconstructed_buffer);

    EXPECT_TRUE(CompareAudioBuffers(test_buffer[0], reconstructed_buffer[0],
                                    buffer_epsilon));
  }

  void ResetTest() {
    num_total_frames_received_ = 0;
    num_total_buffers_received_ = 0;
    num_total_callbacks_triggered_ = 0;
    total_received_buffer_interleaved_.clear();

    partitioner_.Clear();
  }

  AudioBuffer partitioned_buffer_;
  std::vector<float> total_received_buffer_interleaved_;

  BufferPartitioner partitioner_;

  // Stores the total number of frames received in
  // |ValidateAudioBufferCallback|.
  size_t num_total_frames_received_;

  // Stores the total number of buffers received in
  // |ValidateAudioBufferCallback|.
  size_t num_total_buffers_received_;

  // Stores the total number of callbacks being triggered in
  // |ValidateAudioBufferCallback|.
  size_t num_total_callbacks_triggered_;
};

// Tests the concatenation of multiple small buffers into a single
// |AudioBuffer|.
TEST_F(BufferPartitionerTest, TestSmallBufferInput) {
  // int16 to float conversions introduce rounding errors.
  SmallBufferInputTest<int16>(1e-4f /* buffer_epsilon */);
  SmallBufferInputTest<float>(1e-6f /* buffer_epsilon */);
}

// Tests the splitting of a large input buffer into multiple |AudioBuffer|s.
TEST_F(BufferPartitionerTest, TestLargeBufferInput) {
  // int16 to float conversions introduce rounding errors.
  LargeBufferTest<int16>(1e-4f /* buffer_epsilon */);
  LargeBufferTest<float>(1e-6f /* buffer_epsilon */);
}

// Tests the GetNumBufferedFramesTest() and
// GetNumGeneratedBuffersForNumInputFrames() methods.
TEST_F(BufferPartitionerTest, GetNumBufferedFramesTest) {
  const std::vector<float> input_interleaved(1, 1.0f);

  EXPECT_EQ(partitioner_.GetNumGeneratedBuffersForNumInputFrames(
                kNumFramesPerBuffer - 1),
            0U);
  EXPECT_EQ(
      partitioner_.GetNumGeneratedBuffersForNumInputFrames(kNumFramesPerBuffer),
      1U);

  for (size_t i = 0; i < kNumFramesPerBuffer * 2; ++i) {
    EXPECT_EQ(partitioner_.GetNumBufferedFrames(), i % kNumFramesPerBuffer);

    const size_t large_random_frame_number =
        i * 7 * kNumFramesPerBuffer + 13 * i;
    // Expected number of generated buffers is based on
    // |large_random_frame_number| and the internally stored remainder |i %
    // kNumFramesPerBuffer|.
    const size_t expected_num_generated_buffers =
        (large_random_frame_number + partitioner_.GetNumBufferedFrames()) /
        kNumFramesPerBuffer;

    EXPECT_EQ(partitioner_.GetNumGeneratedBuffersForNumInputFrames(
                  large_random_frame_number),
              expected_num_generated_buffers);

    // Add single frame to |partinioner_|.
    partitioner_.AddBuffer(input_interleaved.data(), kNumMonoChannels, 1);
  }
}

// Tests the Flush() method.
TEST_F(BufferPartitionerTest, FlushTest) {
  AudioBuffer test_buffer(vraudio::kNumMonoChannels, kNumFramesPerBuffer);
  GenerateDiracImpulseFilter(0, &test_buffer[0]);

  const std::vector<float> input_interleaved =
      GetInterleavedVectorFromAudioBufferSection<float>(
          test_buffer, 0 /* frame_offset */, kNumFramesPerBuffer);

  // Add only the first sample that contains the dirac impulse.
  partitioner_.AddBuffer(input_interleaved.data(), kNumMonoChannels, 1);
  partitioner_.Flush();

  EXPECT_EQ(num_total_buffers_received_, 1U);
  EXPECT_EQ(num_total_callbacks_triggered_, num_total_buffers_received_ + 1);
  EXPECT_EQ(num_total_frames_received_, kNumFramesPerBuffer);

  AudioBuffer reconstructed_buffer(kNumMonoChannels, kNumFramesPerBuffer);
  FillAudioBuffer(total_received_buffer_interleaved_, kNumMonoChannels,
                  &reconstructed_buffer);

  EXPECT_TRUE(CompareAudioBuffers(test_buffer[0], reconstructed_buffer[0],
                                  kEpsilonFloat));
}

}  // namespace vraudio
