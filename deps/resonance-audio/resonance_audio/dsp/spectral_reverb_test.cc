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

#include "dsp/spectral_reverb.h"

#include <algorithm>
#include <cmath>
#include <numeric>
#include <vector>

#include "third_party/googletest/googletest/include/gtest/gtest.h"
#include "base/constants_and_types.h"
#include "dsp/biquad_filter.h"
#include "dsp/fft_manager.h"
#include "dsp/filter_coefficient_generators.h"
#include "utils/test_util.h"

namespace vraudio {

namespace {

const size_t kFramesPerBuffer512 = 512;
const size_t kFramesPerBuffer2048 = 2048;
const size_t kFramesPerBuffer713 = 713;
const size_t kNumReverbOverlap = 4;
const size_t kReverbFftSize = 4096;
const int kSampleFrequency16 = 16000;
const int kSampleFrequency24 = 24000;

// A set of 9 RT60 values for each of the octave bands with center frequencies
// between 31.25Hz and 8kHz.
const float kRt60s[kNumReverbOctaveBands] = {0.8f,  0.8f, 0.7f, 0.7f, 0.65f,
                                             0.65f, 0.6f, 0.6f, 0.5f};

void ImpulseResponse(float length_sec, int sample_frequency,
                     size_t frames_per_buffer, SpectralReverb* reverb,
                     std::vector<float>* left_response,
                     std::vector<float>* right_response) {
  AudioBuffer input(kNumMonoChannels, frames_per_buffer);
  input.Clear();
  input[0][0] = 1.0f;
  AudioBuffer output(kNumStereoChannels, frames_per_buffer);

  reverb->Process(input[0], &output[0], &output[1]);

  // The number of iterations required so that we will have no more than a few
  // zeros following each of our impulse responses.
  const float tail_length_samples =
      kReverbFftSize + length_sec * static_cast<float>(sample_frequency);
  const size_t num_iterations =
      100 + 1 +
      static_cast<size_t>(std::ceil(tail_length_samples /
                                    static_cast<float>(frames_per_buffer)));

  for (size_t i = 0; i < num_iterations; ++i) {
    left_response->insert(left_response->end(), output[0].begin(),
                          output[0].end());
    right_response->insert(right_response->end(), output[1].begin(),
                           output[1].end());

    input.Clear();
    reverb->Process(input[0], &output[0], &output[1]);
  }
}

void StereoOutputTestHelper(int sample_frequency, size_t frames_per_buffer) {
  SpectralReverb reverb(sample_frequency, frames_per_buffer);
  reverb.SetRt60PerOctaveBand(kRt60s);

  std::vector<float> output_collect_left;
  std::vector<float> output_collect_right;
  ImpulseResponse(1.0f /*length [sec]*/, sample_frequency, frames_per_buffer,
                  &reverb, &output_collect_left, &output_collect_right);

  // First test that there are kReverbFftSize / kNumReverbOverlap zeros at the
  // beginning of the system impulse responses ONLY if this value is greater
  // than |frames_per_buffer|, otherwise expect no onset zeros.
  const size_t kOverlapLength = kReverbFftSize / kNumReverbOverlap;
  const size_t onset = kOverlapLength > frames_per_buffer ? kOverlapLength : 0;
  for (size_t i = 0; i < onset; ++i) {
    EXPECT_NEAR(0.0f, output_collect_left[i], kEpsilonFloat);
    EXPECT_NEAR(0.0f, output_collect_right[i], kEpsilonFloat);
  }
  // Test the sample five samples later is non zero. i.e. the tail has begun.
  EXPECT_NE(0.0f, output_collect_left[onset + 5]);
  EXPECT_NE(0.0f, output_collect_right[onset + 5]);
}

void ZeroRt60TestHelper(int sample_frequency, float rt_60) {
  SpectralReverb reverb(sample_frequency, kFramesPerBuffer512);
  reverb.SetRt60PerOctaveBand(kRt60s);

  std::vector<float> output_collect_left;
  std::vector<float> output_collect_right;
  ImpulseResponse(1.0f /*length [sec]*/, sample_frequency, kFramesPerBuffer512,
                  &reverb, &output_collect_left, &output_collect_right);

  // Test that all the frames of the tail buffer are zeros.
  for (size_t i = 0; i < kFramesPerBuffer512; ++i) {
    EXPECT_NEAR(0.0f, output_collect_left[i], kEpsilonFloat);
    EXPECT_NEAR(0.0f, output_collect_right[i], kEpsilonFloat);
  }
}

void TailDecayTestHelper(int sample_frequency, size_t frames_per_buffer) {
  // Butterworth Lowpass filter with cutoff frequency at 3Hz.
  BiquadCoefficients low_pass_coefficients(
      1.0f, -1.999444639647f, 0.999444793816755f, 1e-5f, 2e-5f, 1e-5f);
  BiquadFilter low_pass_filter(low_pass_coefficients, frames_per_buffer);
  const std::vector<float> kUniformRt60s(kNumReverbOctaveBands, kRt60s[0]);
  SpectralReverb reverb(sample_frequency, frames_per_buffer);
  reverb.SetRt60PerOctaveBand(kUniformRt60s.data());

  std::vector<float> output_collect_left;
  std::vector<float> output_collect_right;
  ImpulseResponse(2.0f /*length [sec]*/, sample_frequency, frames_per_buffer,
                  &reverb, &output_collect_left, &output_collect_right);

  for (size_t i = 0; i < output_collect_left.size(); ++i) {
    output_collect_left[i] = std::abs(output_collect_left[i]);
    output_collect_right[i] = std::abs(output_collect_right[i]);
  }

  const size_t response_length = output_collect_left.size();

  AudioBuffer test_buffer(kNumMonoChannels, response_length);
  test_buffer[0] = output_collect_left;

  // Very low frequency content of tail. This should essentially just preserve
  // the decay.
  low_pass_filter.Filter(test_buffer[0], &test_buffer[0]);

  const size_t max_location = static_cast<size_t>(
      std::max_element(test_buffer[0].begin(), test_buffer[0].end()) -
      test_buffer[0].begin());

  // Stop before the very end of the tail as it goes to zero.
  const size_t end_point = max_location + kReverbFftSize;
  const size_t step_size = (end_point - max_location) / 20;

  // Test for decay.
  for (size_t i = max_location + step_size; i < end_point; i += step_size) {
    EXPECT_GT(std::abs(test_buffer[0][i - step_size]),
              std::abs(test_buffer[0][i]));
  }
}

void DecorrelatedTailsTestHelper(int sample_frequency,
                                 size_t frames_per_buffer) {
  // This value has been found empirically in MATLAB.
  const float kMaxCrossCorrelation = 12.0f;
  const std::vector<float> kUniformRt60s(kNumReverbOctaveBands, 0.7f);
  SpectralReverb reverb(sample_frequency, frames_per_buffer);
  reverb.SetRt60PerOctaveBand(kUniformRt60s.data());

  std::vector<float> output_collect_left;
  std::vector<float> output_collect_right;
  ImpulseResponse(0.7f /*length [sec]*/, sample_frequency, frames_per_buffer,
                  &reverb, &output_collect_left, &output_collect_right);

  // Find the absolute maximum elements of each vector.
  auto min_max_left = std::minmax_element(output_collect_left.begin(),
                                          output_collect_left.end());
  size_t left_max_index =
      std::abs(*min_max_left.first) > std::abs(*min_max_left.second)
          ? (min_max_left.first - output_collect_left.begin())
          : (min_max_left.second - output_collect_left.begin());
  auto min_max_right = std::minmax_element(output_collect_right.begin(),
                                           output_collect_right.end());
  size_t right_max_index =
      std::abs(*min_max_right.first) > std::abs(*min_max_right.second)
          ? (min_max_right.first - output_collect_right.begin())
          : (min_max_right.second - output_collect_right.begin());

  // Take a sample of the tails for cross correlation.
  AudioBuffer pair(kNumStereoChannels, kReverbFftSize);
  for (size_t i = 0; i < kReverbFftSize; ++i) {
    pair[0][i] = output_collect_left[i + left_max_index] /
                 output_collect_left[left_max_index];
    pair[1][i] = output_collect_right[i + right_max_index] /
                 output_collect_right[right_max_index];
  }

  // The cross correlation is not normalized. Thus we can expect a very small
  // value. Naturally, if the RT60 inputs are changed the expected value would
  // thus be different.
  const float max_xcorr = MaxCrossCorrelation(pair[0], pair[1]);
  EXPECT_LT(max_xcorr, kMaxCrossCorrelation);
}

}  // namespace

// Tests that the stereo output from the Reverbs Process fuction has the
// expected properties of predelay and length.
TEST(SpectralReverbTest, StereoOutputTest) {
  StereoOutputTestHelper(kSampleFrequency24, kFramesPerBuffer512);
  StereoOutputTestHelper(kSampleFrequency24, kFramesPerBuffer2048);
  StereoOutputTestHelper(kSampleFrequency24, kFramesPerBuffer713);
  StereoOutputTestHelper(kSampleFrequency16, kFramesPerBuffer512);
  StereoOutputTestHelper(kSampleFrequency16, kFramesPerBuffer2048);
  StereoOutputTestHelper(kSampleFrequency16, kFramesPerBuffer713);
}

// Tests that the stereo output from the Reverb's Process function has the
// output of all zeros when the RT60 values are all zero.
TEST(SpectralReverbTest, ZeroRt60Test) {
  const float kZeroRt = 0.0f;
  const float kBelowMinRt = 0.12f;
  ZeroRt60TestHelper(kSampleFrequency24, kZeroRt);
  ZeroRt60TestHelper(kSampleFrequency16, kBelowMinRt);
  ZeroRt60TestHelper(kSampleFrequency16, kZeroRt);
  ZeroRt60TestHelper(kSampleFrequency16, kBelowMinRt);
}

// Tests that the tail is decaying over time.
TEST(SpectralReverbTest, TailDecayTest) {
  TailDecayTestHelper(kSampleFrequency24, kFramesPerBuffer512);
  TailDecayTestHelper(kSampleFrequency24, kFramesPerBuffer2048);
  TailDecayTestHelper(kSampleFrequency16, kFramesPerBuffer512);
  TailDecayTestHelper(kSampleFrequency16, kFramesPerBuffer2048);
}

// Tests that the stereo tail pairs are highy decorrelated.
TEST(SpectralReverbTest, DecorrelatedTailsTest) {
  DecorrelatedTailsTestHelper(kSampleFrequency24, kFramesPerBuffer512);
  DecorrelatedTailsTestHelper(kSampleFrequency24, kFramesPerBuffer2048);
  DecorrelatedTailsTestHelper(kSampleFrequency24, kFramesPerBuffer713);
  DecorrelatedTailsTestHelper(kSampleFrequency16, kFramesPerBuffer512);
  DecorrelatedTailsTestHelper(kSampleFrequency16, kFramesPerBuffer2048);
  DecorrelatedTailsTestHelper(kSampleFrequency16, kFramesPerBuffer713);
}

// Tests that the gain parameter behaves as expected.
TEST(SpecralReverbTest, GainTest) {
  const float kReverbLength = 0.5f;
  const float kGain = 100.0f;
  const float kGainEpsilon = 0.32f;
  const std::vector<float> kUniformRt60s(kNumReverbOctaveBands, kReverbLength);
  SpectralReverb reverb(kSampleFrequency24, kFramesPerBuffer512);
  reverb.SetRt60PerOctaveBand(kUniformRt60s.data());

  // Calculate scaled and unscaled impulse responses.
  std::vector<float> output_left;
  std::vector<float> output_right;
  ImpulseResponse(kReverbLength, kSampleFrequency24, kFramesPerBuffer512,
                  &reverb, &output_left, &output_right);
  std::vector<float> output_left_scaled;
  reverb.SetGain(kGain);
  ImpulseResponse(kReverbLength, kSampleFrequency24, kFramesPerBuffer512,
                  &reverb, &output_left_scaled, &output_right);

  // Determine the max absolute entry in each impulse response.
  std::transform(output_left.begin(), output_left.end(), output_left.begin(),
                 static_cast<float (*)(float)>(&std::abs));
  std::transform(output_left_scaled.begin(), output_left_scaled.end(),
                 output_left_scaled.begin(),
                 static_cast<float (*)(float)>(&std::abs));
  const float max_unscaled =
      *std::max_element(output_left.begin(), output_left.end());
  const float max_scaled =
      *std::max_element(output_left_scaled.begin(), output_left_scaled.end());
  EXPECT_GT(max_scaled, max_unscaled);
  EXPECT_NEAR((max_unscaled / max_scaled) / (1.0f / kGain), 1.0f, kGainEpsilon);
}

// Tests that when the feedback values are all ~0.0f, no processing is
// performed (output is all zero). Also tests that if even one of the rt60s
// result in a non zero feedback that the result will be non zero.
TEST(SpectralReverbTest, DisabledProcessingTest) {
  const float kReverbLength = 0.1f;
  const std::vector<float> kUniformRt60s(kNumReverbOctaveBands, kReverbLength);
  SpectralReverb reverb(kSampleFrequency24, kFramesPerBuffer512);
  reverb.SetRt60PerOctaveBand(kUniformRt60s.data());
  std::vector<float> output_left;
  std::vector<float> output_right;
  ImpulseResponse(kReverbLength, kSampleFrequency24, kFramesPerBuffer512,
                  &reverb, &output_left, &output_right);
  for (size_t i = 0; i < output_left.size(); ++i) {
    EXPECT_FLOAT_EQ(output_left[i], 0.0f);
    EXPECT_FLOAT_EQ(output_right[i], 0.0f);
  }

  // Test a non zero case.
  const float kLongerReverbLength = 0.4f;
  std::vector<float> rt60s(kNumReverbOctaveBands, 0.0f);
  rt60s[0] = kLongerReverbLength;
  output_left.resize(0);
  output_right.resize(0);
  reverb.SetRt60PerOctaveBand(rt60s.data());
  ImpulseResponse(kReverbLength, kSampleFrequency24, kFramesPerBuffer512,
                  &reverb, &output_left, &output_right);
  const float sum_left =
      std::accumulate(output_left.begin(), output_left.end(), 0.0f);
  EXPECT_NE(sum_left, 0.0f);
  const float sum_right =
      std::accumulate(output_right.begin(), output_right.end(), 0.0f);
  EXPECT_NE(sum_right, 0.0f);

  // Set gain to zero and test again.
  output_left.resize(0);
  output_right.resize(0);
  reverb.SetGain(0.0f);
  ImpulseResponse(kReverbLength, kSampleFrequency24, kFramesPerBuffer512,
                  &reverb, &output_left, &output_right);
  for (size_t i = 0; i < output_left.size(); ++i) {
    EXPECT_FLOAT_EQ(output_left[i], 0.0f);
    EXPECT_FLOAT_EQ(output_right[i], 0.0f);
  }
}

}  // namespace vraudio
