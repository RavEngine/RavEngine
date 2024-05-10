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

#include "ambisonics/ambisonic_lookup_table.h"

#include <cmath>

#include "third_party/googletest/googletest/include/gtest/gtest.h"
#include "ambisonics/utils.h"
#include "base/constants_and_types.h"
#include "base/misc_math.h"

namespace vraudio {

namespace {

const int kSourceAmbisonicOrder0 = 0;
const int kSourceAmbisonicOrder1 = 1;
const int kSourceAmbisonicOrder2 = 2;
const int kSourceAmbisonicOrder3 = 3;

// Minimum angular source spread of 0 ensures that no gain correction
// coefficients should be applied to the Ambisonic encoding coefficients.
const float kMinSpreadDeg = 0.0f;

}  // namespace

class AmbisonicLookupTableTest : public ::testing::TestWithParam<int> {
 protected:
  AmbisonicLookupTableTest()
      : lookup_table_(kMaxSupportedAmbisonicOrder),
        source_ambisonic_order_(GetParam()),
        encoding_coeffs_(GetNumPeriphonicComponents(source_ambisonic_order_)) {}

  // Tests whether GetEncodingCoeffs() method returns correct coefficients.
  void TestEncodingCoefficients() {
    for (size_t i = 0; i < kSourceDirections.size(); ++i) {
      const std::vector<float> kExpectedCoeffs =
          GenerateExpectedCoeffs(kSourceDirections[i]);
      lookup_table_.GetEncodingCoeffs(source_ambisonic_order_,
                                      kSourceDirections[i], kMinSpreadDeg,
                                      &encoding_coeffs_);
      for (size_t j = 0; j < encoding_coeffs_.size(); ++j) {
        EXPECT_NEAR(kExpectedCoeffs[j], encoding_coeffs_[j], kEpsilonFloat);
      }
    }
  }

  // Tests whether changing the source spread results in correct gain changes in
  // pressure and velocity ambisonic encoding coefficients.
  void TestSpreadEnergy() {
    // Choose an artbitrary source direction from |kSourceDirections|.
    const SphericalAngle source_direction = kSourceDirections.back();
    lookup_table_.GetEncodingCoeffs(source_ambisonic_order_, source_direction,
                                    kMinSpreadDeg, &encoding_coeffs_);
    float current_pressure_energy = GetPressureEnergy();
    float current_velocity_energy = GetVelocityEnergy();
    for (int spread_deg = 1; spread_deg <= 360; ++spread_deg) {
      lookup_table_.GetEncodingCoeffs(source_ambisonic_order_, source_direction,
                                      static_cast<float>(spread_deg),
                                      &encoding_coeffs_);
      float new_pressure_energy = GetPressureEnergy();
      float new_velocity_energy = GetVelocityEnergy();
      EXPECT_TRUE(new_pressure_energy >= current_pressure_energy);
      EXPECT_TRUE(new_velocity_energy <= current_velocity_energy);
      current_pressure_energy = new_pressure_energy;
      current_velocity_energy = new_velocity_energy;
    }
  }

 private:
  // Generates expected ambisonic encoding coefficients for ambisonic orders 0
  // to 3, according to http://ambisonics.ch/standards/channels/index.
  std::vector<float> GenerateExpectedCoeffs(const SphericalAngle& angle) {
    const float a = angle.azimuth();
    const float e = angle.elevation();
    return std::vector<float>{
        1.0f,
        std::cos(e) * std::sin(a),
        std::sin(e),
        std::cos(e) * std::cos(a),
        kSqrtThree / 2.0f * std::cos(e) * std::cos(e) * std::sin(2.0f * a),
        kSqrtThree / 2.0f * std::sin(2.0f * e) * std::sin(a),
        0.5f * (3.0f * std::sin(e) * std::sin(e) - 1.0f),
        kSqrtThree / 2.0f * std::sin(2.0f * e) * std::cos(a),
        kSqrtThree / 2.0f * std::cos(e) * std::cos(e) * std::cos(2.0f * a),
        std::sqrt(5.0f / 8.0f) * IntegerPow(std::cos(e), 3) *
            std::sin(3.0f * a),
        std::sqrt(15.0f) / 2.0f * std::sin(e) * std::cos(e) * std::cos(e) *
            std::sin(2.0f * a),
        std::sqrt(3.0f / 8.0f) * std::cos(e) *
            (5.0f * std::sin(e) * std::sin(e) - 1.0f) * std::sin(a),
        0.5f * std::sin(e) * (5.0f * std::sin(e) * std::sin(e) - 3.0f),
        std::sqrt(3.0f / 8.0f) * std::cos(e) *
            (5.0f * std::sin(e) * std::sin(e) - 1.0f) * std::cos(a),
        std::sqrt(15.0f) / 2.0f * std::sin(e) * std::cos(e) * std::cos(e) *
            std::cos(2.0f * a),
        std::sqrt(5.0f / 8.0f) * IntegerPow(std::cos(e), 3) *
            std::cos(3.0f * a)};
  }

  // Computes energy of the pressure channel (Ambisonic channel 0).
  float GetPressureEnergy() {
    return encoding_coeffs_[0] * encoding_coeffs_[0];
  }

  // Computes total energy of all the velocity channels (Ambisonic channel 1 and
  // above).
  float GetVelocityEnergy() {
    float velocity_energy = 0.0f;
    for (size_t i = 1; i < encoding_coeffs_.size(); ++i) {
      velocity_energy += encoding_coeffs_[i] * encoding_coeffs_[i];
    }
    return velocity_energy;
  }

