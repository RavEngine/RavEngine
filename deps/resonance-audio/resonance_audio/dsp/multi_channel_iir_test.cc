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

#include "dsp/multi_channel_iir.h"

#include <memory>
#include <vector>

#include "third_party/googletest/googletest/include/gtest/gtest.h"
#include "base/constants_and_types.h"
#include "base/logging.h"

namespace vraudio {

namespace {

const size_t kFramesPerBuffer = 4;

// Fills a channel buffer with each of the values passed |num_channels| times.
void FillChannels(const std::vector<float>& values, size_t num_channels,
                  AudioBuffer::Channel* buffer) {
  DCHECK_EQ(values.size() * num_channels, buffer->size());
  size_t offset = 0;
  for (auto value : values) {
    std::fill_n(buffer->begin() + offset, num_channels, value);
    offset += num_channels;
  }
}

// Compares the contents of the |input| to the corresponding section of the
// |expected| vector.
void CompareMultiChannelOutput(const std::vector<std::vector<float>>& expected,
                               size_t offset, size_t num_channels,
                               const AudioBuffer::Channel& input) {
  DCHECK_GE(expected[0].size(), kFramesPerBuffer + offset);
  for (size_t channel = 0; channel < num_channels; ++channel) {
    for (size_t frame = 0; frame < kFramesPerBuffer; ++frame) {
      EXPECT_NEAR(expected[channel][frame + offset],
                  input[frame * num_channels + channel], kEpsilonFloat);
    }
  }
}

// Tests that the |MultiChannelIir| can filter a single channel of input
// simultaneously with four different biquad filters.
TEST(MultiChannelIirTest, MultipleFilterSetProcessTest) {
  const std::vector<std::vector<float>> numerators({{1.0f, 3.0f, 5.0f},
                                                    {2.0f, 2.0f, 4.0f},
                                                    {1.0f, 2.0f, 2.0f},
                                                    {2.0f, 4.0f, 2.0f}});
  const std::vector<std::vector<float>> denominators({{1.0f, 1.0f, 0.0f},
                                                      {2.0f, 2.0f, 0.0f},
                                                      {1.0f, 1.0f, 0.0f},
                                                      {2.0f, 2.0f, 0.0f}});

  const std::vector<float> initial_input({1.0f, 4.0f, 6.0f, 0.0f});
  const std::vector<float> zero_input({0.0f, 0.0f, 0.0f, 0.0f});
  // These values have been determined through the MATLAB commands:
  // filter([1 3 5], [1 1 0], [1 4 6 0, 0 0 0 0, 0 0 0 0])
  // filter([2 2 4], [2 2 0], [1 4 6 0, 0 0 0 0, 0 0 0 0])
  // filter([1 2 2], [1 1 0], [1 4 6 0, 0 0 0 0, 0 0 0 0])
  // filter([2 4 2], [2 2 0], [1 4 6 0, 0 0 0 0, 0 0 0 0])
  const std::vector<std::vector<float>> kExpectedOutputs = {
      {1.0f, 6.0f, 17.0f, 21.0f, 9.0f, -9.0f, 9.0f, -9.0f, 9.0f, -9.0f, 9.0f,
       -9.0f},
      {1.0f, 4.0f, 8.0f, 6.0f, 6.0f, -6.0f, 6.0f, -6.0f, 6.0f, -6.0f, 6.0f,
       -6.0f},
      {1.0f, 5.0f, 11.0f, 9.0f, 3.0f, -3.0f, 3.0f, -3.0f, 3.0f, -3.0f, 3.0f,
       -3.0f},
      {1.0f, 5.0f, 10.0f, 6.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f,
       0.0f}};

  const size_t num_channels = numerators.size();

  AudioBuffer interleaved_buffer(kNumMonoChannels,
                                 num_channels * kFramesPerBuffer);
  std::unique_ptr<MultiChannelIir> filter = MultiChannelIir::Create(
      num_channels, kFramesPerBuffer, numerators, denominators);

  // Filter the initial buffer [1 4 6 0], and compare the output to the first
  // kFramesPerBuffer entries in |kExpectedOutputs|.
  FillChannels(initial_input, num_channels, &interleaved_buffer[0]);
  filter->Process(&(interleaved_buffer[0]));
  CompareMultiChannelOutput(kExpectedOutputs, /*offset*/ 0, num_channels,
                            interleaved_buffer[0]);

  // Filter zeros [0 0 0 0], and compare the output to the next kFramesPerBuffer
  // entries in |kExpectedOutputs|.
  FillChannels(zero_input, num_channels, &interleaved_buffer[0]);
  filter->Process(&(interleaved_buffer[0]));
  CompareMultiChannelOutput(kExpectedOutputs, kFramesPerBuffer, num_channels,
                            interleaved_buffer[0]);

  // Filter zeros [0 0 0 0], and compare the output to the next kFramesPerBuffer
  // entries in |kExpectedOutputs|.
  FillChannels(zero_input, num_channels, &interleaved_buffer[0]);
  filter->Process(&(interleaved_buffer[0]));
  CompareMultiChannelOutput(kExpectedOutputs, 2 * kFramesPerBuffer,
                            num_channels, interleaved_buffer[0]);
}

}  // namespace

}  // namespace vraudio
