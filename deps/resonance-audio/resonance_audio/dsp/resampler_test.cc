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

#include "dsp/resampler.h"

#include <numeric>
#include <utility>
#include <vector>

#include "third_party/googletest/googletest/include/gtest/gtest.h"
#include "base/constants_and_types.h"
#include "utils/test_util.h"

namespace vraudio {

namespace {

const int kToneFrequency = 1000;

const size_t kInputBufferLength = 441;

const int kSourceDataSampleRate = 44100;
const int kUpDestinationDataSampleRate = 48000;
const int kDownDestinationDataSampleRate = 24000;

TEST(ResamplerTest, UpSampleTest) {
  // Create input buffer with 1000Hz sine wave at 44100Hz, 441 samples long.
  // This should be ten periods of a sine wave.
  AudioBuffer input(kNumMonoChannels, kInputBufferLength);
  GenerateSineWave(kToneFrequency, kSourceDataSampleRate, &input[0]);

  Resampler resampler;
  resampler.SetRateAndNumChannels(
      kSourceDataSampleRate, kUpDestinationDataSampleRate, kNumMonoChannels);
  AudioBuffer output(kNumMonoChannels,
                     resampler.GetNextOutputLength(kInputBufferLength));
  resampler.Process(input, &output);

  // Ensure the length of the output is as expected.
  const size_t input_length_proportion =
      kSourceDataSampleRate / kInputBufferLength;
  const size_t expected_output_length =
      kUpDestinationDataSampleRate / input_length_proportion;
  EXPECT_EQ(expected_output_length, output.num_frames());

  // Ensure the output also contains 10 periods of a sine wave, confirming it is
  // still at 1000Hz given the new sampling rate.
  AudioBuffer thresholded_output(kNumMonoChannels, output.num_frames());
  for (size_t i = 0; i < output.num_frames(); ++i) {
    thresholded_output[0][i] = std::abs(output[0][i]) < 0.5f ? 0.0f : 1.0f;
  }
  size_t num_square_wave_corners = 1;
  // We have now generated a square wave with twice the number of corners as
  // there are zero crossings.
  for (size_t i = 0; i < output.num_frames() - 1; ++i) {
    if (thresholded_output[0][i] != thresholded_output[0][i + 1]) {
      num_square_wave_corners++;
    }
  }
  const size_t zerocross_count = num_square_wave_corners / 2;

  const size_t num_expected_zero_crossings =
      2 * kToneFrequency / (kSourceDataSampleRate / kInputBufferLength);
  EXPECT_EQ(num_expected_zero_crossings, zerocross_count);
}

TEST(ResamplerTest, DownSampleTest) {
  // Create input buffer with 1000Hz sine wave at 44100Hz, 441 samples long.
  // This should be ten periods of a sine wave.
  AudioBuffer input(kNumMonoChannels, kInputBufferLength);
  GenerateSineWave(kToneFrequency, kSourceDataSampleRate, &input[0]);

  Resampler resampler;
  resampler.SetRateAndNumChannels(
      kSourceDataSampleRate, kDownDestinationDataSampleRate, kNumMonoChannels);
  AudioBuffer output(kNumMonoChannels,
                     resampler.GetNextOutputLength(kInputBufferLength));
  resampler.Process(input, &output);

  // Ensure the length of the output is as expected.
  const size_t input_length_proportion =
      kSourceDataSampleRate / kInputBufferLength;
  const size_t expected_output_length =
      kDownDestinationDataSampleRate / input_length_proportion;
  EXPECT_EQ(expected_output_length, output.num_frames());

  // Ensure the output also contains one period of a sine wave, confirming it is
  // still at 100Hz given the new sampling rate.
  AudioBuffer thresholded_output(kNumMonoChannels, output.num_frames());
  for (size_t i = 0; i < output.num_frames(); ++i) {
    thresholded_output[0][i] = std::abs(output[0][i]) < 0.5f ? 0.0f : 1.0f;
  }
  int zero_cross_count = 1;
  for (size_t i = 0; i < output.num_frames() - 1; ++i) {
    if (thresholded_output[0][i] != thresholded_output[0][i + 1]) {
      zero_cross_count++;
    }
  }
  const int num_expected_zero_crossings =
      4 * kToneFrequency / (kSourceDataSampleRate / kInputBufferLength);
  EXPECT_EQ(num_expected_zero_crossings, zero_cross_count);
}

TEST(ResamplerTest, ResetStateTest) {
  // Create input buffer with 1000Hz sine wave at 44100Hz, 441 samples long.
  // This should be ten periods of a sine wave.
  AudioBuffer input(kNumMonoChannels, kInputBufferLength);
  GenerateSineWave(kToneFrequency, kSourceDataSampleRate, &input[0]);

  Resampler resampler;
  resampler.SetRateAndNumChannels(
      kSourceDataSampleRate, kUpDestinationDataSampleRate, kNumMonoChannels);
  AudioBuffer output_1(kNumMonoChannels,
                       resampler.GetNextOutputLength(kInputBufferLength));
  resampler.Process(input, &output_1);

  resampler.ResetState();
  AudioBuffer output_2(kNumMonoChannels,
                       resampler.GetNextOutputLength(kInputBufferLength));
  resampler.Process(input, &output_2);

  // If the clearing of the resampler worked properly there should be no
  // internal state between process calls and thus both outputs should be
  // identical.
  for (size_t sample = 0; sample < output_1.num_frames(); ++sample) {
    EXPECT_NEAR(output_1[0][sample], output_2[0][sample], kEpsilonFloat);
  }
}

TEST(ResamplerTest, TwoChannelTest) {
  AudioBuffer input(kNumStereoChannels, kInputBufferLength);
  GenerateSineWave(kToneFrequency, kSourceDataSampleRate, &input[0]);
  GenerateSineWave(2 * kToneFrequency, kSourceDataSampleRate, &input[1]);

  Resampler resampler;
  resampler.SetRateAndNumChannels(
      kSourceDataSampleRate, kUpDestinationDataSampleRate, kNumStereoChannels);
  AudioBuffer output(kNumStereoChannels,
                     resampler.GetNextOutputLength(kInputBufferLength));
  resampler.Process(input, &output);

  AudioBuffer thresholded_output(kNumStereoChannels, output.num_frames());
  for (size_t i = 0; i < output.num_frames(); ++i) {
    thresholded_output[0][i] = std::abs(output[0][i]) < 0.5f ? 0.0f : 1.0f;
    thresholded_output[1][i] = std::abs(output[1][i]) < 0.5f ? 0.0f : 1.0f;
  }
  int channel_0_zero_cross_count = 1;
  int channel_1_zero_cross_count = 1;
  for (size_t i = 0; i < output.num_frames() - 1; ++i) {
    if (thresholded_output[0][i] != thresholded_output[0][i + 1]) {
      channel_0_zero_cross_count++;
    }
    if (thresholded_output[1][i] != thresholded_output[1][i + 1]) {
      channel_1_zero_cross_count++;
    }
  }
  const int channel_0_expected_zero_crossings =
      4 * kToneFrequency / (kSourceDataSampleRate / kInputBufferLength);
  const int channel_1_expected_zero_crossings =
      8 * kToneFrequency / (kSourceDataSampleRate / kInputBufferLength);
  EXPECT_EQ(channel_0_zero_cross_count, channel_0_expected_zero_crossings);
  EXPECT_EQ(channel_1_zero_cross_count, channel_1_expected_zero_crossings);
}

TEST(ResamplerTest, DiracImpulseUpsampleTest) {
  const size_t kInputSignalSize = 128;
  AudioBuffer input_signal(kNumMonoChannels, kInputSignalSize);
  GenerateDiracImpulseFilter(kInputSignalSize / 2, &input_signal[0]);

  const int kSourceSamplingRate = 100;
  const int kDestinationSamplingRate = 200;

  Resampler resampler;
  resampler.SetRateAndNumChannels(kSourceSamplingRate, kDestinationSamplingRate,
                                  kNumMonoChannels);

  AudioBuffer output_signal(kNumMonoChannels,
                            resampler.GetNextOutputLength(kInputSignalSize));
  const int resampling_factor = kDestinationSamplingRate / kSourceSamplingRate;
  EXPECT_EQ(kInputSignalSize * resampling_factor, output_signal.num_frames());
  // Perform resampling. Dirac impulse position should shift according to
  // "resampling_factor".
  resampler.Process(input_signal, &output_signal);
  EXPECT_EQ(kInputSignalSize * resampling_factor, output_signal.num_frames());

  DelayCompare(input_signal[0], output_signal[0], kInputSignalSize / 2,
               kEpsilonFloat);
}

TEST(ResamplerTest, GetNextOutputLengthTest) {
  const size_t input_length = 10;
  Resampler resampler;
  resampler.SetRateAndNumChannels(kSourceDataSampleRate,
                                  kSourceDataSampleRate / 2, kNumMonoChannels);
  // In this case the source rate is twice the destination rate, we can expect
  // the output length to be half the input_length.

  resampler.SetRateAndNumChannels(kSourceDataSampleRate,
                                  kSourceDataSampleRate * 2, kNumMonoChannels);
  // In this case the source rate is half the destination rate, we can
  // expect the output length to be twice the input_length.
  EXPECT_EQ(input_length * 2, resampler.GetNextOutputLength(input_length));
}

TEST(Resampler, AreSampleRatesSupportedTest) {
  const size_t kNumPairs = 6;
  const int kSourceRates[kNumPairs] = {4000, 16000, 44100, 96000, 41999, 48000};
  const int kDestRates[kNumPairs] = {96000, 44100, 48000, 32000, 44100, 43210};
  const bool kExpectedResults[kNumPairs] = {true, true,  true,
                                            true, false, false};

  // We generally support only rates with relatively high GCD.
  for (size_t i = 0; i < kNumPairs; ++i) {
    const bool result =
        Resampler::AreSampleRatesSupported(kSourceRates[i], kDestRates[i]);
    EXPECT_EQ(result, kExpectedResults[i]);
  }
}

class OutputLengthTest : public ::testing::TestWithParam<std::pair<int, int>> {
};

// Tests that the lengths returned by the |Process| method are always equal to
// the length returned by |GetMaxOutputLength|, or one sample less.
TEST_P(OutputLengthTest, OutputLengthTest) {
  const size_t kInputSize = 512;
  Resampler resampler;
  const auto rates = GetParam();
  resampler.SetRateAndNumChannels(rates.first, rates.second, kNumMonoChannels);
  AudioBuffer input(kNumMonoChannels, kInputSize);
  const size_t max_output_length = resampler.GetMaxOutputLength(kInputSize);
  AudioBuffer output(kNumMonoChannels, max_output_length);

  // Process 100 times and expect the lengts to be maximum length or one less.
  const size_t kTimesToProcess = 100;
  for (size_t i = 0; i < kTimesToProcess; ++i) {
    const size_t next_output_length = resampler.GetNextOutputLength(kInputSize);
    resampler.Process(input, &output);
    EXPECT_LE(next_output_length, max_output_length);
    EXPECT_LE(max_output_length - next_output_length, 1U);
  }
}

INSTANTIATE_TEST_CASE_P(RatePairs, OutputLengthTest,
                        ::testing::Values(std::make_pair(44100, 48000),
                                          std::make_pair(48000, 44100)));

}  // namespace

class PolyphaseFilterTest : public ::testing::Test {
 protected:
  PolyphaseFilterTest() {}
  // Virtual methods from ::testing::Test
  ~PolyphaseFilterTest() override {}
  void SetUp() override {}
  void TearDown() override {}

