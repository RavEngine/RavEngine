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

#include "dsp/biquad_filter.h"

#include <algorithm>
#include <cmath>
#include <numeric>

#include "third_party/googletest/googletest/include/gtest/gtest.h"
#include "base/constants_and_types.h"

namespace vraudio {

namespace {

const size_t kTestInputDataSize = 16;

const float kTestFilterCoefficients[] = {1.0f, 0.0f, 0.0, 1.0f, 0.1f, 0.1f};

const size_t kIdealSamplesToIterate = 256;

// For use with std::transform in the StabilityTest.
float operator_abs(float f) { return std::abs(f); }

}  // namespace

// Tests that the filtering of a mono signal produces the correct output.
TEST(BiquadFilterTest, FilterTest) {
  const size_t kNumIterations = 1;

  const BiquadCoefficients kCoefficients(
      kTestFilterCoefficients[0], kTestFilterCoefficients[1],
      kTestFilterCoefficients[2], kTestFilterCoefficients[3],
      kTestFilterCoefficients[4], kTestFilterCoefficients[5]);

  AudioBuffer input(kNumMonoChannels, kTestInputDataSize);
  AudioBuffer output(kNumMonoChannels, kTestInputDataSize);
  // Set the input AudioBuffer to a Kronecker delta.
  input.Clear();
  input[0][0] = 1.0f;

  // This vector will accumulate the output from the filter over time (Only
  // works with mono channel).
  std::vector<float> output_accumulator;
  output_accumulator.reserve(kNumIterations * kTestInputDataSize);

  // Create a biquad filter initialized with transfer function coefficients.
  BiquadFilter biquad(kCoefficients, kTestInputDataSize);

  // Perform filtering.
  for (size_t i = 0; i < kNumIterations; ++i) {
    biquad.Filter(input[0], &output[0]);
    output_accumulator.insert(output_accumulator.end(), output[0].begin(),
                              output[0].end());
  }

  // Since the denominator of the biquad coefficients is [1 0 0] we can expect
  // the impulse response to be equal to the numerator.
  for (size_t i = 0; i < output_accumulator.size(); ++i) {
    if (i < 3U) {
      EXPECT_NEAR(kTestFilterCoefficients[3 + i], output_accumulator[i],
                  kEpsilonFloat);
    } else {
      EXPECT_EQ(0.0f, output_accumulator[i]);
    }
  }
}

// Tests that the filtering of a mono signal produces the correct output.
TEST(BiquadFilterTest, InplaceFilterTest) {
  const size_t kNumIterations = 1;

  const BiquadCoefficients kCoefficients(
      kTestFilterCoefficients[0], kTestFilterCoefficients[1],
      kTestFilterCoefficients[2], kTestFilterCoefficients[3],
      kTestFilterCoefficients[4], kTestFilterCoefficients[5]);

  AudioBuffer input(kNumMonoChannels, kTestInputDataSize);
  // Set the input AudioBuffer to a Kronecker delta.
  input.Clear();
  input[0][0] = 1.0f;

  // This vector will accumulate the output from the filter over time (Only
  // works with mono channel).
  std::vector<float> output_accumulator;
  output_accumulator.reserve(kTestInputDataSize);

  // Create a biquad filter initialized with transfer function coefficients.
  BiquadFilter biquad(kCoefficients, kTestInputDataSize);

  // Perform inplace filtering of |input| vector.
  for (size_t i = 0; i < kNumIterations; ++i) {
    biquad.Filter(input[0], &input[0]);
    output_accumulator.insert(output_accumulator.end(), input[0].begin(),
                              input[0].end());
  }

  // Since the denominator of the biquad coefficients is [1 0 0] we can expect
  // the impulse response to be equal to the numerator.
  for (size_t i = 0; i < output_accumulator.size(); ++i) {
    if (i < 3) {
      EXPECT_NEAR(kTestFilterCoefficients[3 + i], output_accumulator[i],
                  kEpsilonFloat);
    } else {
      EXPECT_EQ(0.0f, output_accumulator[i]);
    }
  }
}

// Tests whether the interpolation stops after kIdealSamplesToIterate samples,
// given a long enough buffer.
TEST(BiquadFilterTest, InterpolationStopsTest) {
  const size_t kFramesPerBuffer = 512;

  std::vector<std::vector<float>> planar_input_data(
      kNumMonoChannels, std::vector<float>(kFramesPerBuffer));
  planar_input_data[0][0] = 1.0f;
  planar_input_data[0][256] = 1.0f;

  AudioBuffer input_planar(kNumMonoChannels, kFramesPerBuffer);
  AudioBuffer output_planar(kNumMonoChannels, kFramesPerBuffer);
  input_planar = planar_input_data;

  // Instantiate Biquad for planar data. The impulse response of the default is:
  // 1, 0, 0, ......
  BiquadFilter biquad(BiquadCoefficients(), kFramesPerBuffer);

  // Coefficients we wish to interpolate to. The impulse response of this filter
  // is: 1, 0, 1, ......
  const BiquadCoefficients kNextCoefficients = {1.0f, 0.0f, 0.0f,
                                                1.0f, 0.0f, 1.0f};

  biquad.InterpolateToCoefficients(kNextCoefficients);
  biquad.Filter(input_planar[0], &output_planar[0]);

  // Based on the known impulse responses we can see that if elements 256, 257
  // and 258  have the values 1, 0, 1, then only the new filter coefficients are
  // contributing and we have stopped crossfading. Note: The value of 256 here
  // comes from kIdealSamplesToIterate defined in biquad_filter.cc
  EXPECT_EQ(1.0f, output_planar[0][kIdealSamplesToIterate]);
  EXPECT_EQ(0.0f, output_planar[0][kIdealSamplesToIterate + 1]);
  EXPECT_EQ(1.0f, output_planar[0][kIdealSamplesToIterate + 2]);
}

// Tests whether the |BiquadFilter| remains stable when its z-domain poles lie
// very close to the unit circle.
TEST(BiquadFilterTest, StabilityTest) {
  static const size_t kBufferSize = 1024;
  const size_t kIterations = 200;
  // The following filter was designed in MATLAB with a very slow decay rate.
  // (There are both poles and zeros with magnitude 0.999). Even 200'000 samples
  // after the initial impulse the response will have only died away, in an
  // oscillating manner, to ~1/700 of its peak value.
  static const BiquadCoefficients kHighQCoefficients(
      1.0f, -0.907804951302441f, 0.999869108872718f, 15279.8745150008f, 0.0f,
      -15279.8745150008f);
  BiquadFilter biquad(kHighQCoefficients, kBufferSize);

  AudioBuffer input(kNumMonoChannels, kBufferSize);
  AudioBuffer output(kNumMonoChannels, kBufferSize);
  // Set the input AudioBuffer to a Kronecker delta.
  input.Clear();
  input[0][0] = 1.0f;

  // This vector will accumulate the output from the filter over time (Only
  // works with mono channel).
  std::vector<float> output_accumulator;
  output_accumulator.reserve(kBufferSize * kIterations);

  // Filter once with the Kronecker delta.
  biquad.Filter(input[0], &output[0]);
  output_accumulator.insert(output_accumulator.end(), output[0].begin(),
                            output[0].end());

  // Perform filtering with all zero input.
  input[0][0] = 0.0f;
  for (size_t i = 1; i < kIterations; ++i) {
    biquad.Filter(input[0], &output[0]);
    output_accumulator.insert(output_accumulator.end(), output[0].begin(),
                              output[0].end());
  }

  // Test that the signal is decaying over time (i.e. filter is stable).
  for (size_t i = 0; i < output_accumulator.size() - kBufferSize;
       i += kBufferSize) {
    std::vector<float> absolute_first_half(kBufferSize / 2);
    std::transform(output_accumulator.begin() + i,
                   output_accumulator.begin() + i + kBufferSize / 2,
                   absolute_first_half.begin(), operator_abs);
    std::vector<float> absolute_second_half(kBufferSize / 2);
    std::transform(output_accumulator.begin() + 1 + i + kBufferSize / 2,
                   output_accumulator.begin() + i + kBufferSize,
                   absolute_second_half.begin(), operator_abs);
    const float sum_first_half = std::accumulate(
        absolute_first_half.begin(), absolute_first_half.end(), 0.0f);
    const float sum_second_half = std::accumulate(
        absolute_second_half.begin(), absolute_second_half.end(), 0.0f);
    EXPECT_LT(sum_second_half, sum_first_half);
  }
}

class BiquadFilterInterpolateTest : public ::testing::Test {
 protected:
  BiquadFilterInterpolateTest() {}
  // Virtual methods from ::testing::Test
  ~BiquadFilterInterpolateTest() override {}
  void SetUp() override {}
  void TearDown() override {}

