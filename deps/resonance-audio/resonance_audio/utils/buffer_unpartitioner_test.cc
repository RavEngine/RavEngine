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

#include "utils/buffer_unpartitioner.h"

#include <memory>
#include <numeric>

#include "third_party/googletest/googletest/include/gtest/gtest.h"
#include "base/audio_buffer.h"
#include "base/constants_and_types.h"
#include "base/misc_math.h"
#include "utils/planar_interleaved_conversion.h"
#include "utils/sample_type_conversion.h"
#include "utils/test_util.h"

namespace vraudio {

// Number of channels in test audio buffers.
const size_t kNumChannels = vraudio::kNumStereoChannels;

// Number of frames per test audio buffer.
const size_t kNumFramesPerBuffer = 64;

using vraudio::AudioBuffer;

class BufferUnpartitionerTest : public ::testing::Test {
 public:
  // Generates a vector of int16 values with arbitrary data (vec[i] = i).
  std::vector<int16> GenerateTestData(size_t num_channels, size_t num_frames) {
    std::vector<int16> test_data(num_channels * num_frames);
    std::iota(std::begin(test_data), std::end(test_data), 0);
    return test_data;
  }

  // Generates an AudioBuffer from a segment of an interleaved std::vector.
  AudioBuffer GetAudioBufferFromTestData(const std::vector<int16>& test_data,
                                         size_t num_channels,
                                         size_t num_frame_offset,
                                         size_t num_audio_buffer_frames) {
    const size_t num_input_frames = test_data.size() / num_channels;
    AudioBuffer test_audio_buffer(num_channels, num_audio_buffer_frames);
    FillAudioBufferWithOffset(&test_data[0], num_input_frames, num_channels,
                              num_frame_offset, 0 /* output_frame_offset */,
                              num_audio_buffer_frames, &test_audio_buffer);
    return test_audio_buffer;
  }

 protected:
  BufferUnpartitionerTest() {}
  ~BufferUnpartitionerTest() override {}

  const AudioBuffer* PassNextAudioBufferToBufferUnpartitioner(
      size_t num_input_buffer_size_frames) {
    input_audio_buffer_ = GetAudioBufferFromTestData(
        input_audio_vector_, kNumChannels,
        num_input_buffer_size_frames * num_callback_calls_,
        num_input_buffer_size_frames);
    ++num_callback_calls_;
    return &input_audio_buffer_;
  }

  void InitBufferUnpartitioner(size_t num_total_frames_to_test,
                               size_t num_input_buffer_size_frames) {
    input_audio_vector_ =
        GenerateTestData(kNumChannels, num_total_frames_to_test);

    num_callback_calls_ = 0;
    unpartitioner_.reset(new BufferUnpartitioner(
        kNumChannels, num_input_buffer_size_frames,
        std::bind(
            &BufferUnpartitionerTest::PassNextAudioBufferToBufferUnpartitioner,
            this, num_input_buffer_size_frames)));
  }

  // Returns the number of triggered callback calls.
  template <typename T>
  size_t TestInterleavedBufferOutputTest(size_t num_input_buffer_size_frames,
                                         size_t num_output_buffer_size_frames,
                                         size_t num_total_frames_to_test,
                                         float buffer_epsilon) {
    InitBufferUnpartitioner(num_total_frames_to_test,
                            num_input_buffer_size_frames);

    std::vector<T> output_vector(input_audio_vector_.size(), static_cast<T>(0));
    for (size_t b = 0;
         b < num_total_frames_to_test / num_output_buffer_size_frames; ++b) {
      EXPECT_EQ(
          num_output_buffer_size_frames,
          unpartitioner_->GetBuffer(
              &output_vector[b * num_output_buffer_size_frames * kNumChannels],
              kNumChannels, num_output_buffer_size_frames));
    }

    AudioBuffer input(kNumChannels, num_total_frames_to_test);
    FillAudioBuffer(input_audio_vector_, kNumChannels, &input);

    AudioBuffer output(kNumChannels, num_total_frames_to_test);
    FillAudioBuffer(output_vector, kNumChannels, &output);

    for (size_t channel = 0; channel < kNumChannels; ++channel) {
      EXPECT_TRUE(
          CompareAudioBuffers(input[channel], output[channel], buffer_epsilon));
    }

    EXPECT_EQ(unpartitioner_->GetNumBuffersRequestedForNumInputFrames(
                  num_total_frames_to_test),
              num_callback_calls_);
    return num_callback_calls_;
  }

