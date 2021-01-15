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

#include "ambisonics/associated_legendre_polynomials_generator.h"

#include <cmath>

#include "third_party/googletest/googletest/include/gtest/gtest.h"
#include "base/constants_and_types.h"
#include "base/misc_math.h"

namespace vraudio {

namespace {

// Tolerated test error margin for single-precision floating points.
const float kEpsilon = 1e-5f;

// Generates associated Legendre polynomials up to and including the 4th degree,
// with the Condon-Shortley phase and negative orders included.
std::vector<float> GenerateExpectedValuesFourthDegree(float x) {
  // From http://en.wikipedia.org/wiki/Associated_Legendre_polynomials
  // Comments are (degree, order).
  const std::vector<float> expected_values = {
      1.0f,                                                  // (0, 0)
      0.5f * std::sqrt(1.0f - x * x),                        // (1, -1)
      x,                                                     // (1, 0)
      -std::sqrt(1.0f - x * x),                              // (1, 1)
      1.0f / 8.0f * (1.0f - x * x),                          // (2, -2)
      0.5f * x * std::sqrt(1.0f - x * x),                    // (2, -1)
      0.5f * (3.0f * x * x - 1.0f),                          // (2, 0)
      -3.0f * x * std::sqrt(1.0f - x * x),                   // (2, 1)
      3.0f * (1.0f - x * x),                                 // (2, 2)
      15.0f / 720.0f * std::pow(1.0f - x * x, 3.0f / 2.0f),  // (3, -3)
      15.0f / 120.0f * x * (1.0f - x * x),                   // (3, -2)
      3.0f / 24.0f * (5.0f * x * x - 1.0f) *
          std::sqrt(1.0f - x * x),                  // (3, -1)
      0.5f * (5.0f * IntegerPow(x, 3) - 3.0f * x),  // (3, 0)
      -3.0f / 2.0f * (5.0f * x * x - 1.0f) * std::sqrt(1.0f - x * x),  // (3, 1)
      15.0f * x * (1.0f - x * x),                                      // (3, 2)
      -15.0f * std::pow(1.0f - x * x, 3.0f / 2.0f),                    // (3, 3)
      105.0f / 40320.0f * IntegerPow(1.0f - x * x, 2),             // (4, -4)
      105.0f / 5040.0f * x * std::pow(1.0f - x * x, 3.0f / 2.0f),  // (4, -3)
      15.0f / 720.0f * (7.0f * x * x - 1.0f) * (1.0f - x * x),     // (4, -2)
      5.0f / 40.0f * (7.0f * IntegerPow(x, 3) - 3.0f * x) *
          std::sqrt(1.0f - x * x),  // (4, -1)
      1.0f / 8.0f *
          (35.0f * IntegerPow(x, 4) - 30.0f * x * x + 3.0f),  // (4, 0)
      -5.0f / 2.0f * (7.0f * IntegerPow(x, 3) - 3.0f * x) *
          std::sqrt(1.0f - x * x),                            // (4, 1)
      15.0f / 2.0f * (7.0f * x * x - 1.0f) * (1.0f - x * x),  // (4, 2)
      -105.0f * x * std::pow(1.0f - x * x, 3.0f / 2.0f),      // (4, 3)
      105.0f * IntegerPow(1.0f - x * x, 2)                    // (4, 4)
  };

  return expected_values;
}

// Tests that the values given by GetIndex are successive indices (n, n+1, n+2,
// and so on).
TEST(AssociatedLegendrePolynomialsGeneratorTest, GetIndex_SuccessiveIndices) {
  const int kMaxDegree = 5;
  const AssociatedLegendrePolynomialsGenerator alp_generator(kMaxDegree, false,
                                                             true);
  int last_index = -1;
  for (int degree = 0; degree <= kMaxDegree; ++degree) {
    for (int order = -degree; order <= degree; ++order) {
      int index = static_cast<int>(alp_generator.GetIndex(degree, order));
      EXPECT_EQ(last_index + 1, index);
      last_index = index;
    }
  }
}

// Tests that the zeroth-degree, zeroth-order ALP is always 1.
TEST(AssociatedLegendrePolynomialsGeneratorTest, Generate_ZerothElementIsOne) {
  const int kMaxDegree = 10;
  for (int max_degree = 0; max_degree <= kMaxDegree; ++max_degree) {
    for (int condon_shortley_phase = 0; condon_shortley_phase <= 1;
         ++condon_shortley_phase) {
      for (int compute_negative_order = 0; compute_negative_order <= 1;
           ++compute_negative_order) {
        AssociatedLegendrePolynomialsGenerator alp_generator(
            max_degree, condon_shortley_phase != 0,
            compute_negative_order != 0);

        const float kVariableStep = 0.2f;
        for (float x = -1.0f; x <= 1.0f; x += kVariableStep) {
          const std::vector<float> values = alp_generator.Generate(x);

          EXPECT_NEAR(values[0], 1.0f, kEpsilon);
        }
      }
    }
  }
}

// Tests that the polynomials generated are correct until the 4th degree.
TEST(AssociatedLegendrePolynomialsGeneratorTest, Generate_CorrectFourthDegree) {
  const int kMaxDegree = 4;
  const bool kCondonShortleyPhase = true;
  const bool kComputeNegativeOrder = true;
  const AssociatedLegendrePolynomialsGenerator alp_generator(
      kMaxDegree, kCondonShortleyPhase, kComputeNegativeOrder);

  const float kVariableStep = 0.05f;
  for (float x = -1.0f; x <= 1.0f; x += kVariableStep) {
    const std::vector<float> generated_values = alp_generator.Generate(x);
    const std::vector<float> expected_values =
        GenerateExpectedValuesFourthDegree(x);
    ASSERT_EQ(expected_values.size(), generated_values.size());
    for (size_t i = 0; i < expected_values.size(); ++i) {
      EXPECT_NEAR(generated_values[i], expected_values[i], kEpsilon)
          << " at index " << i;
    }
  }
}

// Tests that the Condon-Shortley phase is correctly applied.
TEST(AssociatedLegendrePolynomialsGeneratorTest, Generate_CondonShortleyPhase) {
  const int kMaxDegree = 10;
  const float kValue = 0.12345f;
  for (int max_degree = 0; max_degree <= kMaxDegree; ++max_degree) {
    for (int compute_negative_order = 0; compute_negative_order <= 1;
         ++compute_negative_order) {
      const AssociatedLegendrePolynomialsGenerator alp_generator_without_phase(
          max_degree, false, compute_negative_order != 0);
      const std::vector<float> values_without_phase =
          alp_generator_without_phase.Generate(kValue);

      const AssociatedLegendrePolynomialsGenerator alp_generator_with_phase(
          max_degree, true, compute_negative_order != 0);
      const std::vector<float> values_with_phase =
          alp_generator_with_phase.Generate(kValue);

      ASSERT_EQ(values_with_phase.size(), values_without_phase.size());
      for (int degree = 0; degree <= max_degree; ++degree) {
        const int start_order = compute_negative_order ? -degree : 0;
        for (int order = start_order; order <= degree; ++order) {
          const size_t index =
              alp_generator_without_phase.GetIndex(degree, order);
          const float expected = values_without_phase[index] *
                                 std::pow(-1.0f, static_cast<float>(order));
          EXPECT_NEAR(values_with_phase[index], expected, kEpsilon)
              << " at degree " << degree << " and order " << order;
        }
      }
    }
  }
}

}  // namespace

}  // namespace vraudio
