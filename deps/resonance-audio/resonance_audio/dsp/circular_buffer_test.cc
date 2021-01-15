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

#include "dsp/circular_buffer.h"

#include "third_party/googletest/googletest/include/gtest/gtest.h"
#include "base/constants_and_types.h"

namespace vraudio {

namespace {

// Tests that when the output from the circular buffer is of a smaller length
// than the input, the |CircularBuffer|correctly accepts and rejects inserts
// and retrievals.
TEST(CircularBufferTest, RemainingReadSpaceTest) {
  const size_t kInputSize = 5;
  const size_t kOutputSize = 4;
  const size_t kBufferSize = 9;
  CircularBuffer circular_buffer(kBufferSize, kInputSize, kOutputSize);
  AudioBuffer input_a(kNumMonoChannels, kInputSize);
  input_a[0] = {0.0f, 1.0f, 2.0f, 3.0f, 4.0f};
  AudioBuffer output(kNumMonoChannels, kOutputSize);
  output.Clear();

  EXPECT_TRUE(circular_buffer.InsertBuffer(input_a[0]));
  // There should not be enough space to write another buffer.
  EXPECT_FALSE(circular_buffer.InsertBuffer(input_a[0]));

  EXPECT_TRUE(circular_buffer.RetrieveBuffer(&output[0]));
  // There should not be enough data to read from the buffer.
  EXPECT_FALSE(circular_buffer.RetrieveBuffer(&output[0]));

  // The output buffer should contain the first |kOutputSize| entries from the
  // input buffer.
  for (size_t i = 0; i < kOutputSize; ++i) {
    EXPECT_FLOAT_EQ(output[0][i], input_a[0][i]);
  }

  AudioBuffer input_b(kNumMonoChannels, kInputSize);
  input_b[0] = {5.0f, 6.0f, 7.0f, 8.0f, 9.0f};

  EXPECT_TRUE(circular_buffer.InsertBuffer(input_b[0]));
  // There should not be enough space to write another buffer.
  EXPECT_FALSE(circular_buffer.InsertBuffer(input_b[0]));

  EXPECT_TRUE(circular_buffer.RetrieveBuffer(&output[0]));
  // There should not be enough data to read from the buffer.
  EXPECT_FALSE(circular_buffer.RetrieveBuffer(&output[0]));

  // The output buffer should contain the final entry from the first input
  // buffer and then |kOutputSize| - 1 entries from the second.
  EXPECT_FLOAT_EQ(output[0][0], input_a[0][kInputSize - 1]);
  for (size_t i = 1; i < kOutputSize; ++i) {
    EXPECT_FLOAT_EQ(output[0][i], input_b[0][i - 1]);
  }
}

// Tests that when the output from the circular buffer is of a greater length
// than the input, the |CircularBuffer|correctly accepts and rejects inserts
// and retrievals.
TEST(CircularBufferTest, RemainingWriteSpaceTest) {
  const size_t kInputSize = 4;
  const size_t kOutputSize = 5;
  const size_t kBufferSize = 10;
  CircularBuffer circular_buffer(kBufferSize, kInputSize, kOutputSize);
  AudioBuffer input_a(kNumMonoChannels, kInputSize);
  input_a[0] = {0.0f, 1.0f, 2.0f, 3.0f};
  AudioBuffer input_b(kNumMonoChannels, kInputSize);
  input_b[0] = {4.0f, 5.0f, 6.0f, 7.0f};
  AudioBuffer input_c(kNumMonoChannels, kInputSize);
  input_c[0] = {8.0f, 9.0f, 10.0f, 11.0f};
  AudioBuffer input_d(kNumMonoChannels, kInputSize);
  input_d[0] = {12.0f, 13.0f, 14.0f, 15.0f};
  AudioBuffer input_e(kNumMonoChannels, kInputSize);
  input_e[0] = {16.0f, 17.0f, 18.0f, 19.0f};
  AudioBuffer output(kNumMonoChannels, kOutputSize);
  output.Clear();

  EXPECT_TRUE(circular_buffer.InsertBuffer(input_a[0]));
  EXPECT_TRUE(circular_buffer.InsertBuffer(input_b[0]));
  // There should not be enough space to write another buffer.
  EXPECT_FALSE(circular_buffer.InsertBuffer(input_c[0]));

  EXPECT_TRUE(circular_buffer.RetrieveBuffer(&output[0]));
  // There should not be enough data to read from the buffer.
  EXPECT_FALSE(circular_buffer.RetrieveBuffer(&output[0]));

  float value = 0.0f;
  for (size_t i = 0; i < kOutputSize; ++i) {
    EXPECT_FLOAT_EQ(output[0][i], value);
    value += 1.0f;
  }

  // Add another 4 samples of input in.
  EXPECT_TRUE(circular_buffer.InsertBuffer(input_c[0]));
  // There should not be enough space to write another buffer.
  EXPECT_FALSE(circular_buffer.InsertBuffer(input_d[0]));

  EXPECT_TRUE(circular_buffer.RetrieveBuffer(&output[0]));
  // There should not be enough data to read from the buffer.
  EXPECT_FALSE(circular_buffer.RetrieveBuffer(&output[0]));

  for (size_t i = 0; i < kOutputSize; ++i) {
    EXPECT_FLOAT_EQ(output[0][i], value);
    value += 1.0f;
  }

  // Add another 8 samples of input in.
  EXPECT_TRUE(circular_buffer.InsertBuffer(input_d[0]));
  EXPECT_TRUE(circular_buffer.InsertBuffer(input_e[0]));
  // There should not be enough space to write another buffer.
  EXPECT_FALSE(circular_buffer.InsertBuffer(input_a[0]));

  // We should be able to get 10 samples of output.
  EXPECT_TRUE(circular_buffer.RetrieveBuffer(&output[0]));
  for (size_t i = 0; i < kOutputSize; ++i) {
    EXPECT_FLOAT_EQ(output[0][i], value);
    value += 1.0f;
  }
  EXPECT_TRUE(circular_buffer.RetrieveBuffer(&output[0]));
  for (size_t i = 0; i < kOutputSize; ++i) {
    EXPECT_FLOAT_EQ(output[0][i], value);
    value += 1.0f;
  }

  // There should not be enough data to read from the buffer.
  EXPECT_FALSE(circular_buffer.RetrieveBuffer(&output[0]));

  // The buffer should now be completely empty.
  EXPECT_EQ(circular_buffer.GetOccupancy(), 0U);
}

// Tests tha a call to RetrieveBuffer will work when the output buffer is
// oversized, but that only the first kOutputSize samples are filled.
TEST(CircularBufferTest, LongerWriteSpaceTest) {
  const size_t kInputSize = 5;
  const size_t kOutputSize = 3;
  const size_t kBufferSize = 10;
  CircularBuffer circular_buffer(kBufferSize, kInputSize, kOutputSize);
  AudioBuffer input_a(kNumMonoChannels, kInputSize);
  input_a[0] = {1.0f, 2.0f, 3.0f, 4.0f, 5.0f};
  AudioBuffer output(kNumMonoChannels, kBufferSize);
  output.Clear();

  EXPECT_TRUE(circular_buffer.InsertBuffer(input_a[0]));
  EXPECT_TRUE(circular_buffer.RetrieveBuffer(&output[0]));

  for (size_t i = 0; i < kBufferSize; ++i) {
    if (i < kOutputSize) {
      EXPECT_FLOAT_EQ(output[0][i], input_a[0][i]);
    } else {
      EXPECT_FLOAT_EQ(output[0][i], 0.0f);
    }
  }
}

// Tests tha a call to RetrieveBufferOffset will work when the output buffer is
// oversized, but that the first kOutputSize samples after kOffset are filled.
TEST(CircularBufferTest, OffsetRerieveTest) {
  const size_t kInputSize = 5;
  const size_t kOutputSize = 3;
  const size_t kBufferSize = 10;
  const size_t kOffset = 2;
  CircularBuffer circular_buffer(kBufferSize, kInputSize, kOutputSize);
  AudioBuffer input_a(kNumMonoChannels, kInputSize);
  input_a[0] = {1.0f, 2.0f, 3.0f, 4.0f, 5.0f};
  AudioBuffer output(kNumMonoChannels, kBufferSize);
  output.Clear();

  EXPECT_TRUE(circular_buffer.InsertBuffer(input_a[0]));
  EXPECT_TRUE(circular_buffer.RetrieveBufferWithOffset(kOffset, &output[0]));

  for (size_t i = 0; i < kBufferSize; ++i) {
    if (i < kOffset) {
      EXPECT_FLOAT_EQ(output[0][i], 0.0f);
    } else if (i < kOffset + kOutputSize) {
      EXPECT_FLOAT_EQ(output[0][i], input_a[0][i - kOffset]);
    } else {
      EXPECT_FLOAT_EQ(output[0][i], 0.0f);
    }
  }
}

// Tests that the circular buffer handles odd input buffer lengths in the
// manner that it will be passed in the spectral reverb
// (see: dsp/spectral_reverb.cc)
TEST(CircularBufferTest, Strange) {
  const size_t kNumRuns = 100;
  // An odd non pwer of two input buffer size.
  const size_t kOddInputSize = 713;
  // Output size is equal to the SpectralReverb's internal buffer size.
  const size_t kOutputSize = 1024;
  // SpectralReverb's internal forier transform length.
  const size_t kFFTSize = 4096;
  CircularBuffer input(kFFTSize + kOddInputSize, kOddInputSize, kOutputSize);
  CircularBuffer output(kOutputSize + kOddInputSize, kOutputSize,
                        kOddInputSize);
  AudioBuffer in(kNumMonoChannels, kOddInputSize);
  AudioBuffer out(kNumMonoChannels, kOutputSize);

  // AudioBuffers will be input and output in the same manner as in
  // dsp/spectral_reverb.cc.
  EXPECT_TRUE(output.InsertBuffer(out[0]));
  for (size_t i = 0; i < kNumRuns; ++i) {
    EXPECT_TRUE(input.InsertBuffer(in[0]));
    while (input.GetOccupancy() >= kOutputSize) {
      EXPECT_TRUE(input.RetrieveBuffer(&out[0]));
      EXPECT_TRUE(output.InsertBuffer(out[0]));
    }
    EXPECT_TRUE(output.RetrieveBuffer(&in[0]));
  }
}

}  // namespace

}  // namespace vraudio