  std::unique_ptr<AudioBuffer> GetPolyphaseFilter(
      const AudioBuffer::Channel& filter_coefficients, int coeffs_per_phase,
      int up_rate) {
    // Create a resampler with arbitrary source and destination sampling rates.
    Resampler resampler;
    resampler.SetRateAndNumChannels(kSourceDataSampleRate,
                                    kSourceDataSampleRate, kNumMonoChannels);
    resampler.up_rate_ = up_rate;
    resampler.transposed_filter_coeffs_.Clear();
    resampler.coeffs_per_phase_ = coeffs_per_phase;
    resampler.ArrangeFilterAsPolyphase(filter_coefficients.size(),
                                       filter_coefficients);
    std::unique_ptr<AudioBuffer> polyphase_filter(new AudioBuffer());
    *polyphase_filter = resampler.transposed_filter_coeffs_;
    return polyphase_filter;
  }
};

TEST_F(PolyphaseFilterTest, CorrectPolyphaseFilterTest) {
  // Choose an uprate which is a factor of the filters length.
  const int filter_length = 24;
  const int uprate = 4;
  const int phase_length = filter_length / uprate;

  // Create a vector of ascending numbers (1:24).
  AudioBuffer ascending(kNumMonoChannels, filter_length);
  ascending.Clear();
  std::iota(ascending[0].begin(), ascending[0].end(), 1.0f);

  std::unique_ptr<AudioBuffer> output =
      GetPolyphaseFilter(ascending[0], phase_length, uprate);

  // In polyphase format, the filter coefficients can be thought of as a matrix
  // with uprate columns. The last uprate coeffecients are in the first row and
  // the first uprate coefficients are in the last row. This is stored in a
  // vector, column by column.
  //
  // 1 2 3 4 5 6 7 8 9 10 11 12 13 14 15 16 17 18 19 20 21 22 23 24
  // becomes:
  //  21    22    23    24
  //  17    18    19    20
  //  13    14    15    16
  //   9    10    11    12
  //   5     6     7     8
  //   1     2     3     4
  size_t index = 0;
  for (int phase = uprate; phase > 0; --phase) {
    for (int value = filter_length - phase + 1; value > uprate - phase;
         value -= uprate) {
      EXPECT_EQ(static_cast<float>(value), (*output)[0][index++]);
    }
  }
}

}  // namespace vraudio
