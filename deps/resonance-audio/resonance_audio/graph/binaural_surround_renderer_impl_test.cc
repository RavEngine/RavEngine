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

#include "graph/binaural_surround_renderer_impl.h"

#include <algorithm>
#include <vector>

#include "third_party/googletest/googletest/include/gtest/gtest.h"
#include "base/constants_and_types.h"

namespace vraudio {

namespace {

class BinauralSurroundRendererTest
    : public ::testing::TestWithParam<
          BinauralSurroundRenderer::SurroundFormat> {
 protected:
  BinauralSurroundRendererTest() {}

  // Virtual methods from ::testing::Test
  ~BinauralSurroundRendererTest() override {}
  void SetUp() override {}
  void TearDown() override {}

  void InitBinauralSurroundRenderer(size_t frames_per_buffer,
                                    int sample_rate_hz) {
    binaural_surround_renderer_api_.reset(
        new BinauralSurroundRendererImpl(frames_per_buffer, sample_rate_hz));
  }

  // Processes an interleaved input vector and returns interleaved binaural
  // stereo output.
  std::vector<float> ProcessInterleaved(
      const std::vector<float>& interleaved_input, size_t num_channels,
      size_t frames_per_buffer) {
    EXPECT_EQ(interleaved_input.size() % (num_channels * frames_per_buffer),
              0U);
    std::vector<float> interleaved_output;
    const size_t num_buffers =
        interleaved_input.size() / (num_channels * frames_per_buffer);
    for (size_t b = 0; b < num_buffers; ++b) {
      const float* interleaved_input_ptr =
          interleaved_input.data() + b * num_channels * frames_per_buffer;
      binaural_surround_renderer_api_->AddInterleavedInput(
          interleaved_input_ptr, num_channels, frames_per_buffer);

      interleaved_output.resize((b + 1) * kNumStereoChannels *
                                frames_per_buffer);
      float* interleaved_output_ptr =
          interleaved_output.data() +
          b * kNumStereoChannels * frames_per_buffer;

      EXPECT_EQ(binaural_surround_renderer_api_->GetInterleavedStereoOutput(
                    interleaved_output_ptr, frames_per_buffer),
                frames_per_buffer);
    }
    return interleaved_output;
  }

  // Calculates the maximum absolute difference between adjacent samples in an
  // interleaved audio buffer.
  float GetMaximumSampleDiff(const std::vector<float>& interleaved_input,
                             size_t num_channels) {
    if (interleaved_input.size() <= num_channels) {
      return 0.0f;
    }

    float max_sample_diff = 0.0f;
    std::vector<float> prev_samples(num_channels);
    for (size_t i = 0; i < num_channels; ++i) {
      prev_samples[i] = interleaved_input[i];
    }
    for (size_t i = num_channels; i < interleaved_input.size(); ++i) {
      const size_t channel = i % num_channels;
      max_sample_diff =
          std::max(max_sample_diff,
                   std::abs(interleaved_input[i] - prev_samples[channel]));
      prev_samples[channel] = interleaved_input[i];
    }
    return max_sample_diff;
  }

  // Helper to return the number of input channels for a given surround format.
  size_t GetNumInputChannelsForSurroundFormat(
      BinauralSurroundRenderer::SurroundFormat format) {
    switch (format) {
      case BinauralSurroundRenderer::SurroundFormat::kSurroundMono:
        return kNumMonoChannels;
        break;
      case BinauralSurroundRenderer::SurroundFormat::kSurroundStereo:
        return kNumStereoChannels;
        break;
      case BinauralSurroundRenderer::SurroundFormat::kSurroundFiveDotOne:
        return kNumSurroundFiveDotOneChannels;
        break;
      case BinauralSurroundRenderer::SurroundFormat::kSurroundSevenDotOne:
        return kNumSurroundSevenDotOneChannels;
        break;
      case BinauralSurroundRenderer::SurroundFormat::kFirstOrderAmbisonics:
        return kNumFirstOrderAmbisonicChannels;
        break;
      case BinauralSurroundRenderer::SurroundFormat::
          kFirstOrderAmbisonicsWithNonDiegeticStereo:
        return kNumFirstOrderAmbisonicWithNonDiegeticStereoChannels;
        break;
      case BinauralSurroundRenderer::SurroundFormat::kSecondOrderAmbisonics:
        return kNumSecondOrderAmbisonicChannels;
        break;
      case BinauralSurroundRenderer::SurroundFormat::
          kSecondOrderAmbisonicsWithNonDiegeticStereo:
        return kNumSecondOrderAmbisonicWithNonDiegeticStereoChannels;
        break;
      case BinauralSurroundRenderer::SurroundFormat::kThirdOrderAmbisonics:
        return kNumThirdOrderAmbisonicChannels;
        break;
      case BinauralSurroundRenderer::SurroundFormat::
          kThirdOrderAmbisonicsWithNonDiegeticStereo:
        return kNumThirdOrderAmbisonicWithNonDiegeticStereoChannels;
        break;
      default:
        break;
    }
    LOG(FATAL) << "Unexpected format";
    return 0;
  }

  // VR Audio API instance to test.
  std::unique_ptr<BinauralSurroundRendererImpl> binaural_surround_renderer_api_;
};

// Processes an input signal with constant DC offset and scans the output for
// drop outs and noise.
TEST_P(BinauralSurroundRendererTest, DropOutGlitchTesting) {

  const std::vector<int> kTestSampleRates = {44100, 48000};
  const std::vector<int> kTestBufferSizes = {256, 413, 512};
  const size_t kNumTestBuffers = 16;
  const size_t kNumNumChannels =
      GetNumInputChannelsForSurroundFormat(GetParam());

  for (int sample_rate : kTestSampleRates) {
    for (int buffer_size : kTestBufferSizes) {
      InitBinauralSurroundRenderer(buffer_size, sample_rate);
      binaural_surround_renderer_api_->Init(GetParam());

      // Create DC input signal with magnitude 0.5f.
      const std::vector<float> interleaved_dc_signal(
          buffer_size * kNumNumChannels * kNumTestBuffers, 0.5f);

      std::vector<float> interleaved_output = ProcessInterleaved(
          interleaved_dc_signal, kNumNumChannels, buffer_size);

      // Remove first half of samples from output vector to remove initial
      // filter ringing effects and initial gain ramps.
      interleaved_output.erase(
          interleaved_output.begin(),
          interleaved_output.begin() + interleaved_output.size() / 2);

      const float kMaxExpectedMagnitudeDiff = 0.07f;
      const float maximum_adjacent_frames_magnitude_diff =
          GetMaximumSampleDiff(interleaved_output, kNumStereoChannels);
      EXPECT_LT(maximum_adjacent_frames_magnitude_diff,
                kMaxExpectedMagnitudeDiff);
    }
  }
}

INSTANTIATE_TEST_CASE_P(
    SurroundFormatInstances, BinauralSurroundRendererTest,
    ::testing::Values(
        BinauralSurroundRenderer::SurroundFormat::kSurroundMono,
        BinauralSurroundRenderer::SurroundFormat::kSurroundStereo,
        BinauralSurroundRenderer::SurroundFormat::kSurroundFiveDotOne,
        BinauralSurroundRenderer::SurroundFormat::kSurroundSevenDotOne,
        BinauralSurroundRenderer::SurroundFormat::kFirstOrderAmbisonics,
        BinauralSurroundRenderer::SurroundFormat::
            kFirstOrderAmbisonicsWithNonDiegeticStereo,
        BinauralSurroundRenderer::SurroundFormat::kSecondOrderAmbisonics,
        BinauralSurroundRenderer::SurroundFormat::
            kSecondOrderAmbisonicsWithNonDiegeticStereo,
        BinauralSurroundRenderer::SurroundFormat::kThirdOrderAmbisonics,
        BinauralSurroundRenderer::SurroundFormat::
            kThirdOrderAmbisonicsWithNonDiegeticStereo));

}  // namespace

}  // namespace vraudio
