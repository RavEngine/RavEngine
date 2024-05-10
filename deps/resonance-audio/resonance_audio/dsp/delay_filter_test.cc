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

#include "dsp/delay_filter.h"

#include "third_party/googletest/googletest/include/gtest/gtest.h"
#include "base/constants_and_types.h"

namespace vraudio {

namespace {

// Set of delay lengths to be used in the tests below.
const size_t kInitDelayLength = 4;
const size_t kSecondDelayLength = 5;
const size_t kThirdDelayLength = 2;
const size_t kFourthDelayLength = 11;
const size_t kZeroDelayLength = 0;

// Frames for buffer for the input and output data buffers below.
const size_t kFramesPerBuffer = 6;

// Frames per buffer and channel number for the input and output data.
const size_t kFramesPerBuffer2 = 10;

// Function which passes an AudioBuffer through the delay line followed by all
// zero AudioBuffers to flush the data out. This function then tests that the
// input data has been delayed by the correct amount.
//
// @param delay A pointer to a DelayFilter which will be used in the test.
// @param delay_length An integer delay length to be used in the test.
void IntegerDelayTestHelper(DelayFilter* delay, size_t delay_length) {
  std::vector<float> output_collect;
  delay->SetMaximumDelay(delay_length);
  delay->ClearBuffer();

  // Initialize mono input buffer and fill with test data.
  const std::vector<float> kData = {1.0f, 2.0f, 3.0f, 4.0f, 5.0f, 6.0f};
  AudioBuffer input(kNumMonoChannels, kFramesPerBuffer);
  auto* input_channel = &input[0];
  *input_channel = kData;

  // Process input.
  delay->InsertData(*input_channel);
  delay->GetDelayedData(delay_length, input_channel);
  output_collect.insert(output_collect.end(), input_channel->begin(),
                        input_channel->end());

  // Keep passing zeros through till we flush all of the input data out.
  while (delay_length + kFramesPerBuffer > output_collect.size()) {
    // Set the |input| AudioBuffer to have all zero data.
    input.Clear();
    // Process input.
    delay->InsertData(*input_channel);
    delay->GetDelayedData(delay_length, input_channel);
    output_collect.insert(output_collect.end(), input_channel->begin(),
                          input_channel->end());
  }

  // Check that the first GetDelayedData() call yields |delay_length| zeros at
  // the beginning of its output.
  for (size_t i = 0; i < delay_length; ++i) {
    EXPECT_EQ(output_collect[i], 0.0f);
  }
  // Check that the output is the same data as the input but delayed by
  // delay_length samples.
  for (size_t i = 0; i < kFramesPerBuffer; ++i) {
    EXPECT_EQ(output_collect[i + delay_length], kData[i]);
  }
}

// Tests that the maximum_delay_length_ and delay_buffer_size_ are set to the
// correct values on construction and when SetDelay() is called.
TEST(DelayFilterTest, DelayCorrectTest) {
  DelayFilter delay(kInitDelayLength, kFramesPerBuffer);

  EXPECT_EQ(delay.GetMaximumDelayLength(), kInitDelayLength);
  EXPECT_EQ(delay.GetDelayBufferLength(), kInitDelayLength + kFramesPerBuffer);

  delay.SetMaximumDelay(kSecondDelayLength);

  EXPECT_EQ(delay.GetMaximumDelayLength(), kSecondDelayLength);
  EXPECT_EQ(delay.GetDelayBufferLength(),
            kSecondDelayLength + kFramesPerBuffer);

  // In this next case we reduce the maximum length and thus do not expect a
  // reallocation.
  delay.SetMaximumDelay(kThirdDelayLength);

  EXPECT_EQ(delay.GetMaximumDelayLength(), kThirdDelayLength);
  EXPECT_EQ(delay.GetDelayBufferLength(),
            kSecondDelayLength + kFramesPerBuffer);

  delay.SetMaximumDelay(kFourthDelayLength);

  EXPECT_EQ(delay.GetMaximumDelayLength(), kFourthDelayLength);
  // Now we expect the buffer to have been reallocated.
  EXPECT_EQ(delay.GetDelayBufferLength(),
            kFourthDelayLength + kFramesPerBuffer);
}

// Tests whether when setting a different delay on one DelayFilter,
// the output is correct in each case.
TEST(DelayFilterTest, DelayTest) {
  DelayFilter delay(kInitDelayLength, kFramesPerBuffer);

  IntegerDelayTestHelper(&delay, kInitDelayLength);

  // Tests the case of an increasing delay.
  IntegerDelayTestHelper(&delay, kSecondDelayLength);

  // Tests the case of a decreasing delay.
  IntegerDelayTestHelper(&delay, kThirdDelayLength);

  // Tests the case of an increasing delay with allocation of more buffer space.
  IntegerDelayTestHelper(&delay, kFourthDelayLength);
}

// Tests that differently delayed buffers can be extracted from a single delay
// line.
TEST(DelayFilterTest, MultipleDelaysTest) {
  DelayFilter delay(kFramesPerBuffer2, kFramesPerBuffer2);
  AudioBuffer input(kNumMonoChannels, kFramesPerBuffer2);
  for (size_t i = 0; i < kFramesPerBuffer2; ++i) {
    input[0][i] = static_cast<float>(i + 1);
  }
  delay.InsertData(input[0]);
  const size_t kDelayOne = 1;
  const size_t kDelayTwo = 2;
  AudioBuffer buffer_1(kNumMonoChannels, kFramesPerBuffer2);
  AudioBuffer buffer_2(kNumMonoChannels, kFramesPerBuffer2);

  delay.GetDelayedData(kDelayOne, &buffer_1[0]);
  for (size_t i = 0; i < kFramesPerBuffer2 - kDelayOne; ++i) {
    EXPECT_NEAR(buffer_1[0][i + kDelayOne], input[0][i], kEpsilonFloat);
  }

  delay.GetDelayedData(kDelayTwo, &buffer_2[0]);
  for (size_t i = 0; i < kFramesPerBuffer2 - kDelayTwo; ++i) {
    EXPECT_NEAR(buffer_2[0][i + kDelayTwo], input[0][i], kEpsilonFloat);
  }
}

// Tests whether a zero delay length is dealt with correctly, Along with a
// negative delay value (treated as zero delay.
TEST(DelayFilterTest, ZeroDelayTest) {
  DelayFilter delay(kZeroDelayLength, kFramesPerBuffer);
  IntegerDelayTestHelper(&delay, kZeroDelayLength);
}

// Tests that output from a delay line that is initally large enough vs one that
// is resized is the same.
TEST(DelayFilterTest, InitialSizeVsResizeTest) {
  const size_t kSmallMaxDelay = 2;
  const size_t kLargeMaxDelay = 5;
  const size_t kActualDelay = 4;

  DelayFilter delay_sufficient(kLargeMaxDelay, kFramesPerBuffer);
  DelayFilter delay_insufficient(kSmallMaxDelay, kFramesPerBuffer);

  AudioBuffer input_buffer(kNumMonoChannels, kFramesPerBuffer);
  for (size_t i = 0; i < kFramesPerBuffer; ++i) {
    input_buffer[0][i] = static_cast<float>(i + 1);
  }

  delay_sufficient.InsertData(input_buffer[0]);
  delay_insufficient.InsertData(input_buffer[0]);
  delay_insufficient.SetMaximumDelay(kLargeMaxDelay);

  AudioBuffer buffer_sufficient(kNumMonoChannels, kFramesPerBuffer);
  AudioBuffer buffer_insufficient(kNumMonoChannels, kFramesPerBuffer);

  delay_sufficient.GetDelayedData(kActualDelay, &buffer_sufficient[0]);
  delay_insufficient.GetDelayedData(kActualDelay, &buffer_insufficient[0]);

  for (size_t i = 0; i < kFramesPerBuffer; ++i) {
    EXPECT_NEAR(buffer_sufficient[0][i], buffer_insufficient[0][i],
                kEpsilonFloat);
  }
}

}  // namespace

}  // namespace vraudio
