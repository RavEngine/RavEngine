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

#include "dsp/utils.h"

#include <cmath>
#include <limits>
#include <memory>
#include <vector>

#include "third_party/googletest/googletest/include/gtest/gtest.h"
#include "base/constants_and_types.h"
#include "dsp/fft_manager.h"
#include "dsp/partitioned_fft_filter.h"
#include "utils/test_util.h"

namespace vraudio {

namespace {

const int kSamplingRate = 48000;

const size_t kHalfHannWindowLength = 8;
const float kExpectedHalfHannWindow[] = {0.0000000f, 0.04322727f, 0.1654347f,
                                         0.3454915f, 0.55226423f, 0.7500000f,
                                         0.9045085f, 0.98907380f};

const size_t kHannWindowLength = 15;
const float kExpectedHannWindow[] = {
    0.0000000f, 0.0495156f, 0.1882551f, 0.3887395f, 0.6112605f,
    0.8117449f, 0.9504844f, 1.0000000f, 0.9504844f, 0.8117449f,
    0.6112605f, 0.3887395f, 0.1882551f, 0.0495156f, 0.0000000f};

// Test that the noise generation functions create noise vectors with the
// expected means etc.
TEST(DspUtilsTest, NoiseTest) {
  const size_t kNoiseBufferLength = 1e5;
  // A high epsilon is used to determine if mean, std dev etc. are acurate as it
  // would be infeasible to have enough samples to take advantage of the central
  // limit theorem.
  const float kEpsilon = 1e-1f;

  AudioBuffer gaussian(kNumMonoChannels, kNoiseBufferLength);
  AudioBuffer uniform(kNumMonoChannels, kNoiseBufferLength);

  // Generate Gaussian Noise with mean 0 and std deviation 2.
  const float kGaussianMean = 0.0f;
  const float kStdDeviation = 2.0f;
  GenerateGaussianNoise(kGaussianMean, kStdDeviation, /*seed=*/1U,
                        &gaussian[0]);

  // Calculate the mean and compare to that specified.
  float mean = 0.0f;
  for (auto& sample : gaussian[0]) {
    mean += sample;
  }
  mean /= static_cast<float>(gaussian.num_frames());
  EXPECT_NEAR(mean, kGaussianMean, kEpsilon);

  // Calculate the std deviation and compare to that specified.
  float std_dev = 0.0f;
  for (auto& sample : gaussian[0]) {
    std_dev += (sample - mean) * (sample - mean);
  }
  std_dev /= static_cast<float>(gaussian.num_frames());
  std_dev = std::sqrt(std_dev);
  EXPECT_NEAR(std_dev, kStdDeviation, kEpsilon);

  // Genarate uniformly distributed noise min -1, max 1 and thus mean 0.
  const float kMin = -1.0f;
  const float kMax = 1.0f;
  GenerateUniformNoise(kMin, kMax, /*seed=*/1U, &uniform[0]);
  // Calculate the mean and min/max values, compare to expected values.
  mean = 0.0f;
  float min = std::numeric_limits<float>::max();
  float max = std::numeric_limits<float>::min();
  for (auto& sample : uniform[0]) {
    mean += sample;
    min = sample < min ? sample : min;
    max = sample > max ? sample : max;
  }
  mean /= static_cast<float>(uniform.num_frames());
  EXPECT_NEAR(mean, (kMax + kMin) / 2.0f, kEpsilon);
  EXPECT_GE(kMax, max);
  EXPECT_LE(kMin, min);
}

// Tests that the ceiled input size in frames matches the expected multiple of
// frames per buffer for arbitrary inputs.
TEST(DspUtilsTest, CeilToMultipleOfFramesPerBufferTest) {
  const size_t kFramesPerBuffer = 512;
  const std::vector<size_t> kInput = {0, 100, 512, 1000, 5000, 10240};
  const std::vector<size_t> kExpectedOutput = {512,  512,  512,
                                               1024, 5120, 10240};

  for (size_t i = 0; i < kInput.size(); ++i) {
    EXPECT_EQ(kExpectedOutput[i],
              CeilToMultipleOfFramesPerBuffer(kInput[i], kFramesPerBuffer));
  }
}

// Tests that on filtering a noise sample with a pair of decorrelation filters,
// the correlation between those outputs is less than the result of an
// autocorrelation.
TEST(DspUtilsTest, GenerateDecorrelationFiltersTest) {
  // Size of FFT to be used in |GenerateDecorrelationFiltersTest|.
  const size_t kBufferSize = 512;
  // Centre frequency for noise used in |GenerateDecorrelationFiltersTest|.
  const float kNoiseCenter = 1000.0f;
  std::unique_ptr<AudioBuffer> kernels =
      GenerateDecorrelationFilters(kSamplingRate);
  AudioBuffer noise(kNumMonoChannels, kBufferSize);
  GenerateBandLimitedGaussianNoise(kNoiseCenter, kSamplingRate, /*seed=*/1U,
                                   &noise);
  FftManager fft_manager(kBufferSize);
  PartitionedFftFilter fft_filter(kernels->num_frames(), kBufferSize,
                                  &fft_manager);
  fft_filter.SetTimeDomainKernel((*kernels)[0]);
  AudioBuffer output_one(kNumMonoChannels, kBufferSize);

  PartitionedFftFilter::FreqDomainBuffer freq_domain_buffer(kNumMonoChannels,
                                                            kBufferSize * 2);
  fft_manager.FreqFromTimeDomain(noise[0], &freq_domain_buffer[0]);
  fft_filter.Filter(freq_domain_buffer[0]);
  fft_filter.GetFilteredSignal(&output_one[0]);
  fft_filter.SetTimeDomainKernel((*kernels)[1]);
  AudioBuffer output_two(kNumMonoChannels, kBufferSize);
  fft_manager.FreqFromTimeDomain(noise[0], &freq_domain_buffer[0]);
  fft_filter.Filter(freq_domain_buffer[0]);
  fft_filter.GetFilteredSignal(&output_two[0]);
  const float auto_correlation = MaxCrossCorrelation(noise[0], noise[0]);
  const float decorrelated = MaxCrossCorrelation(output_one[0], output_two[0]);
  EXPECT_LT(decorrelated, auto_correlation);
}

// Tests half-Hann window calculation against values returned by MATLAB's hann()
// function.
TEST(DspUtilsTest, GenerateHalfHannWindowTest) {
  AudioBuffer half_hann_window(kNumMonoChannels, kHalfHannWindowLength);
  GenerateHannWindow(false, kHalfHannWindowLength, &half_hann_window[0]);
  for (size_t i = 0; i < kHalfHannWindowLength; ++i) {
    EXPECT_NEAR(half_hann_window[0][i], kExpectedHalfHannWindow[i],
                kEpsilonFloat);
  }
}

// Tests Hann window generation for odd window lengths.
TEST(DspUtilsTest, GenerateHannWindowOddLengthTest) {
  AudioBuffer hann_window(kNumMonoChannels, kHannWindowLength);
  GenerateHannWindow(true, kHannWindowLength, &hann_window[0]);
  for (size_t i = 0; i < kHannWindowLength; ++i) {
    EXPECT_NEAR(hann_window[0][i], kExpectedHannWindow[i], kEpsilonFloat);
  }
}

// Tests that the calculated number of reverb octave bands matches the
// pre-computed results with arbitrary sampling rates.
TEST(DspUtilsTest, GetNumReverbOctaveBandsTest) {
  const std::vector<int> kSamplingRates = {8000, 22050, 44100, 48000, 96000};
  const std::vector<size_t> kExpectedOutput = {7, 8, 9, 9, 9};

  for (size_t i = 0; i < kSamplingRates.size(); ++i) {
    EXPECT_EQ(kExpectedOutput[i], GetNumReverbOctaveBands(kSamplingRates[i]));
  }
}

// Tests that the calculated number of samples for arbitrary milliseconds values
// matches the pre-computed results with a specific sampling rate.
TEST(DspUtilsTest, GetNumSamplesFromMillisecondsTest) {
  const std::vector<float> kInput = {0.0f, 2.5f, 50.0f, 123.45f, 1000.0f};
  const std::vector<size_t> kExpectedOutput = {0, 120, 2400, 5925, 48000};

  for (size_t i = 0; i < kInput.size(); ++i) {
    EXPECT_EQ(kExpectedOutput[i],
              GetNumSamplesFromMilliseconds(kInput[i], kSamplingRate));
  }
}

}  // namespace

}  // namespace vraudio
