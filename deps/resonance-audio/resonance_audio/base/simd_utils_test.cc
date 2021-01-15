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

#include "base/simd_utils.h"

#include <algorithm>
#include <cmath>
#include <cstdint>
#include <vector>

#include "third_party/googletest/googletest/include/gtest/gtest.h"
#include "base/audio_buffer.h"
#include "base/constants_and_types.h"

namespace vraudio {

namespace {

// Input lengths (purposefully chosen not to be a multiple of SIMD_LENGTH).
const size_t kInputSize = 7;
const size_t kNumTestChannels = 3;
const size_t kNumQuadChannels = 4;

// Length of deinterleaved and interleaved buffers.
const size_t kHalfSize = 21;
const size_t kFullSize = kHalfSize * 2;
const size_t kQuadSize = kHalfSize * 4;
const size_t kPentSize = kHalfSize * 5;

// The int16 values for the deinterleaving test.
const int16_t kOne = 0x0001;
const int16_t kTwo = 0x0002;
const int16_t kThree = 0x0003;
const int16_t kFour = 0x0004;
const int16_t kMax = 0x7FFF;
const int16_t kMin = -0x7FFF;

// Epsilon for conversion from int16_t back to float.
const float kFloatEpsilon = 1e-4f;
const int16_t kIntEpsilon = 1;

// Intereleaved data.
const int16_t kInterleavedInput[kFullSize] = {
    kOne, kTwo, kOne, kTwo, kOne, kTwo, kOne, kTwo, kOne, kTwo, kOne,
    kTwo, kOne, kTwo, kOne, kTwo, kOne, kTwo, kOne, kTwo, kOne, kTwo,
    kOne, kTwo, kOne, kTwo, kOne, kTwo, kOne, kTwo, kOne, kTwo, kOne,
    kTwo, kOne, kTwo, kOne, kTwo, kOne, kTwo, kOne, kTwo};

// Corresponding values for float and 16 bit int.
const float kFloatInput[kInputSize] = {0.5f, -0.5f, 1.0f, -1.0f,
                                       1.0f, -1.0f, 0.0f};
const int16_t kIntInput[kInputSize] = {0x4000, -0x4000, 0x7FFF, -0x7FFF,
                                       0x7FFF, -0x7FFF, 0};

TEST(SimdUtilsTest, IsAlignedTest) {
  AudioBuffer aligned_audio_buffer(kNumMonoChannels, kInputSize);
  const float* aligned_ptr = aligned_audio_buffer[0].begin();
  const float* unaligned_ptr = aligned_ptr + 1;
  EXPECT_TRUE(IsAligned(aligned_ptr));
  EXPECT_FALSE(IsAligned(unaligned_ptr));
}

TEST(SimdUtilsTest, AddPointwiseTest) {
  const float kResult = 3.0f;
  AudioBuffer aligned_audio_buffer(kNumTestChannels, kInputSize);
  aligned_audio_buffer.Clear();
  for (size_t i = 0; i < kInputSize; ++i) {
    aligned_audio_buffer[0][i] = 1.0f;
    aligned_audio_buffer[1][i] = 2.0f;
  }
  AddPointwise(kInputSize, &aligned_audio_buffer[0][0],
               &aligned_audio_buffer[1][0], &aligned_audio_buffer[2][0]);
  for (size_t i = 0; i < kInputSize; ++i) {
    EXPECT_FLOAT_EQ(aligned_audio_buffer[2][i], kResult);
  }
}

TEST(SimdUtilsTest, AddPointwiseInPlaceTest) {
  AudioBuffer aligned_audio_buffer(kNumStereoChannels, kInputSize);
  aligned_audio_buffer.Clear();
  for (size_t i = 0; i < kInputSize; ++i) {
    aligned_audio_buffer[0][i] = static_cast<float>(i);
  }
  const size_t kRuns = 2;
  for (size_t i = 0; i < kRuns; ++i) {
    AddPointwise(kInputSize, &aligned_audio_buffer[0][0],
                 &aligned_audio_buffer[1][0], &aligned_audio_buffer[1][0]);
  }
  for (size_t i = 0; i < kInputSize; ++i) {
    EXPECT_FLOAT_EQ(aligned_audio_buffer[1][i], static_cast<float>(i * kRuns));
  }
}

TEST(SimdUtilsTest, SubtractPointwiseTest) {
  AudioBuffer aligned_audio_buffer(kNumStereoChannels, kInputSize);
  aligned_audio_buffer.Clear();
  for (size_t i = 0; i < kInputSize; ++i) {
    aligned_audio_buffer[0][i] = static_cast<float>(i);
    aligned_audio_buffer[1][i] = static_cast<float>(2 * i);
  }
  SubtractPointwise(kInputSize, &aligned_audio_buffer[0][0],
                    &aligned_audio_buffer[1][0], &aligned_audio_buffer[1][0]);
  for (size_t i = 0; i < kInputSize; ++i) {
    EXPECT_FLOAT_EQ(aligned_audio_buffer[1][i], aligned_audio_buffer[0][i]);
  }
}

TEST(SimdUtilsTest, MultiplyPointwiseTest) {
  AudioBuffer aligned_audio_buffer(kNumStereoChannels, kInputSize);
  aligned_audio_buffer.Clear();
  for (size_t i = 0; i < kInputSize; ++i) {
    aligned_audio_buffer[0][i] = static_cast<float>(i);
    aligned_audio_buffer[1][i] = static_cast<float>(i);
  }
  MultiplyPointwise(kInputSize, &aligned_audio_buffer[0][0],
                    &aligned_audio_buffer[1][0], &aligned_audio_buffer[1][0]);
  for (size_t i = 0; i < kInputSize; ++i) {
    EXPECT_FLOAT_EQ(aligned_audio_buffer[1][i],
                    aligned_audio_buffer[0][i] * aligned_audio_buffer[0][i]);
  }
}

TEST(SimdUtilsTest, MultiplyAndAccumulatePointwiseTest) {
  const float kInitialOutput = 1.0f;
  AudioBuffer aligned_input_buffer(kNumStereoChannels, kInputSize);
  aligned_input_buffer.Clear();
  AudioBuffer aligned_output_buffer(kNumMonoChannels, kInputSize);
  aligned_output_buffer.Clear();
  for (size_t i = 0; i < kInputSize; ++i) {
    aligned_input_buffer[0][i] = static_cast<float>(i);
    aligned_input_buffer[1][i] = static_cast<float>(i);
    aligned_output_buffer[0][i] = kInitialOutput;
  }
  MultiplyAndAccumulatePointwise(kInputSize, &aligned_input_buffer[0][0],
                                 &aligned_input_buffer[1][0],
                                 &aligned_output_buffer[0][0]);
  for (size_t i = 0; i < kInputSize; ++i) {
    EXPECT_FLOAT_EQ(aligned_output_buffer[0][i],
                    kInitialOutput + aligned_input_buffer[0][i] *
                                         aligned_input_buffer[1][i]);
  }
}

TEST(SimdUtilsTest, ScalarMultiplyTest) {
  AudioBuffer aligned_audio_buffer(kNumStereoChannels, kInputSize);
  aligned_audio_buffer.Clear();
  for (size_t i = 0; i < kInputSize; ++i) {
    aligned_audio_buffer[0][i] = 1.0f;
    aligned_audio_buffer[1][i] = 0.0f;
  }
  const float gain = 0.5f;
  ScalarMultiply(kInputSize, gain, &aligned_audio_buffer[0][0],
                 &aligned_audio_buffer[1][0]);
  for (size_t i = 0; i < kInputSize; ++i) {
    EXPECT_FLOAT_EQ(aligned_audio_buffer[1][i],
                    aligned_audio_buffer[0][i] * gain);
  }
}

TEST(SimdUtilsTest, ScalarMultiplyAndAccumuateTest) {
  const float kResult = 2.0f;
  AudioBuffer aligned_audio_buffer(kNumStereoChannels, kInputSize);
  aligned_audio_buffer.Clear();
  for (size_t i = 0; i < kInputSize; ++i) {
    aligned_audio_buffer[0][i] = 0.5f;
    aligned_audio_buffer[1][i] = 1.0f;
  }
  const float gain = 2.0f;
  ScalarMultiplyAndAccumulate(kInputSize, gain, &aligned_audio_buffer[0][0],
                              &aligned_audio_buffer[1][0]);
  for (size_t i = 0; i < kInputSize; ++i) {
    EXPECT_FLOAT_EQ(aligned_audio_buffer[1][i], kResult);
  }
}

TEST(SimdUtilsTest, SqrtTest) {
  const std::vector<float> kNumbers{130.0f, 13.0f,  1.3f,
                                    0.13f,  0.013f, 0.0013f};
  AudioBuffer numbers(kNumMonoChannels, kNumbers.size());
  AudioBuffer approximate(kNumMonoChannels, kNumbers.size());
  numbers[0] = kNumbers;
  const float kSqrtEpsilon = 2e-3f;

  Sqrt(kNumbers.size(), numbers[0].begin(), approximate[0].begin());

  for (size_t i = 0; i < kNumbers.size(); ++i) {
    const float actual = std::sqrt(kNumbers[i]);
    EXPECT_LT(std::abs(actual - approximate[0][i]) / actual, kSqrtEpsilon);
  }
}

TEST(SimdUtilsTest, ReciprocalSqrtTest) {
  const std::vector<float> kNumbers{130.0f, 13.0f,  1.3f,
                                    0.13f,  0.013f, 0.0013f};
  AudioBuffer numbers(kNumMonoChannels, kNumbers.size());
  AudioBuffer sqrt(kNumMonoChannels, kNumbers.size());
  AudioBuffer recip_sqrt(kNumMonoChannels, kNumbers.size());

  Sqrt(kNumbers.size(), numbers[0].begin(), sqrt[0].begin());
  ReciprocalSqrt(kNumbers.size(), numbers[0].begin(), recip_sqrt[0].begin());

  for (size_t i = 0; i < kNumbers.size(); ++i) {
    EXPECT_FLOAT_EQ(1.0f / recip_sqrt[0][i], sqrt[0][i]);
  }
}

// Tests that the correct complex magnitudes are calculated for a range of
// complex numbers with both positive and negative imaginary part.
TEST(SimdUtilsTest, ApproxComplexMagnitudeTest) {
  const size_t kFramesPerBuffer = 17;
  // Check that we are correct to within 0.5% of each value.
  const float kErrEpsilon = 5e-3f;
  AudioBuffer complex_buffer(kNumMonoChannels, 2 * kFramesPerBuffer);
  for (size_t i = 0; i < kFramesPerBuffer; ++i) {
    const size_t j = 2 * i;
    complex_buffer[0][j] = static_cast<float>(i);
    complex_buffer[0][j + 1] = ((i % 2) ? -1.0f : 1.0f) * static_cast<float>(i);
  }
  AudioBuffer magnitude_buffer(kNumMonoChannels, kFramesPerBuffer);

  ApproxComplexMagnitude(kFramesPerBuffer, complex_buffer[0].begin(),
                         magnitude_buffer[0].begin());

  for (size_t sample = 0; sample < kFramesPerBuffer; ++sample) {
    const float expected = static_cast<float>(sample) * kSqrtTwo;
    // Check its correct to within 0.5%.
    EXPECT_NEAR(magnitude_buffer[0][sample], expected, kErrEpsilon * expected);
  }
}

// Tests that the ComplexInterleavedFormatFromMagnitudeAndSinCosPhase() method
// correctly recovers the frequency response from magnitude and phase.
TEST(SimdUtilsTest, ComplexInterleavedFormatFromMagnitudeAndSinCosPhaseTest) {
  // The folowing vectors contain the inverse sines and cosines of the numbers
  // 0 to 0.75 in steps of 0.05 (calculated in MATLAB).
  const size_t kLength = 16;
  AudioBuffer cos_vec(kNumMonoChannels, kLength);
  cos_vec[0] = {1.5708f, 1.5208f, 1.4706f, 1.4202f, 1.3694f, 1.3181f,
                1.2661f, 1.2132f, 1.1593f, 1.1040f, 1.0472f, 0.9884f,
                0.9273f, 0.8632f, 0.7954f, 0.7227f};
  AudioBuffer sin_vec(kNumMonoChannels, kLength);
  sin_vec[0] = {0.0000f, 0.0500f, 0.1002f, 0.1506f, 0.2014f, 0.2527f,
                0.3047f, 0.3576f, 0.4115f, 0.4668f, 0.5236f, 0.5824f,
                0.6435f, 0.7076f, 0.7754f, 0.8481f};
  const float kMagnitude = 10.0f;
  AudioBuffer magnitude(kNumMonoChannels, kLength);
  std::fill(magnitude[0].begin(), magnitude[0].end(), kMagnitude);
  const size_t output_size = 2 * sin_vec.num_frames();
  AudioBuffer output(kNumMonoChannels, output_size);
  output.Clear();

  ComplexInterleavedFormatFromMagnitudeAndSinCosPhase(
      output_size, &magnitude[0][0], &cos_vec[0][0], &sin_vec[0][0],
      &output[0][0]);

  for (size_t i = 0, j = 0; i < output_size; i += 2, ++j) {
    EXPECT_FLOAT_EQ(output[0][i], kMagnitude * cos_vec[0][j]);
    EXPECT_FLOAT_EQ(output[0][i + 1], kMagnitude * sin_vec[0][j]);
  }
}

TEST(SimdUtilsTest, StereoMonoTest) {
  const float kResult = 2.0f / std::sqrt(2.0f);
  AudioBuffer aligned_audio_buffer(kNumTestChannels, kInputSize);
  aligned_audio_buffer.Clear();
  for (size_t i = 0; i < kInputSize; ++i) {
    aligned_audio_buffer[0][i] = 1.0f;
    aligned_audio_buffer[1][i] = 1.0f;
  }
  MonoFromStereoSimd(kInputSize, &aligned_audio_buffer[0][0],
                     &aligned_audio_buffer[1][0], &aligned_audio_buffer[2][0]);
  for (size_t i = 0; i < kInputSize; ++i) {
    EXPECT_FLOAT_EQ(aligned_audio_buffer[2][i], kResult);
  }
  // Perform inverse operation.
  StereoFromMonoSimd(kInputSize, &aligned_audio_buffer[2][0],
                     &aligned_audio_buffer[0][0], &aligned_audio_buffer[1][0]);
  for (size_t i = 0; i < kInputSize; ++i) {
    EXPECT_FLOAT_EQ(aligned_audio_buffer[0][i], 1.0f);
    EXPECT_FLOAT_EQ(aligned_audio_buffer[1][i], 1.0f);
  }
}

TEST(SimdUtilsTest, InterleaveAlignedInt16Test) {
  AudioBuffer::AlignedInt16Vector interleaved(kFullSize);
  AudioBuffer::AlignedInt16Vector channel_0(kHalfSize);
  AudioBuffer::AlignedInt16Vector channel_1(kHalfSize);

  // Fill the aligned input buffer.
  for (size_t i = 0; i < kHalfSize; ++i) {
    channel_0[i] = kOne;
    channel_1[i] = kTwo;
  }

  InterleaveStereo(kHalfSize, channel_0.data(), channel_1.data(),
                   interleaved.data());

  for (size_t i = 0; i < kFullSize; ++i) {
    const int16_t value = (i % 2 == 0) ? kOne : kTwo;
    EXPECT_EQ(interleaved[i], value);
  }
}

TEST(SimdUtilsTest, InterleaveUnalignedInt16Test) {
  AudioBuffer::AlignedInt16Vector interleaved(kFullSize);
  AudioBuffer::AlignedInt16Vector channel_0(kHalfSize);
  AudioBuffer::AlignedInt16Vector channel_1(kHalfSize);

  // Fill the aligned input buffer.
  for (size_t i = 0; i < kHalfSize; ++i) {
    channel_0[i] = kOne;
    channel_1[i] = kTwo;
  }

  InterleaveStereo(kHalfSize, channel_0.data(), channel_1.data(),
                   interleaved.data());

  for (size_t i = 0; i < kFullSize; ++i) {
    const int16_t value = (i % 2 == 0) ? kOne : kTwo;
    EXPECT_EQ(interleaved[i], value);
  }
}

TEST(SimdUtilsTest, DeinterleaveAlignedInt16Test) {
  AudioBuffer::AlignedInt16Vector interleaved(kFullSize);
  AudioBuffer::AlignedInt16Vector channel_0(kHalfSize);
  AudioBuffer::AlignedInt16Vector channel_1(kHalfSize);

  // Fill the aligned input buffer.
  for (size_t i = 0; i < kFullSize; ++i) {
    interleaved[i] = kInterleavedInput[i];
  }

  // Clear the output buffers.
  for (size_t i = 0; i < kHalfSize; ++i) {
    channel_0[i] = static_cast<int16_t>(0);
    channel_1[i] = static_cast<int16_t>(0);
  }

  // Test the case where input is aligned.
  DeinterleaveStereo(kHalfSize, interleaved.data(), channel_0.data(),
                     channel_1.data());
  for (size_t i = 0; i < kHalfSize; ++i) {
    EXPECT_EQ(channel_0[i], kOne);
    EXPECT_EQ(channel_1[i], kTwo);
  }
}

TEST(SimdUtilsTest, DeinterleaveUnalignedInt16Test) {
  AudioBuffer::AlignedInt16Vector channel_0(kHalfSize);
  AudioBuffer::AlignedInt16Vector channel_1(kHalfSize);

  // Clear the output buffers.
  for (size_t i = 0; i < kHalfSize; ++i) {
    channel_0[i] = static_cast<int16_t>(0);
    channel_1[i] = static_cast<int16_t>(0);
  }

  // Test the case where input is unaligned.
  DeinterleaveStereo(kHalfSize, kInterleavedInput, channel_0.data(),
                     channel_1.data());
  for (size_t i = 0; i < kHalfSize; ++i) {
    EXPECT_EQ(channel_0[i], kOne);
    EXPECT_EQ(channel_1[i], kTwo);
  }
}

TEST(SimdUtilsTest, DeinterleaveAlignedInt16ConvertToFloatTest) {
  AudioBuffer::AlignedInt16Vector interleaved(kFullSize);
  AudioBuffer planar(kNumStereoChannels, kHalfSize);
  AudioBuffer::Channel& channel_0 = planar[0];
  AudioBuffer::Channel& channel_1 = planar[1];

  // Fill the aligned input buffer.
  for (size_t i = 0; i < kFullSize; ++i) {
    interleaved[i] = i % 2 ? kMin : kMax;
  }

  // Clear the output buffers.
  planar.Clear();

  // Test the case where input is aligned.
  DeinterleaveStereo(kHalfSize, interleaved.data(), channel_0.begin(),
                     channel_1.begin());
  for (size_t i = 0; i < kHalfSize; ++i) {
    EXPECT_NEAR(channel_0[i], 1.0f, kEpsilonFloat);
    EXPECT_NEAR(channel_1[i], -1.0f, kEpsilonFloat);
  }
}

TEST(SimdUtilsTest, DeinterleaveUnalignedInt16ConvertToFloatTest) {
  int16_t interleaved[kFullSize];
  AudioBuffer planar(kNumStereoChannels, kHalfSize);
  AudioBuffer::Channel& channel_0 = planar[0];
  AudioBuffer::Channel& channel_1 = planar[1];

  // Fill the unaligned input buffer.
  for (size_t i = 0; i < kFullSize; ++i) {
    interleaved[i] = i % 2 ? kMin : kMax;
  }

  // Clear the output buffers.
  planar.Clear();

  // Test the case where input is unaligned.
  DeinterleaveStereo(kHalfSize, interleaved, channel_0.begin(),
                     channel_1.begin());

  for (size_t i = 0; i < kHalfSize; ++i) {
    EXPECT_NEAR(channel_0[i], 1.0f, kEpsilonFloat);
    EXPECT_NEAR(channel_1[i], -1.0f, kEpsilonFloat);
  }
}

TEST(SimdUtilsTest, InterleaveAlignedFloatTest) {
  AudioBuffer interleaved(kNumMonoChannels, kFullSize);
  AudioBuffer planar(kNumStereoChannels, kHalfSize);
  AudioBuffer::Channel& channel_0 = planar[0];
  AudioBuffer::Channel& channel_1 = planar[1];

  // Fill the aligned input buffer.
  for (size_t i = 0; i < kHalfSize; ++i) {
    channel_0[i] = 1.0f;
    channel_1[i] = 2.0f;
  }

  InterleaveStereo(kHalfSize, channel_0.begin(), channel_1.begin(),
                   interleaved[0].begin());

  for (size_t i = 0; i < kFullSize; ++i) {
    const float value = (i % 2 == 0) ? 1.0f : 2.0f;
    EXPECT_FLOAT_EQ(interleaved[0][i], value);
  }
}

TEST(SimdUtilsTest, InterleaveUnalignedFloatTest) {
  float interleaved[kFullSize];
  AudioBuffer planar(kNumStereoChannels, kHalfSize);
  AudioBuffer::Channel& channel_0 = planar[0];
  AudioBuffer::Channel& channel_1 = planar[1];

  // Fill the aligned input buffer.
  for (size_t i = 0; i < kHalfSize; ++i) {
    channel_0[i] = 1.0f;
    channel_1[i] = 2.0f;
  }

  InterleaveStereo(kHalfSize, channel_0.begin(), channel_1.begin(),
                   interleaved);

  for (size_t i = 0; i < kFullSize; ++i) {
    const float value = (i % 2 == 0) ? 1.0f : 2.0f;
    EXPECT_FLOAT_EQ(interleaved[i], value);
  }
}

TEST(SimdUtilsTest, InterleaveAlignedFloatConvertToInt16Test) {
  AudioBuffer::AlignedInt16Vector interleaved(kFullSize);
  AudioBuffer planar(kNumStereoChannels, kHalfSize);
  AudioBuffer::Channel& channel_0 = planar[0];
  AudioBuffer::Channel& channel_1 = planar[1];

  // Fill the aligned input buffer.
  for (size_t i = 0; i < kHalfSize; ++i) {
    channel_0[i] = 1.0f;
    channel_1[i] = -1.0f;
  }

  InterleaveStereo(kHalfSize, channel_0.begin(), channel_1.begin(),
                   interleaved.data());

  for (size_t i = 0; i < kFullSize; ++i) {
    const int16_t value = i % 2 ? kMin : kMax;
    EXPECT_NEAR(interleaved[i], value, kIntEpsilon);
  }
}

TEST(SimdUtilsTest, InterleaveUnalignedFloatConvertToInt16Test) {
  int16_t interleaved[kFullSize];
  AudioBuffer planar(kNumStereoChannels, kHalfSize);
  AudioBuffer::Channel& channel_0 = planar[0];
  AudioBuffer::Channel& channel_1 = planar[1];

  // Fill the aligned input buffer.
  for (size_t i = 0; i < kHalfSize; ++i) {
    channel_0[i] = 1.0f;
    channel_1[i] = -1.0f;
  }

  InterleaveStereo(kHalfSize, channel_0.begin(), channel_1.begin(),
                   interleaved);

  for (size_t i = 0; i < kFullSize; ++i) {
    const int16_t value = i % 2 ? kMin : kMax;
    EXPECT_NEAR(interleaved[i], value, kIntEpsilon);
  }
}

TEST(SimdUtilsTest, DeinterleaveAlignedFloatTest) {
  AudioBuffer interleaved(kNumMonoChannels, kFullSize);
  AudioBuffer planar(kNumStereoChannels, kHalfSize);
  AudioBuffer::Channel& channel_0 = planar[0];
  AudioBuffer::Channel& channel_1 = planar[1];

  // Fill the aligned input buffer.
  for (size_t i = 0; i < kFullSize; ++i) {
    interleaved[0][i] = (i % 2 == 0) ? 1.0f : 2.0f;
  }

  // Clear the output buffers.
  planar.Clear();

  // Test the case where input is aligned.
  DeinterleaveStereo(kHalfSize, interleaved[0].begin(), channel_0.begin(),
                     channel_1.begin());
  for (size_t i = 0; i < kHalfSize; ++i) {
    EXPECT_FLOAT_EQ(channel_0[i], 1.0f);
    EXPECT_FLOAT_EQ(channel_1[i], 2.0f);
  }
}

TEST(SimdUtilsTest, DeinterleaveUnalignedFloatTest) {
  float interleaved[kFullSize];
  AudioBuffer planar(kNumStereoChannels, kHalfSize);
  AudioBuffer::Channel& channel_0 = planar[0];
  AudioBuffer::Channel& channel_1 = planar[1];

  // Fill the aligned input buffer.
  for (size_t i = 0; i < kFullSize; ++i) {
    interleaved[i] = (i % 2 == 0) ? 1.0f : 2.0f;
  }

  // Clear the output buffers.
  planar.Clear();

  // Test the case where input is unaligned.
  DeinterleaveStereo(kHalfSize, interleaved, channel_0.begin(),
                     channel_1.begin());
  for (size_t i = 0; i < kHalfSize; ++i) {
    EXPECT_FLOAT_EQ(channel_0[i], 1.0f);
    EXPECT_FLOAT_EQ(channel_1[i], 2.0f);
  }
}

TEST(SimdUtilsTest, InterleaveQuadInt16Test) {
  AudioBuffer::AlignedInt16Vector interleaved(kQuadSize);
  AudioBuffer::AlignedInt16Vector workspace(kPentSize);
  AudioBuffer::AlignedInt16Vector channel_0(kHalfSize);
  AudioBuffer::AlignedInt16Vector channel_1(kHalfSize);
  AudioBuffer::AlignedInt16Vector channel_2(kHalfSize);
  AudioBuffer::AlignedInt16Vector channel_3(kHalfSize);

  // Fill the aligned input buffer.
  for (size_t i = 0; i < kHalfSize; ++i) {
    channel_0[i] = kOne;
    channel_1[i] = kTwo;
    channel_2[i] = kThree;
    channel_3[i] = kFour;
  }

  InterleaveQuad(kHalfSize, channel_0.data(), channel_1.data(),
                 channel_2.data(), channel_3.data(), workspace.data(),
                 interleaved.data());

  for (size_t i = 0; i < kQuadSize; ++i) {
    const int16_t value = static_cast<int16_t>(1 + (i % kNumQuadChannels));
    EXPECT_FLOAT_EQ(interleaved[i], value);
  }
}

TEST(SimdUtilsTest, DeinterleaveQuadInt16Test) {
  AudioBuffer::AlignedInt16Vector interleaved(kQuadSize);
  AudioBuffer::AlignedInt16Vector workspace(kPentSize);
  AudioBuffer::AlignedInt16Vector channel_0(kHalfSize);
  AudioBuffer::AlignedInt16Vector channel_1(kHalfSize);
  AudioBuffer::AlignedInt16Vector channel_2(kHalfSize);
  AudioBuffer::AlignedInt16Vector channel_3(kHalfSize);

  // Fill the aligned input buffer.
  for (size_t i = 0; i < kQuadSize; ++i) {
    interleaved[i] = static_cast<int16_t>(1 + (i % kNumQuadChannels));
  }

  // Clear the output buffers.
  for (size_t i = 0; i < kHalfSize; ++i) {
    channel_0[i] = static_cast<int16_t>(0);
    channel_1[i] = static_cast<int16_t>(0);
    channel_2[i] = static_cast<int16_t>(0);
    channel_3[i] = static_cast<int16_t>(0);
  }

  // Test the case where input is aligned.
  DeinterleaveQuad(kHalfSize, interleaved.data(), workspace.data(),
                   channel_0.data(), channel_1.data(), channel_2.data(),
                   channel_3.data());

  for (size_t i = 0; i < kHalfSize; ++i) {
    EXPECT_EQ(channel_0[i], kOne);
    EXPECT_EQ(channel_1[i], kTwo);
    EXPECT_EQ(channel_2[i], kThree);
    EXPECT_EQ(channel_3[i], kFour);
  }
}

TEST(SimdUtilsTest, InterleaveQuadFloatTest) {
  AudioBuffer interleaved(kNumMonoChannels, kQuadSize);
  AudioBuffer workspace(kNumMonoChannels, kPentSize);
  AudioBuffer planar(kNumQuadChannels, kHalfSize);
  AudioBuffer::Channel& channel_0 = planar[0];
  AudioBuffer::Channel& channel_1 = planar[1];
  AudioBuffer::Channel& channel_2 = planar[2];
  AudioBuffer::Channel& channel_3 = planar[3];

  // Fill the aligned input buffer.
  for (size_t i = 0; i < kHalfSize; ++i) {
    channel_0[i] = 1.0f;
    channel_1[i] = 2.0f;
    channel_2[i] = 3.0f;
    channel_3[i] = 4.0f;
  }

  InterleaveQuad(kHalfSize, channel_0.begin(), channel_1.begin(),
                 channel_2.begin(), channel_3.begin(), workspace[0].begin(),
                 interleaved[0].begin());

  for (size_t i = 0; i < kQuadSize; ++i) {
    const float value = static_cast<float>(1 + (i % kNumQuadChannels));
    EXPECT_FLOAT_EQ(interleaved[0][i], value);
  }
}

TEST(SimdUtilsTest, DeinterleaveQuadFloatTest) {
  AudioBuffer interleaved(kNumMonoChannels, kQuadSize);
  AudioBuffer workspace(kNumMonoChannels, kPentSize);
  AudioBuffer planar(kNumQuadChannels, kHalfSize);
  AudioBuffer::Channel& channel_0 = planar[0];
  AudioBuffer::Channel& channel_1 = planar[1];
  AudioBuffer::Channel& channel_2 = planar[2];
  AudioBuffer::Channel& channel_3 = planar[3];

  // Fill the aligned input buffer.
  for (size_t i = 0; i < kQuadSize; ++i) {
    interleaved[0][i] = static_cast<float>(1 + (i % kNumQuadChannels));
  }

  // Clear the output buffers.
  planar.Clear();

  // Test the case where input is aligned.
  DeinterleaveQuad(kHalfSize, interleaved[0].begin(), workspace[0].begin(),
                   channel_0.begin(), channel_1.begin(), channel_2.begin(),
                   channel_3.begin());

  for (size_t i = 0; i < kHalfSize; ++i) {
    EXPECT_FLOAT_EQ(channel_0[i], 1.0f);
    EXPECT_FLOAT_EQ(channel_1[i], 2.0f);
    EXPECT_FLOAT_EQ(channel_2[i], 3.0f);
    EXPECT_FLOAT_EQ(channel_3[i], 4.0f);
  }
}

TEST(SimdUtilsTest, Int16FromFloatTest) {
  AudioBuffer float_buffer(kNumMonoChannels, kInputSize);
  float_buffer.Clear();

  AudioBuffer::AlignedInt16Vector int_buffer(kInputSize);

  for (size_t i = 0; i < kInputSize; ++i) {
    float_buffer[0][i] = kFloatInput[i];
  }

  Int16FromFloat(kInputSize, &(float_buffer[0][0]), int_buffer.data());

  for (size_t i = 0; i < kInputSize; ++i) {
    EXPECT_NEAR(int_buffer[i], kIntInput[i], kIntEpsilon);
  }
}

TEST(SimdUtilsTest, FloatFromInt16Test) {
  AudioBuffer float_buffer(kNumMonoChannels, kInputSize);
  float_buffer.Clear();

  AudioBuffer::AlignedInt16Vector int_buffer(kInputSize);

  for (size_t i = 0; i < kInputSize; ++i) {
    int_buffer[i] = static_cast<int16_t>(kIntInput[i]);
  }

  FloatFromInt16(kInputSize, int_buffer.data(), &(float_buffer[0][0]));

  for (size_t i = 0; i < kInputSize; i += 2) {
    EXPECT_NEAR(float_buffer[0][i], kFloatInput[i], kFloatEpsilon);
  }
}

}  // namespace

}  // namespace vraudio