  // Source angles corresponding to each Cartesian axis direction as well as
  // some arbitrary directions (full degrees) in each quadrant of the sphere.
  const std::vector<SphericalAngle> kSourceDirections = {
      {SphericalAngle::FromDegrees(0.0f, 0.0f)} /* Front */,
      {SphericalAngle::FromDegrees(90.0f, 0.0f)} /* Left */,
      {SphericalAngle::FromDegrees(180.0f, 0.0f)} /* Back */,
      {SphericalAngle::FromDegrees(-90.0f, 0.0f)} /* Right */,
      {SphericalAngle::FromDegrees(0.0f, 90.0f)} /* Up */,
      {SphericalAngle::FromDegrees(0.0f, -90.0f)} /* Down */,
      {SphericalAngle::FromDegrees(10.0f, 20.0f)} /* Left-Top-Front */,
      {SphericalAngle::FromDegrees(-30.0f, 40.0f)} /* Right-Top-Front  */,
      {SphericalAngle::FromDegrees(50.0f, -60.0f)} /* Left-Bottom-Front */,
      {SphericalAngle::FromDegrees(290.0f, -80.0f)} /* Right-Bottom-Front */,
      {SphericalAngle::FromDegrees(110.0f, 5.0f)} /* Left-Top-Back */,
      {SphericalAngle::FromDegrees(-120.0f, 15.0f)} /* Right-Top-Back */,
      {SphericalAngle::FromDegrees(130.0f, -25.0f)} /* Left-Bottom-Back */,
      {SphericalAngle::FromDegrees(220.0f, -35.0f)} /* Right-Bottom-Back */,
  };

  AmbisonicLookupTable lookup_table_;
  const int source_ambisonic_order_;
  std::vector<float> encoding_coeffs_;
};

// Tests whether GetEncodingCoeffs() method returns correct coefficients.
TEST_P(AmbisonicLookupTableTest, GetEncodingCoeffsTest) {
  TestEncodingCoefficients();
}

// Tests whether changing the source spread results in correct gain changes in
// pressure and velocity ambisonic encoding coefficients. For example,
// increasing of the source spread should result in overall monotonic increase
// of energy in the pressure channel and overall monotonic decrease in the
// velocity channels.
TEST_P(AmbisonicLookupTableTest, SpreadEnergyTest) { TestSpreadEnergy(); }

INSTANTIATE_TEST_CASE_P(TestParameters, AmbisonicLookupTableTest,
                        testing::Values(kSourceAmbisonicOrder0,
                                        kSourceAmbisonicOrder1,
                                        kSourceAmbisonicOrder2,
                                        kSourceAmbisonicOrder3));

class PreComputedCoeffsTest : public ::testing::Test {
 protected:
  // Tests whether the lookup table returns correct coefficients for sources
  // with arbitrary ambisonic order, direction and spread.
  void TestSpreadCoefficients() {
    // Test spread coefficients for each config.
    for (const auto& config : kConfigs) {
      const int source_ambisonic_order = config.source_ambisonic_order;
      const SphericalAngle& source_direction = config.source_direction;
      const float source_spread_deg = config.source_spread_deg;
      const std::vector<float>& expected_coeffs = config.expected_coefficients;
      std::vector<float> encoding_coeffs(
          GetNumPeriphonicComponents(source_ambisonic_order));
      AmbisonicLookupTable lookup_table(kMaxSupportedAmbisonicOrder);
      lookup_table.GetEncodingCoeffs(source_ambisonic_order, source_direction,
                                     source_spread_deg, &encoding_coeffs);
      for (size_t i = 0; i < encoding_coeffs.size(); ++i) {
        EXPECT_NEAR(expected_coeffs[i], encoding_coeffs[i], kEpsilonFloat);
      }
    }
  }

 private:
  struct TestConfig {
    int source_ambisonic_order;
    SphericalAngle source_direction;
    float source_spread_deg;
    std::vector<float> expected_coefficients;
  };

  const std::vector<TestConfig> kConfigs = {
      // Zeroth order sound source.
      {0 /* ambisonic order */,
       SphericalAngle::FromDegrees(0.0f, 0.0f) /* source direction */,
       120.0f /* source spread */,
       {1.0f} /* expected coefficients */},

      // First order sound source.
      {1 /* ambisonic order */,
       SphericalAngle::FromDegrees(36.0f, 45.0f) /* source direction */,
       70.0f /* source spread */,
       {1.20046f, 0.310569f, 0.528372f, 0.427462f}
       /* expected coefficients */},

      // Second order sound source.
      {2 /* ambisonic order */,
       SphericalAngle::FromDegrees(55.0f, -66.0f) /* source direction */,
       41.0f /* source spread */,
       {1.038650f, 0.337027f, -0.924096f, 0.2359899f, 0.124062f, -0.485807f,
        0.6928289f, -0.340166f, -0.045155f}
       /* expected coefficients */},

      // Third order sound source.
      {3 /* ambisonic order */,
       SphericalAngle::FromDegrees(-13.0f, 90.0f) /* source direction */,
       32.0f /* source spread */,
       {1.03237f, 0.0f, 1.02119f, 0.0f, 0.0f, 0.0f, 0.990433f, 0.0f, 0.0f, 0.0f,
        0.0f, 0.0f, 0.898572f, 0.0f, 0.0f, 0.0f}
       /* expected coefficients */}};
};

// Tests whether the lookup table returns correct coefficients for sources with
// arbitrary ambisonic order, direction and spread. The expected coefficients
// have been pre-computed using Matlab.
TEST_F(PreComputedCoeffsTest, SpreadCoefficientsTest) {
  TestSpreadCoefficients();
}

}  // namespace vraudio
