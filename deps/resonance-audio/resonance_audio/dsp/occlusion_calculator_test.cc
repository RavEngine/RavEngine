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

#include "dsp/occlusion_calculator.h"

#include <algorithm>

#include "third_party/googletest/googletest/include/gtest/gtest.h"
#include "base/constants_and_types.h"
#include "base/misc_math.h"

namespace vraudio {

namespace {

// Spherical coordinates of listener relative to source.
const float kListenerAheadRads[2] = {0.0f, 0.0f};
const float kListenerBesideRads[2] = {0.0f, static_cast<float>(M_PI_2)};
const float kListenerAboveAndAheadRads[2] = {static_cast<float>(M_PI_4), 0.0f};

// Test case data structure.
struct DirectivityTestParams {
  // Directivity function's alpha weighting.
  const float alpha;
  // Directivity function's order.
  const float order;
  // Spherical angle of the listener relative to the source.
  const float* relative_spherical_angle;
  // Expected value produced by the directivity function for the given input
  // parameters.
  const float expected_directivity;
};

// This test runs through a series of parametrized test cases and checks that
// the directivity value calculated according to those parameters is correct.
class DirectivityCalculatorParametrizedTest
    : public testing::Test,
      public testing::WithParamInterface<DirectivityTestParams> {};

// Parametrized test which verifies that the directivity value calculated from
// the test parameters matches the expected value supplied.
TEST_P(DirectivityCalculatorParametrizedTest, CalculateDirectivityTest) {
  // Test parameters.
  const DirectivityTestParams test_params = GetParam();

  // Construct test angle.
  const float elevation_rad = test_params.relative_spherical_angle[0];
  const float azimuth_rad = test_params.relative_spherical_angle[1];
  SphericalAngle test_angle(elevation_rad, azimuth_rad);

  // Calculate directivity.
  const float directivity =
      CalculateDirectivity(test_params.alpha, test_params.order, test_angle);

  // Check calculated directivity.
  EXPECT_NEAR(test_params.expected_directivity, directivity, kEpsilonFloat);
}

// Test parameters, according to struct |DirectivityParams|.
DirectivityTestParams test_cases[] = {
    // Omnidirectional.
    {0.0f, 1.0f, &(kListenerAheadRads[0]), 1.0f},
    {0.0f, 1.0f, &(kListenerBesideRads[0]), 1.0f},
    {0.0f, 1.0f, &(kListenerAboveAndAheadRads[0]), 1.0f},
    {0.0f, 2.0f, &(kListenerAboveAndAheadRads[0]), 1.0f},
    {0.0f, 0.5f, &(kListenerAboveAndAheadRads[0]), 1.0f},
    // Hypocardioid.
    {0.25f, 1.0f, &(kListenerAheadRads[0]), 1.0f},
    {0.25f, 1.0f, &(kListenerBesideRads[0]), 0.75f},
    {0.25f, 1.0f, &(kListenerAboveAndAheadRads[0]), 0.926777f},
    {0.25f, 2.0f, &(kListenerAboveAndAheadRads[0]), 0.858915f},
    {0.25f, 0.5f, &(kListenerAboveAndAheadRads[0]), 0.926777f},
    // Cardioid.
    {0.5f, 1.0f, &(kListenerAheadRads[0]), 1.0f},
    {0.5f, 1.0f, &(kListenerBesideRads[0]), 0.5f},
    {0.5f, 1.0f, &(kListenerAboveAndAheadRads[0]), 0.853553f},
    {0.5f, 2.0f, &(kListenerAboveAndAheadRads[0]), 0.728553f},
    // Hypercardioid.
    {0.75f, 1.0f, &(kListenerAheadRads[0]), 1.0f},
    {0.75f, 1.0f, &(kListenerBesideRads[0]), 0.25f},
    {0.75f, 1.0f, &(kListenerAboveAndAheadRads[0]), 0.780330f},
    {0.75f, 2.0f, &(kListenerAboveAndAheadRads[0]), 0.608915f},
    {0.75f, 0.5f, &(kListenerAboveAndAheadRads[0]), 0.780330f},
    // Dipole.
    {1.0f, 1.0f, &(kListenerAheadRads[0]), 1.0f},
    {1.0f, 1.0f, &(kListenerBesideRads[0]), 0.0f},
    {1.0f, 1.0f, &(kListenerAboveAndAheadRads[0]), 0.707107f},
    {1.0f, 2.0f, &(kListenerAboveAndAheadRads[0]), 0.5f},
    {1.0f, 0.5f, &(kListenerAboveAndAheadRads[0]), 0.707107f}};

INSTANTIATE_TEST_CASE_P(Instance, DirectivityCalculatorParametrizedTest,
                        testing::ValuesIn(test_cases));

TEST(DirectivityTest, CalculateOcclusionFilterCoefficientTest) {
  // When there is no occlusion (occlusion == 0) expect the filter coefficient
  // to be 1 - directivity.
  const std::vector<float> directivities = {0.0f, 0.3f, 0.5f, 0.7f, 0.9f, 1.5f};
  for (size_t i = 0; i < directivities.size() - 1; ++i) {
    const float coefficient =
        CalculateOcclusionFilterCoefficient(directivities[i], 0.0f);
    EXPECT_EQ(1.0f - directivities[i], coefficient);
  }
  // Ensure the minimum value returned is 0.
  const float coefficient =
      CalculateOcclusionFilterCoefficient(directivities.back(), 0.0f);
  EXPECT_EQ(0.0f, coefficient);
}

TEST(OcclusionTest, CalculateOcclusionFilterCoefficientTest) {
  // When there is no effect on directivity, expect the filter coefficient
  // to be  1 / (x + 1)^4 where x is the occlusion intensity.
  const std::vector<float> occlusions = {0.01f, 0.1f, 1.0f, 10.0f, 100.0f};
  std::vector<float> expected_coefficients = occlusions;
  // Calculate 1 / (x + 1)^4 where x is the occlusion intensity.
  std::for_each(expected_coefficients.begin(), expected_coefficients.end(),
                [](float& n) { n = 1.0f - 1.0f / IntegerPow(n + 1.0f, 4); });
  for (size_t i = 0; i < expected_coefficients.size(); ++i) {
    const float coefficient =
        CalculateOcclusionFilterCoefficient(1.0f, occlusions[i]);
    EXPECT_EQ(expected_coefficients[i], coefficient);
  }
}

}  // namespace

}  // namespace vraudio
