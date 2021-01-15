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

#include "dsp/fir_filter.h"

#include "third_party/googletest/googletest/include/gtest/gtest.h"
#include "base/audio_buffer.h"
#include "base/constants_and_types.h"
#include "base/simd_macros.h"

namespace vraudio {

namespace {

const size_t kFirFramesPerBuffer = 32;
const size_t kFirKernelLength = 8;

// Tests that the filter is resized to a length compatable with the current
// SIMD_LENGTH whether it is one greater than or one less than a multiple of
// SIMD_LENGTH to begin with.
TEST(FirFilterTest, GetFilterLengthTest) {
  const size_t twice_simd_length = (SIMD_LENGTH * 2);
  // Divisible by the length of a SIMD vector.
  AudioBuffer kernel_buffer1(kNumMonoChannels, twice_simd_length);
  FirFilter filter1(kernel_buffer1[0], twice_simd_length);
  EXPECT_EQ(twice_simd_length, filter1.filter_length());

  // Shorter filter.
  AudioBuffer kernel_buffer2(kNumMonoChannels, twice_simd_length - 1);
  FirFilter filter2(kernel_buffer2[0], twice_simd_length - 1);
  EXPECT_EQ(twice_simd_length, filter2.filter_length());

  // Longer filter.
  AudioBuffer kernel_buffer3(kNumMonoChannels, twice_simd_length + 1);
  FirFilter filter3(kernel_buffer3[0], twice_simd_length + 1);
  EXPECT_EQ(twice_simd_length + SIMD_LENGTH, filter3.filter_length());
}

// Tests that the output from a convolution between a double kronecker input and
// a alternating 1, 0 buffer of input yields the correct output buffer.
TEST(FirFilterTest, ProcessWithDoubleKronecker) {
  AudioBuffer input_buffer(kNumMonoChannels, kFirFramesPerBuffer);
  input_buffer.Clear();
  AudioBuffer output_buffer(1, kFirFramesPerBuffer);
  output_buffer.Clear();
  // First create a vector containing the FIR filter coefficients in standard
  // format.
  AudioBuffer kernel_buffer(kNumMonoChannels, kFirKernelLength);
  kernel_buffer.Clear();
  kernel_buffer[0][0] = 1.0f;
  kernel_buffer[0][kFirKernelLength / 2] = 1.0f;
  // Now we can create the kernel buffer in its repeated entry representation
  // from this standard form FIR representation.
  FirFilter filter(kernel_buffer[0], kFirFramesPerBuffer);
  // Next populate the input buffer.
  for (size_t i = 0; i < kFirFramesPerBuffer; i += 2) {
    input_buffer[0][i] = 1.0f;
  }

  filter.Process(input_buffer[0], &(output_buffer[0]));

  for (size_t i = 0; i < kFirFramesPerBuffer; ++i) {
    if (i % 2 != 0) {
      EXPECT_NEAR(0.0f, output_buffer[0][i], kEpsilonFloat);
    } else if (i <= 3) {
      EXPECT_NEAR(1.0f, output_buffer[0][i], kEpsilonFloat);
    } else {
      EXPECT_NEAR(2.0f, output_buffer[0][i], kEpsilonFloat);
    }
  }

  // Run again with a cleared buffer to flush out the filter state.
  input_buffer.Clear();
  output_buffer.Clear();

  filter.Process(input_buffer[0], &(output_buffer[0]));

  for (size_t i = 0; i < kFirFramesPerBuffer; ++i) {
    if (i % 2 != 0) {
      EXPECT_NEAR(0.0f, output_buffer[0][i], kEpsilonFloat);
    } else if (i <= 3) {
      EXPECT_NEAR(1.0f, output_buffer[0][i], kEpsilonFloat);
    } else {
      EXPECT_NEAR(0.0f, output_buffer[0][i], kEpsilonFloat);
    }
  }
}

}  // namespace

}  // namespace vraudio
