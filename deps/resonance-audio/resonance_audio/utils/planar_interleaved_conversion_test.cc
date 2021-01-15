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

#include "utils/planar_interleaved_conversion.h"

#include <algorithm>

#include "third_party/googletest/googletest/include/gtest/gtest.h"
#include "base/audio_buffer.h"
#include "base/constants_and_types.h"
#include "base/misc_math.h"

namespace vraudio {

namespace {

// Epsilon for conversion from int16_t back to float.
const float kFloatEpsilon = 1e-4f;
const int16_t kIntEpsilon = 1;

const int kMemoryAlignmentBytesInt = static_cast<int>(kMemoryAlignmentBytes);

const size_t kMaxNumFrames = 16;
const size_t kMaxNumChannels = 8;

int16_t SingleFloatToInt16(float value) {
  const float kInt16Min = static_cast<float>(-0x7FFF);
  const float kInt16Max = static_cast<float>(0x7FFF);
  const float scaled_value = value * kInt16Max;
  const float clamped_value =
      std::min(kInt16Max, std::max(kInt16Min, scaled_value));
  return static_cast<int16_t>(clamped_value);
}

// Creates a trivial channel map, i.e. no mapping.
std::vector<size_t> GetTrivialChannelMap(size_t size) {
  std::vector<size_t> channel_map(size);
  for (size_t i = 0; i < size; ++i) {
    channel_map[i] = i;
  }
  return channel_map;
}

// Fills an interleaved buffer with the channel number / 10, in each frame
// (converted to integer format).
void FillInterleaved(size_t num_channels, size_t num_frames, int16_t* buffer) {
  for (size_t f = 0; f < num_frames; ++f) {
    for (size_t c = 0; c < num_channels; ++c) {
      buffer[f * num_channels + c] =
          SingleFloatToInt16(static_cast<float>(c) * 0.1f);
    }
  }
}

// Fills an interleaved buffer with the channel number / 10, in each frame.
void FillInterleaved(size_t num_channels, size_t num_frames, float* buffer) {
  for (size_t f = 0; f < num_frames; ++f) {
    for (size_t c = 0; c < num_channels; ++c) {
      buffer[f * num_channels + c] = static_cast<float>(c) * 0.1f;
    }
  }
}

// Fills a planar buffer with the channel number / 10, in each frame.
void FillPlanar(AudioBuffer* buffer) {
  for (size_t c = 0; c < buffer->num_channels(); ++c) {
    std::fill_n(&(*buffer)[c][0], buffer->num_frames(),
                static_cast<float>(c) * 0.1f);
  }
}

// Fills a planar buffer with the channel number / 10, in each frame.
void FillPlanar(size_t num_frames, std::vector<float*>* buffer) {
  for (size_t c = 0; c < buffer->size(); ++c) {
    std::fill_n((*buffer)[c], num_frames, static_cast<float>(c) * 0.1f);
  }
}

// Fills a planar buffer with the channel number / 10, in each frame (converted
// to integer format).
void FillPlanar(size_t num_frames, int16_t** buffer, size_t num_channels) {
  for (size_t c = 0; c < num_channels; ++c) {
    std::fill_n(buffer[c], num_frames,
                SingleFloatToInt16(static_cast<float>(c) * 0.1f));
  }
}

// Fills a planar buffer with the channel number / 10, in each frame (converted
// to integer format).
void FillPlanar(size_t num_frames, std::vector<int16_t*>* buffer) {
  FillPlanar(num_frames, buffer->data(), buffer->size());
}

// Verifies that the output and expected output match.
void VerifyOutput(size_t num_channels, size_t num_frames,
                  const std::vector<int16_t*>& expected_output,
                  const std::vector<int16_t*>& output) {
  for (size_t c = 0; c < num_channels; ++c) {
    for (size_t f = 0; f < num_frames; ++f) {
      EXPECT_NEAR(expected_output[c][f], output[c][f], kIntEpsilon);
    }
  }
}

// Verifies that the output and expected output match.
template <typename InputType, typename OutputType>
void VerifyOutputFloatTemplated(size_t num_channels, size_t num_frames,
                                const InputType& expected_output,
                                const OutputType& output,
                                const std::vector<size_t>& channel_map,
                                size_t output_offset) {
  for (size_t c = 0; c < num_channels; ++c) {
    for (size_t f = output_offset; f < output_offset + num_frames; ++f) {
      EXPECT_NEAR(expected_output[channel_map[c]][f], output[c][f],
                  kFloatEpsilon);
    }
  }
}

// Verifies that the output and expected output match.
void VerifyOutput(size_t num_channels, size_t num_frames,
                  const AudioBuffer& expected_output,
                  const AudioBuffer& output) {
  VerifyOutputFloatTemplated<AudioBuffer, AudioBuffer>(
      num_channels, num_frames, expected_output, output,
      GetTrivialChannelMap(num_channels), 0);
}

// Verifies that the output and expected output match.
void VerifyOutput(size_t num_channels, size_t num_frames,
                  const std::vector<float*>& expected_output,
                  const AudioBuffer& output) {
  VerifyOutputFloatTemplated<std::vector<float*>, AudioBuffer>(
      num_channels, num_frames, expected_output, output,
      GetTrivialChannelMap(num_channels), 0);
}

// Verifies that the output and expected output match.
void VerifyOutput(size_t num_channels, size_t num_frames,
                  const AudioBuffer& expected_output,
                  const std::vector<float*>& output) {
  VerifyOutputFloatTemplated<AudioBuffer, std::vector<float*>>(
      num_channels, num_frames, expected_output, output,
      GetTrivialChannelMap(num_channels), 0);
}

// Verifies that the output and expected output match.
void VerifyOutput(size_t num_channels, size_t num_frames,
                  const std::vector<float*>& expected_output,
                  const std::vector<float*>& output) {
  VerifyOutputFloatTemplated<std::vector<float*>, std::vector<float*>>(
      num_channels, num_frames, expected_output, output,
      GetTrivialChannelMap(num_channels), 0);
}

// Verifies that the output and expected output match.
void VerifyOutput(size_t num_channels, size_t num_frames,
                  const std::vector<const float*>& expected_output,
                  const std::vector<const float*>& output) {
  VerifyOutputFloatTemplated<std::vector<const float*>,
                             std::vector<const float*>>(
      num_channels, num_frames, expected_output, output,
      GetTrivialChannelMap(num_channels), 0);
}

// Verifies that the output and expected output match.
void VerifyOutput(size_t num_channels, size_t num_frames,
                  const AudioBuffer& expected_output, const AudioBuffer& output,
                  const std::vector<size_t>& channel_map) {
  VerifyOutputFloatTemplated<AudioBuffer, AudioBuffer>(
      num_channels, num_frames, expected_output, output, channel_map, 0);
}

// Verifies that the output and expected output match.
void VerifyOutput(size_t num_channels, size_t num_frames, size_t output_offset,
                  const AudioBuffer& expected_output,
                  const AudioBuffer& output) {
  VerifyOutputFloatTemplated<AudioBuffer, AudioBuffer>(
      num_channels, num_frames, expected_output, output,
      GetTrivialChannelMap(num_channels), output_offset);
}

// Verifies that the output and expected output match.
void VerifyOutput(size_t length, const std::vector<float>& expected_output,
                  int16_t* output) {
  for (size_t f = 0; f < length; ++f) {
    EXPECT_NEAR(SingleFloatToInt16(expected_output[f]), output[f], kIntEpsilon);
  }
}

// Verifies that the output and expected output match.
void VerifyOutput(size_t length, const std::vector<float>& expected_output,
                  const std::vector<int16_t>& output) {
  for (size_t f = 0; f < length; ++f) {
    EXPECT_NEAR(SingleFloatToInt16(expected_output[f]), output[f], kIntEpsilon);
  }
}

// Verifies that the output and expected output match.
void VerifyOutput(size_t length, const std::vector<float>& expected_output,
                  float* output) {
  for (size_t f = 0; f < length; ++f) {
    EXPECT_NEAR(expected_output[f], output[f], kIntEpsilon);
  }
}

// Verifies that the output and expected output match.
void VerifyOutput(size_t length, const std::vector<float>& expected_output,
                  const std::vector<float>& output) {
  for (size_t f = 0; f < length; ++f) {
    EXPECT_NEAR(expected_output[f], output[f], kIntEpsilon);
  }
}

typedef std::tuple<size_t, size_t> TestParams;
class PlanarInterleavedConverterTest
    : public ::testing::Test,
      public ::testing::WithParamInterface<TestParams> {
 protected:
  PlanarInterleavedConverterTest() {}

  // Virtual methods from ::testing::Test
  ~PlanarInterleavedConverterTest() override {}

  void SetUp() override {}

  void TearDown() override {}
};

// Tests that interleaved (float/int16_t) data can be correctly written into
// vectors of float pointers.
TEST_P(PlanarInterleavedConverterTest, TestInterleavedIntoVectorFloatPtr) {
  const size_t num_channels = ::testing::get<0>(GetParam());
  const size_t num_frames = ::testing::get<1>(GetParam());
  const size_t num_interleaved_samples = num_frames * num_channels;

  AudioBuffer expected_output(kMaxNumChannels, num_frames);
  FillPlanar(&expected_output);

  // Create output buffers memory.
  AudioBuffer aligned_output_buffer(kMaxNumChannels, num_frames);
  std::vector<float> unaligned_output_memory(num_interleaved_samples);

  // Create output planar buffers.
  std::vector<float*> aligned_output(num_channels);
  std::vector<float*> unaligned_output(num_channels);
  for (size_t c = 0; c < num_channels; ++c) {
    aligned_output[c] = &aligned_output_buffer[c][0];
    unaligned_output[c] = &unaligned_output_memory[num_frames * c];
  }

  // Integer.
  AudioBuffer::AlignedInt16Vector interleaved_aligned_int(
      num_interleaved_samples);
  std::vector<int16_t> interleaved_unaligned_int(num_interleaved_samples);
  FillInterleaved(num_channels, num_frames, interleaved_aligned_int.data());
  FillInterleaved(num_channels, num_frames, &interleaved_unaligned_int[0]);

  // Aligned Input, Aligned Output.
  aligned_output_buffer.Clear();
  LOG(INFO) << "aligned_output.size()" << aligned_output.size();
  LOG(INFO) << "num_frames" << num_frames;
  PlanarFromInterleaved(interleaved_aligned_int.data(), num_frames,
                        num_channels, aligned_output, num_frames);
  VerifyOutput(num_channels, std::min(num_frames, num_frames), expected_output,
               aligned_output);

  // Unaligned Input, Aligned Output.
  aligned_output_buffer.Clear();
  PlanarFromInterleaved(interleaved_unaligned_int.data(), num_frames,
                        num_channels, aligned_output, num_frames);
  VerifyOutput(num_channels, std::min(num_frames, num_frames), expected_output,
               aligned_output);

  // Aligned Input. Unaligned Output.
  std::fill(unaligned_output_memory.begin(), unaligned_output_memory.end(),
            0.0f);
  PlanarFromInterleaved(interleaved_aligned_int.data(), num_frames,
                        num_channels, unaligned_output, num_frames);
  VerifyOutput(num_channels, std::min(num_frames, num_frames), expected_output,
               unaligned_output);
  // Unligned Input. Unaligned Output.
  std::fill(unaligned_output_memory.begin(), unaligned_output_memory.end(),
            0.0f);
  PlanarFromInterleaved(&interleaved_unaligned_int[0], num_frames, num_channels,
                        unaligned_output, num_frames);
  VerifyOutput(num_channels, std::min(num_frames, num_frames), expected_output,
               unaligned_output);

  // Floating point.
  AudioBuffer::AlignedFloatVector interleaved_aligned_float(
      num_interleaved_samples);
  std::vector<float> interleaved_plane_float(num_interleaved_samples);
  FillInterleaved(num_channels, num_frames, interleaved_aligned_float.data());
  FillInterleaved(num_channels, num_frames, &interleaved_plane_float[0]);
  // Aligned Input, Aligned Output.
  aligned_output_buffer.Clear();
  PlanarFromInterleaved(interleaved_aligned_float.data(), num_frames,
                        num_channels, aligned_output, num_frames);
  VerifyOutput(num_channels, std::min(num_frames, num_frames), expected_output,
               aligned_output);
  // Unaligned Input, Aligned Output.
  aligned_output_buffer.Clear();
  PlanarFromInterleaved(&interleaved_plane_float[0], num_frames, num_channels,
                        aligned_output, num_frames);
  VerifyOutput(num_channels, std::min(num_frames, num_frames), expected_output,
               aligned_output);
  // Aligned Input. Unaligned Output.
  std::fill(unaligned_output_memory.begin(), unaligned_output_memory.end(),
            0.0f);
  PlanarFromInterleaved(interleaved_aligned_float.data(), num_frames,
                        num_channels, unaligned_output, num_frames);
  VerifyOutput(num_channels, std::min(num_frames, num_frames), expected_output,
               unaligned_output);
  // Unligned Input. Unaligned Output.
  std::fill(unaligned_output_memory.begin(), unaligned_output_memory.end(),
            0.0f);
  PlanarFromInterleaved(&interleaved_plane_float[0], num_frames, num_channels,
                        unaligned_output, num_frames);
  VerifyOutput(num_channels, std::min(num_frames, num_frames), expected_output,
               unaligned_output);
}

// Tests that interleaved (float/int16_t) data can be correctly written into
// |AudioBuffer|s.
TEST_P(PlanarInterleavedConverterTest, TestInterleavedIntoAudioBuffer) {
  const size_t num_channels = ::testing::get<0>(GetParam());
  const size_t num_frames = ::testing::get<1>(GetParam());
  const size_t num_interleaved_samples = num_frames * num_channels;

  AudioBuffer expected_output(num_channels, num_frames);
  FillPlanar(&expected_output);

  // Create output buffers memory.
  AudioBuffer output_buffer(num_channels, num_frames);

  // Integer.
  AudioBuffer::AlignedInt16Vector interleaved_aligned_int(
      num_interleaved_samples);
  std::vector<int16_t> interleaved_unaligned_int(num_interleaved_samples);
  FillInterleaved(num_channels, num_frames, interleaved_aligned_int.data());
  FillInterleaved(num_channels, num_frames, &interleaved_unaligned_int[0]);
  // Aligned Input, Aligned Output.
  output_buffer.Clear();
  FillAudioBuffer(interleaved_aligned_int.data(), num_frames, num_channels,
                  &output_buffer);
  VerifyOutput(num_channels, std::min(num_frames, num_frames), expected_output,
               output_buffer);
  // Unaligned Input, Aligned Output.
  output_buffer.Clear();
  FillAudioBuffer(&interleaved_unaligned_int[0], num_frames, num_channels,
                  &output_buffer);
  VerifyOutput(num_channels, std::min(num_frames, num_frames), expected_output,
               output_buffer);

  // Floating point.
  AudioBuffer::AlignedFloatVector interleaved_aligned_float(
      num_interleaved_samples);
  std::vector<float> interleaved_plane_float(num_interleaved_samples);
  FillInterleaved(num_channels, num_frames, interleaved_aligned_float.data());
  FillInterleaved(num_channels, num_frames, &interleaved_plane_float[0]);
  // Aligned Input, Aligned Output.
  output_buffer.Clear();
  FillAudioBuffer(interleaved_aligned_float.data(), num_frames, num_channels,
                  &output_buffer);
  VerifyOutput(num_channels, std::min(num_frames, num_frames), expected_output,
               output_buffer);
  // Unaligned Input, Aligned Output.
  output_buffer.Clear();
  FillAudioBuffer(&interleaved_plane_float[0], num_frames, num_channels,
                  &output_buffer);
  VerifyOutput(num_channels, std::min(num_frames, num_frames), expected_output,
               output_buffer);
}

// Tests that interleaved (float/int16_t) vectors can be correctly written into
// |AudioBuffer|s.
TEST_P(PlanarInterleavedConverterTest, TestInterleavedVectorIntoAudioBuffer) {
  const size_t num_channels = ::testing::get<0>(GetParam());
  const size_t num_frames = ::testing::get<1>(GetParam());
  const size_t num_interleaved_samples = num_frames * num_channels;

  AudioBuffer expected_output(num_channels, num_frames);
  FillPlanar(&expected_output);

  // Create output buffers memory.
  AudioBuffer output_buffer(num_channels, num_frames);

  // Integer.
  std::vector<int16_t> interleaved_unaligned_int(num_interleaved_samples);
  FillInterleaved(num_channels, num_frames, &interleaved_unaligned_int[0]);
  // Unaligned Input, Aligned Output.
  output_buffer.Clear();
  FillAudioBuffer(&interleaved_unaligned_int[0], num_frames, num_channels,
                  &output_buffer);
  VerifyOutput(num_channels, std::min(num_frames, num_frames), expected_output,
               output_buffer);

  // Floating point.
  std::vector<float> interleaved_plane_float(num_interleaved_samples);
  FillInterleaved(num_channels, num_frames, &interleaved_plane_float[0]);
  // Unaligned Input, Aligned Output.
  output_buffer.Clear();
  FillAudioBuffer(&interleaved_plane_float[0], num_frames, num_channels,
                  &output_buffer);
  VerifyOutput(num_channels, std::min(num_frames, num_frames), expected_output,
               output_buffer);
}

// Tests that vectors of planar (float/int16_t) pointers can be correctly
// written into |AudioBuffer|s.
TEST_P(PlanarInterleavedConverterTest, TestPlanarPtrsIntoAudioBuffer) {
  const size_t num_channels = ::testing::get<0>(GetParam());
  const size_t num_frames = ::testing::get<1>(GetParam());

  AudioBuffer expected_output(kMaxNumChannels, num_frames);
  FillPlanar(&expected_output);

  // Create output buffers memory.
  AudioBuffer output_buffer(num_channels, num_frames);

  // Integer.
  std::vector<AudioBuffer::AlignedInt16Vector> planar_aligned_channels_int_buf(
      num_channels, AudioBuffer::AlignedInt16Vector(num_frames));
  std::vector<std::vector<int16_t>> planar_unaligned_channels_int_buffers(
      num_channels, std::vector<int16_t>(num_frames));
  std::vector<int16_t*> planar_aligned_channels_int(num_channels);
  std::vector<int16_t*> planar_unaligned_channels_int(num_channels);
  for (size_t c = 0; c < num_channels; ++c) {
    planar_aligned_channels_int[c] = planar_aligned_channels_int_buf[c].data();
    planar_unaligned_channels_int[c] =
        planar_unaligned_channels_int_buffers[c].data();
  }
  FillPlanar(num_frames, &planar_aligned_channels_int);
  FillPlanar(num_frames, &planar_unaligned_channels_int);
  // Aligned Input, Aligned Output.
  output_buffer.Clear();
  FillAudioBuffer(&planar_aligned_channels_int[0], num_frames, num_channels,
                  &output_buffer);
  VerifyOutput(num_channels, std::min(num_frames, num_frames), expected_output,
               output_buffer);
  // Unaligned Input, Aligned Output.
  output_buffer.Clear();
  FillAudioBuffer(&planar_unaligned_channels_int[0], num_frames, num_channels,
                  &output_buffer);
  VerifyOutput(num_channels, std::min(num_frames, num_frames), expected_output,
               output_buffer);

  // Floating point.
  std::vector<AudioBuffer::AlignedFloatVector>
      planar_aligned_channels_float_buf(
          num_channels, AudioBuffer::AlignedFloatVector(num_frames));
  std::vector<std::vector<float>> planar_unaligned_channels_float_buffers(
      num_channels, std::vector<float>(num_frames));
  std::vector<float*> planar_aligned_channels_float(num_channels);
  std::vector<float*> planar_unaligned_channels_float(num_channels);
  for (size_t c = 0; c < num_channels; ++c) {
    planar_aligned_channels_float[c] =
        planar_aligned_channels_float_buf[c].data();
    planar_unaligned_channels_float[c] =
        planar_unaligned_channels_float_buffers[c].data();
  }
  FillPlanar(num_frames, &planar_aligned_channels_float);
  FillPlanar(num_frames, &planar_unaligned_channels_float);
  // Aligned Input, Aligned Output.
  output_buffer.Clear();
  FillAudioBuffer(&planar_aligned_channels_float[0], num_frames, num_channels,
                  &output_buffer);
  VerifyOutput(num_channels, std::min(num_frames, num_frames), expected_output,
               output_buffer);
  // Unaligned Input, Aligned Output.
  output_buffer.Clear();
  FillAudioBuffer(&planar_unaligned_channels_float[0], num_frames, num_channels,
                  &output_buffer);
  VerifyOutput(num_channels, std::min(num_frames, num_frames), expected_output,
               output_buffer);
}

// Tests that interleaved (float/int16_t) data can be correctly written into
// |AudioBuffer|s, with offsets into both the input and output data.
TEST_P(PlanarInterleavedConverterTest,
       TestInterleavedIntoAudioBufferWithOffset) {
  const size_t num_channels = ::testing::get<0>(GetParam());
  const size_t num_frames = ::testing::get<1>(GetParam());

  AudioBuffer expected_output(kMaxNumChannels, num_frames);
  FillPlanar(&expected_output);

  const size_t num_interleaved_samples = num_frames * num_channels;
  // Create output buffers memory.
  AudioBuffer output_buffer(num_channels, num_frames);

  AudioBuffer::AlignedInt16Vector interleaved_aligned_int(
      num_interleaved_samples);
  std::vector<int16_t> interleaved_unaligned_int(num_interleaved_samples);
  FillInterleaved(num_channels, num_frames, interleaved_aligned_int.data());
  FillInterleaved(num_channels, num_frames, &interleaved_unaligned_int[0]);

  AudioBuffer::AlignedFloatVector interleaved_aligned_float(
      num_interleaved_samples);
  std::vector<float> interleaved_plane_float(num_interleaved_samples);
  FillInterleaved(num_channels, num_frames, interleaved_aligned_float.data());
  FillInterleaved(num_channels, num_frames, &interleaved_plane_float[0]);

  for (size_t output_offset = 1; output_offset <= 4; ++output_offset) {
    for (size_t input_offset = 1; input_offset <= 4; ++input_offset) {
      const size_t num_frames_to_copy =
          std::min(num_frames - input_offset, num_frames - output_offset);
      // Integer.
      // Aligned Input, Aligned Output.
      output_buffer.Clear();
      FillAudioBufferWithOffset(interleaved_aligned_int.data(), num_frames,
                                num_channels, input_offset, output_offset,
                                num_frames_to_copy, &output_buffer);
      VerifyOutput(num_channels, num_frames_to_copy, output_offset,
                   expected_output, output_buffer);
      // Unaligned Input, Aligned Output.
      output_buffer.Clear();
      FillAudioBufferWithOffset(&interleaved_unaligned_int[0], num_frames,
                                num_channels, input_offset, output_offset,
                                num_frames_to_copy, &output_buffer);
      VerifyOutput(num_channels, num_frames_to_copy, output_offset,
                   expected_output, output_buffer);

      // Floating point.
      // Aligned Input, Aligned Output.
      output_buffer.Clear();
      FillAudioBufferWithOffset(interleaved_aligned_float.data(), num_frames,
                                num_channels, input_offset, output_offset,
                                num_frames_to_copy, &output_buffer);
      VerifyOutput(num_channels, num_frames_to_copy, output_offset,
                   expected_output, output_buffer);
      // Unaligned Input, Aligned Output.
      output_buffer.Clear();
      FillAudioBufferWithOffset(&interleaved_plane_float[0], num_frames,
                                num_channels, input_offset, output_offset,
                                num_frames_to_copy, &output_buffer);
      VerifyOutput(num_channels, num_frames_to_copy, output_offset,
                   expected_output, output_buffer);
    }
  }
}

// Tests that planar (float/int16_t) data can be correctly written into
// |AudioBuffer|s, with offsets into both the input and output data.
TEST_P(PlanarInterleavedConverterTest,
       TestPlanarPtrsIntoAudioBufferWithOffset) {
  const size_t num_channels = ::testing::get<0>(GetParam());
  const size_t num_frames = ::testing::get<1>(GetParam());

  AudioBuffer expected_output(num_channels, num_frames);
  FillPlanar(&expected_output);

  // Create output buffers memory.
  AudioBuffer output_buffer(num_channels, num_frames);

  std::vector<AudioBuffer::AlignedInt16Vector>
      planar_aligned_channels_int_buffers(
          num_channels, AudioBuffer::AlignedInt16Vector(num_frames));
  std::vector<std::vector<int16_t>> planar_unaligned_channels_int_buffers(
      num_channels, std::vector<int16_t>(num_frames));

  std::vector<int16_t*> planar_aligned_channels_int(num_channels);
  std::vector<int16_t*> planar_unaligned_channels_int(num_channels);
  for (size_t c = 0; c < num_channels; ++c) {
    planar_aligned_channels_int[c] =
        planar_aligned_channels_int_buffers[c].data();
    planar_unaligned_channels_int[c] =
        planar_unaligned_channels_int_buffers[c].data();
  }
  FillPlanar(num_frames, &planar_aligned_channels_int);
  FillPlanar(num_frames, &planar_unaligned_channels_int);

  std::vector<AudioBuffer::AlignedFloatVector>
      planar_aligned_channels_float_buffer(
          num_channels, AudioBuffer::AlignedFloatVector(num_frames));
  std::vector<std::vector<float>> planar_unaligned_channels_float_buffers(
      num_channels, std::vector<float>(num_frames));

  std::vector<float*> planar_aligned_channels_float(num_channels);
  std::vector<float*> planar_unaligned_channels_float(num_channels);

  for (size_t c = 0; c < num_channels; ++c) {
    planar_aligned_channels_float[c] =
        planar_aligned_channels_float_buffer[c].data();
    planar_unaligned_channels_float[c] =
        planar_unaligned_channels_float_buffers[c].data();
  }
  FillPlanar(num_frames, &planar_aligned_channels_float);
  FillPlanar(num_frames, &planar_unaligned_channels_float);

  for (size_t output_offset = 1; output_offset <= 4; ++output_offset) {
    for (size_t input_offset = 1; input_offset <= 4; ++input_offset) {
      const size_t num_frames_to_copy =
          std::min(num_frames - input_offset, num_frames - output_offset);
      // Integer.
      // Aligned Input, Aligned Output.
      output_buffer.Clear();
      FillAudioBufferWithOffset(&planar_aligned_channels_int[0], num_frames,
                                num_channels, input_offset, output_offset,
                                num_frames_to_copy, &output_buffer);
      VerifyOutput(num_channels, num_frames_to_copy, output_offset,
                   expected_output, output_buffer);
      // Unaligned Input, Aligned Output.
      output_buffer.Clear();
      FillAudioBufferWithOffset(&planar_unaligned_channels_int[0], num_frames,
                                num_channels, input_offset, output_offset,
                                num_frames_to_copy, &output_buffer);
      VerifyOutput(num_channels, num_frames_to_copy, output_offset,
                   expected_output, output_buffer);

      // Floating point.
      // Aligned Input, Aligned Output.
      output_buffer.Clear();
      FillAudioBufferWithOffset(&planar_aligned_channels_float[0], num_frames,
                                num_channels, input_offset, output_offset,
                                num_frames_to_copy, &output_buffer);
      VerifyOutput(num_channels, num_frames_to_copy, output_offset,
                   expected_output, output_buffer);
      // Unaligned Input, Aligned Output.
      output_buffer.Clear();
      FillAudioBufferWithOffset(&planar_unaligned_channels_float[0], num_frames,
                                num_channels, input_offset, output_offset,
                                num_frames_to_copy, &output_buffer);
      VerifyOutput(num_channels, num_frames_to_copy, output_offset,
                   expected_output, output_buffer);
    }
  }
}

// Tests that interleaved (float/int16_t) data can be correctly written into
// |AudioBuffer|s, with remapping of channels.
TEST_P(PlanarInterleavedConverterTest,
       TestInterleavedIntoAudioBufferRemapping) {
  const size_t num_channels = ::testing::get<0>(GetParam());
  const size_t num_frames = ::testing::get<1>(GetParam());

  AudioBuffer expected_output(num_channels, num_frames);
  FillPlanar(&expected_output);

  std::vector<size_t> channel_map(num_channels);
  for (size_t c = 0; c < num_channels; ++c) {
    channel_map[c] = c;
  }

  // Permute the channel map.
  std::next_permutation(channel_map.begin(), channel_map.end());
  const size_t num_interleaved_samples = num_frames * num_channels;

  // Create output buffers memory.
  AudioBuffer output_buffer(num_channels, num_frames);

  // Integer.
  AudioBuffer::AlignedInt16Vector interleaved_aligned_int(
      num_interleaved_samples);
  std::vector<int16_t> interleaved_unaligned_int(num_interleaved_samples);
  FillInterleaved(num_channels, num_frames, interleaved_aligned_int.data());
  FillInterleaved(num_channels, num_frames, &interleaved_unaligned_int[0]);
  // Aligned Input, Aligned Output.
  output_buffer.Clear();
  FillAudioBufferWithChannelRemapping(interleaved_aligned_int.data(),
                                      num_frames, num_channels, channel_map,
                                      &output_buffer);
  VerifyOutput(num_channels, num_frames, expected_output, output_buffer,
               channel_map);
  // Unaligned Input, Aligned Output.
  output_buffer.Clear();
  FillAudioBufferWithChannelRemapping(&interleaved_unaligned_int[0], num_frames,
                                      num_channels, channel_map,
                                      &output_buffer);
  VerifyOutput(num_channels, num_frames, expected_output, output_buffer,
               channel_map);

  // Floating point.
  AudioBuffer::AlignedFloatVector interleaved_aligned_float(
      num_interleaved_samples);
  std::vector<float> interleaved_plane_float(num_interleaved_samples);
  FillInterleaved(num_channels, num_frames, interleaved_aligned_float.data());
  FillInterleaved(num_channels, num_frames, &interleaved_plane_float[0]);
  // Aligned Input, Aligned Output.
  output_buffer.Clear();
  FillAudioBufferWithChannelRemapping(interleaved_aligned_float.data(),
                                      num_frames, num_channels, channel_map,
                                      &output_buffer);
  VerifyOutput(num_channels, num_frames, expected_output, output_buffer,
               channel_map);
  // Unaligned Input, Aligned Output.
  output_buffer.Clear();
  FillAudioBufferWithChannelRemapping(&interleaved_plane_float[0], num_frames,
                                      num_channels, channel_map,
                                      &output_buffer);
  VerifyOutput(num_channels, num_frames, expected_output, output_buffer,
               channel_map);
}

// Tests that planar (float/int16_t) data can be correctly written into
// |AudioBuffer|s, with remapping of channels.
TEST_P(PlanarInterleavedConverterTest, TestPlanarPtrsIntoAudioBufferRemapping) {
  const size_t num_channels = ::testing::get<0>(GetParam());
  const size_t num_frames = ::testing::get<1>(GetParam());

  AudioBuffer expected_output(num_channels, num_frames);
  FillPlanar(&expected_output);

  std::vector<size_t> channel_map(num_channels);
  for (size_t c = 0; c < num_channels; ++c) {
    channel_map[c] = c;
  }
  // Permute the channel map.
  std::next_permutation(channel_map.begin(), channel_map.end());
  // Create output buffers memory.
  AudioBuffer output_buffer(num_channels, num_frames);

  // Integer.
  std::vector<AudioBuffer::AlignedInt16Vector>
      planar_aligned_channels_int_bufferss(
          num_channels, AudioBuffer::AlignedInt16Vector(num_frames));
  std::vector<std::vector<int16_t>> planar_unaligned_channels_int_buffers(
      num_channels, std::vector<int16_t>(num_frames));

  std::vector<int16_t*> planar_aligned_channels_int(num_channels);
  std::vector<int16_t*> planar_unaligned_channels_int(num_channels);
  for (size_t c = 0; c < num_channels; ++c) {
    planar_aligned_channels_int[c] =
        planar_aligned_channels_int_bufferss[c].data();
    planar_unaligned_channels_int[c] =
        planar_unaligned_channels_int_buffers[c].data();
  }

  FillPlanar(num_frames, &planar_aligned_channels_int);
  FillPlanar(num_frames, &planar_unaligned_channels_int);
  // Aligned Input, Aligned Output.
  output_buffer.Clear();
  FillAudioBufferWithChannelRemapping(&planar_aligned_channels_int[0],
                                      num_frames, num_channels, channel_map,
                                      &output_buffer);
  VerifyOutput(num_channels, num_frames, expected_output, output_buffer,
               channel_map);
  // Unaligned Input, Aligned Output.
  output_buffer.Clear();
  FillAudioBufferWithChannelRemapping(&planar_unaligned_channels_int[0],
                                      num_frames, num_channels, channel_map,
                                      &output_buffer);
  VerifyOutput(num_channels, num_frames, expected_output, output_buffer,
               channel_map);

  // Floating point.
  std::vector<AudioBuffer::AlignedFloatVector>
      planar_aligned_channels_float_buffers(
          num_channels, AudioBuffer::AlignedFloatVector(num_frames));
  std::vector<std::vector<float>> planar_unaligned_channels_float_buffers(
      num_channels, std::vector<float>(num_frames));

  std::vector<float*> planar_aligned_channels_float(num_channels);
  std::vector<float*> planar_unaligned_channels_float(num_channels);
  for (size_t c = 0; c < num_channels; ++c) {
    planar_aligned_channels_float[c] =
        planar_aligned_channels_float_buffers[c].data();
    planar_unaligned_channels_float[c] =
        planar_unaligned_channels_float_buffers[c].data();
  }
  FillPlanar(num_frames, &planar_aligned_channels_float);
  FillPlanar(num_frames, &planar_unaligned_channels_float);
  // Aligned Input, Aligned Output.
  output_buffer.Clear();
  FillAudioBufferWithChannelRemapping(&planar_aligned_channels_float[0],
                                      num_frames, num_channels, channel_map,
                                      &output_buffer);
  VerifyOutput(num_channels, num_frames, expected_output, output_buffer,
               channel_map);
  // Unaligned Input, Aligned Output.
  output_buffer.Clear();
  FillAudioBufferWithChannelRemapping(&planar_unaligned_channels_float[0],
                                      num_frames, num_channels, channel_map,
                                      &output_buffer);
  VerifyOutput(num_channels, num_frames, expected_output, output_buffer,
               channel_map);
}

// Tests that an |AudioBuffer| can be correctly written into an interleaved
// vector of (float/int16_t) data.
TEST_P(PlanarInterleavedConverterTest, TestAudioBufferIntoInterleavedVector) {
  const size_t num_channels = ::testing::get<0>(GetParam());
  const size_t num_frames = ::testing::get<1>(GetParam());
  const size_t num_interleaved_samples = num_frames * num_channels;
  std::vector<float> expected_output(num_interleaved_samples);
  FillInterleaved(num_channels, num_frames, &expected_output[0]);

  // Floating point.
  AudioBuffer planar_input(num_channels, num_frames);
  FillPlanar(&planar_input);
  // Aligned Input, Unaligned Output.
  std::vector<float> interleaved_output;
  FillExternalBuffer(planar_input, &interleaved_output);
  VerifyOutput(num_frames * num_channels, expected_output, interleaved_output);
}

// Tests that an |AudioBuffer| can be correctly written into an interleaved
// (float/int16_t) array.
TEST_P(PlanarInterleavedConverterTest, TestAudioBufferIntoInterleavedPtr) {
  const size_t num_channels = ::testing::get<0>(GetParam());
  const size_t num_frames = ::testing::get<1>(GetParam());
  const size_t num_interleaved_samples = num_frames * num_channels;

  std::vector<float> expected_output(num_interleaved_samples);
  FillInterleaved(num_channels, num_frames, &expected_output[0]);

  AudioBuffer planar_input(num_channels, num_frames);
  FillPlanar(&planar_input);

  // Integer.
  AudioBuffer::AlignedInt16Vector interleaved_aligned_int(
      num_interleaved_samples);
  std::vector<int16_t> interleaved_unaligned_int(num_interleaved_samples);

  // Aligned Input, Aligned Output.
  DCHECK_EQ(interleaved_aligned_int.size(),
            planar_input.num_frames() * planar_input.num_channels());
  FillExternalBuffer(planar_input, interleaved_aligned_int.data(),
                     planar_input.num_frames(), planar_input.num_channels());
  VerifyOutput(num_frames * num_channels, expected_output,
               interleaved_aligned_int.data());
  // Aligned Input, Unaligned Output.
  DCHECK_EQ(interleaved_unaligned_int.size(),
            planar_input.num_frames() * planar_input.num_channels());
  FillExternalBuffer(planar_input, interleaved_unaligned_int.data(),
                     planar_input.num_frames(), planar_input.num_channels());
  VerifyOutput(num_frames * num_channels, expected_output,
               interleaved_unaligned_int);

  // Floating point.
  AudioBuffer::AlignedFloatVector interleaved_aligned_float(
      num_interleaved_samples);
  std::vector<float> interleaved_plane_float(num_interleaved_samples);

  // Aligned Input, Aligned Output.
  DCHECK_EQ(interleaved_aligned_float.size(),
            planar_input.num_frames() * planar_input.num_channels());
  FillExternalBuffer(planar_input, interleaved_aligned_float.data(),
                     planar_input.num_frames(), planar_input.num_channels());
  VerifyOutput(num_frames * num_channels, expected_output,
               interleaved_aligned_float.data());
  // Aligned Input, Unaligned Output.
  FillExternalBuffer(planar_input, &interleaved_plane_float[0],
                     planar_input.num_frames(), planar_input.num_channels());
  VerifyOutput(num_frames * num_channels, expected_output,
               interleaved_plane_float);
}

// Test Params define: channels, frames
INSTANTIATE_TEST_CASE_P(
    TestParameters, PlanarInterleavedConverterTest,
    testing::Values(
        TestParams(2, 8), TestParams(2, 13), TestParams(2, kMaxNumFrames),
        TestParams(4, 8), TestParams(4, 13), TestParams(4, kMaxNumFrames),
        TestParams(5, 8), TestParams(5, 13), TestParams(5, kMaxNumFrames),
        TestParams(kMaxNumChannels, 8), TestParams(kMaxNumChannels, 13),
        TestParams(kMaxNumChannels, kMaxNumFrames)));

}  // namespace

}  // namespace vraudio