  // Returns the number of triggered callback calls.
  template <typename T>
  size_t TestPlanarBufferOutputTest(size_t input_buffer_size_frames,
                                    size_t num_output_buffer_size_frames,
                                    size_t num_total_frames_to_test,
                                    float buffer_epsilon) {
    InitBufferUnpartitioner(num_total_frames_to_test, input_buffer_size_frames);

    std::vector<std::vector<T>> planar_output_vector(
        kNumChannels,
        std::vector<T>(num_total_frames_to_test, static_cast<T>(0)));
    std::vector<T*> planar_output_vector_ptrs(kNumChannels);
    for (size_t channel = 0; channel < kNumChannels; ++channel) {
      planar_output_vector_ptrs[channel] = &planar_output_vector[channel][0];
    }

    const size_t num_total_buffers =
        num_total_frames_to_test / num_output_buffer_size_frames;
    for (size_t buffer = 0; buffer < num_total_buffers; ++buffer) {
      EXPECT_EQ(num_output_buffer_size_frames,
                unpartitioner_->GetBuffer(planar_output_vector_ptrs.data(),
                                          kNumChannels,
                                          num_output_buffer_size_frames));
      for (T*& planar_output_vector_ptr : planar_output_vector_ptrs) {
        planar_output_vector_ptr += num_output_buffer_size_frames;
      }
    }
    AudioBuffer input(kNumChannels, num_total_frames_to_test);
    FillAudioBuffer(input_audio_vector_, kNumChannels, &input);

    AudioBuffer output(kNumChannels, num_total_frames_to_test);
    for (size_t channel = 0; channel < kNumChannels; ++channel) {
      ConvertPlanarSamples(num_total_frames_to_test,
                           &planar_output_vector[channel][0],
                           &output[channel][0]);
    }

    for (size_t channel = 0; channel < kNumChannels; ++channel) {
      EXPECT_TRUE(
          CompareAudioBuffers(input[channel], output[channel], buffer_epsilon));
    }

    EXPECT_EQ(unpartitioner_->GetNumBuffersRequestedForNumInputFrames(
                  num_total_frames_to_test),
              num_callback_calls_);
    return num_callback_calls_;
  }

  size_t num_callback_calls_;
  AudioBuffer input_audio_buffer_;
  std::vector<int16> input_audio_vector_;