  // Returns true if |filter|'s internal coefficients are equal to those of
  // |expected_coefficients| after scaling by a0.
  bool TestInternalCoefficients(
      BiquadFilter* filter, const BiquadCoefficients& expected_coefficients) {
    bool return_value = true;
    return_value &=
        expected_coefficients.a[0] - filter->coefficients_.a[0] < kEpsilonFloat;

    // From this point we must take account of the scaling by 1/a0.
    const float a0 = filter->coefficients_.a[0];
    return_value &=
        expected_coefficients.b[0] - filter->coefficients_.b[0] * a0 <
        kEpsilonFloat;
    for (int i = 1; i < 3; ++i) {
      return_value &=
          expected_coefficients.b[i] - filter->coefficients_.b[i] * a0 <
          kEpsilonFloat;
      return_value &=
          expected_coefficients.a[i] - filter->coefficients_.a[i] * a0 <
          kEpsilonFloat;
    }
    return return_value;
  }
};

// Tests whether updating the filter's coefficients with the
// InterpolateToCoefficients reaches the correct coefficients after filtering a
// block of data and whether the samples_to_iterate_over_ value is set
// correctly.
TEST_F(BiquadFilterInterpolateTest, InterpolatedUpdateTest) {
  const size_t kFramesPerBuffer = 8;

  const std::vector<std::vector<float>> kPlanarInputData = {
      {1.0f, 2.0f, 3.0f, 4.0f, 5.0f, 6.0f, 7.0f, 8.0f}};

  const std::vector<float> kExpectedOutputAfterChange = {
      8.0f, 10.0f, 4.0f, 6.0f, 8.0f, 10.0f, 12.0f, 14.0f};

  // Default constructor sets a = {1.0, 0.0, 0.0} and b = {1.0, 0.0, 0.0}.
  const BiquadCoefficients kCoefficients;

  // Create input and output AudioBuffers.
  AudioBuffer input_planar(kNumMonoChannels, kFramesPerBuffer);
  AudioBuffer output_planar(kNumMonoChannels, kFramesPerBuffer);
  input_planar = kPlanarInputData;

  // Instantiate Biquad for planar data.
  BiquadFilter biquad(kCoefficients, kFramesPerBuffer);

  // Perform filtering on kNumMonoChannels interleaved.
  biquad.Filter(input_planar[0], &output_planar[0]);

  for (size_t i = 0; i < kFramesPerBuffer; ++i) {
    EXPECT_NEAR(output_planar[0][i], kPlanarInputData[0][i], kEpsilonFloat);
  }

  // Coefficients we wish to interpolate to.
  static const BiquadCoefficients kNextCoefficients = {1.0f, 0.0f, 0.0f,
                                                       1.0f, 0.0f, 1.0f};

  biquad.InterpolateToCoefficients(kNextCoefficients);

  // Filter once to transition and once to flush out the state.
  biquad.Filter(input_planar[0], &output_planar[0]);
  biquad.Filter(input_planar[0], &output_planar[0]);

  // Now the output should be just from the new state.
  biquad.Filter(input_planar[0], &output_planar[0]);

  // Now check that we transitioned properly, i.e. the output is as expected.
  for (size_t i = 0; i < kFramesPerBuffer; ++i) {
    EXPECT_NEAR(output_planar[0][i], kExpectedOutputAfterChange[i],
                kEpsilonFloat);
  }

  // Now check that we have the new coefficients.
  EXPECT_TRUE(TestInternalCoefficients(&biquad, kNextCoefficients));
}

}  // namespace vraudio