  std::unique_ptr<BufferUnpartitioner> unpartitioner_;
};

TEST_F(BufferUnpartitionerTest, TestInterleavedBufferOutputTest) {
  const size_t kNumInputBuffers = 8;
  EXPECT_EQ(kNumInputBuffers,
            TestInterleavedBufferOutputTest<int16>(
                kNumFramesPerBuffer / kNumInputBuffers /* input_buffer_size */,
                kNumFramesPerBuffer /* output_buffer_size */,
                kNumFramesPerBuffer /* num_frames_to_test */,
                1e-4f /* buffer_epsilon */));

  EXPECT_EQ(kNumInputBuffers,
            TestInterleavedBufferOutputTest<float>(
                kNumFramesPerBuffer / kNumInputBuffers /* input_buffer_size */,
                kNumFramesPerBuffer /* output_buffer_size */,
                kNumFramesPerBuffer /* num_frames_to_test */,
                1e-6f /* buffer_epsilon */));

  EXPECT_EQ(1U,  // Single callback expected.
            TestInterleavedBufferOutputTest<int16>(
                kNumFramesPerBuffer /* input_buffer_size */,
                kNumFramesPerBuffer / kNumInputBuffers /* output_buffer_size */,
                kNumFramesPerBuffer /* num_frames_to_test */,
                1e-4f /* buffer_epsilon */));

  EXPECT_EQ(1U,  // Single callback expected.
            TestInterleavedBufferOutputTest<float>(
                kNumFramesPerBuffer /* input_buffer_size */,
                kNumFramesPerBuffer / kNumInputBuffers /* output_buffer_size */,
                kNumFramesPerBuffer /* num_frames_to_test */,
                1e-6f /* buffer_epsilon */));
}

TEST_F(BufferUnpartitionerTest, TestPlanarBufferOutputTest) {
  const size_t kNumInputBuffers = 8;
  EXPECT_EQ(kNumInputBuffers,
            TestPlanarBufferOutputTest<int16>(
                kNumFramesPerBuffer / kNumInputBuffers /* input_buffer_size */,
                kNumFramesPerBuffer /* output_buffer_size */,
                kNumFramesPerBuffer /* num_frames_to_test */,
                5e-3f /* buffer_epsilon */));

  EXPECT_EQ(kNumInputBuffers,
            TestPlanarBufferOutputTest<float>(
                kNumFramesPerBuffer / kNumInputBuffers /* input_buffer_size */,
                kNumFramesPerBuffer /* output_buffer_size */,
                kNumFramesPerBuffer /* num_frames_to_test */,
                1e-6f /* buffer_epsilon */));

  EXPECT_EQ(1U,  // Single callback expected.
            TestPlanarBufferOutputTest<int16>(
                kNumFramesPerBuffer /* input_buffer_size */,
                kNumFramesPerBuffer / kNumInputBuffers /* output_buffer_size */,
                kNumFramesPerBuffer /* num_frames_to_test */,
                5e-3f /* buffer_epsilon */));

  EXPECT_EQ(1U,  // Single callback expected.
            TestPlanarBufferOutputTest<float>(
                kNumFramesPerBuffer /* input_buffer_size */,
                kNumFramesPerBuffer / kNumInputBuffers /* output_buffer_size */,
                kNumFramesPerBuffer /* num_frames_to_test */,
                1e-6f /* buffer_epsilon */));
}

TEST_F(BufferUnpartitionerTest, GetNumBuffersRequestedForNumInputFramesTest) {
  AudioBuffer input_audio_buffer(kNumChannels, kNumFramesPerBuffer);
  size_t num_callback_calls = 0;
  const auto input_callback = [this, &input_audio_buffer,
                               &num_callback_calls]() -> AudioBuffer* {
    ++num_callback_calls;
    return &input_audio_buffer;
  };
  unpartitioner_.reset(new BufferUnpartitioner(
      kNumChannels, kNumFramesPerBuffer, input_callback));

  EXPECT_EQ(unpartitioner_->GetNumBuffersRequestedForNumInputFrames(0), 0U);
  EXPECT_EQ(unpartitioner_->GetNumBuffersRequestedForNumInputFrames(1), 1U);
  EXPECT_EQ(unpartitioner_->GetNumBuffersRequestedForNumInputFrames(
                kNumFramesPerBuffer - 1),
            1U);
  EXPECT_EQ(unpartitioner_->GetNumBuffersRequestedForNumInputFrames(
                kNumFramesPerBuffer),
            1U);
  EXPECT_EQ(unpartitioner_->GetNumBuffersRequestedForNumInputFrames(
                kNumFramesPerBuffer + 1),
            2U);

  const auto GetLargeRandomFrameNumber = [](size_t i) -> size_t {
    return i * 7 * kNumFramesPerBuffer + 13 * i;
  };

  const size_t kMaximumInputSize =
      kNumChannels * GetLargeRandomFrameNumber(kNumFramesPerBuffer * 2);
  std::vector<float> input(kMaximumInputSize);

  for (size_t i = 0; i < kNumFramesPerBuffer * 2; ++i) {
    // Reset unpartitioner.
    unpartitioner_->Clear();

    // Simulate initial read of |i| frames.
    EXPECT_EQ(i, unpartitioner_->GetBuffer(&input[0], kNumChannels, i));

    const size_t large_random_frame_number = GetLargeRandomFrameNumber(i);
    const size_t expected_num_buffer_requests =
        unpartitioner_->GetNumBuffersRequestedForNumInputFrames(
            large_random_frame_number);

    num_callback_calls = 0;
    EXPECT_EQ(large_random_frame_number,
              unpartitioner_->GetBuffer(&input[0], kNumChannels,
                                        large_random_frame_number));

    EXPECT_EQ(expected_num_buffer_requests, num_callback_calls);
  }
}

}  // namespace vraudio
