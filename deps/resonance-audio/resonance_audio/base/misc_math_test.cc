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

#include "base/misc_math.h"

#include "third_party/googletest/googletest/include/gtest/gtest.h"

#include "base/constants_and_types.h"

namespace vraudio {

namespace {

TEST(MiscMath, WorldPositionEqualityTest) {
  const WorldPosition kOriginalWorldPosition(0.33f, 0.44f, 0.55f);
  const WorldPosition kSameWorldPosition = kOriginalWorldPosition;
  EXPECT_FALSE(kOriginalWorldPosition != kSameWorldPosition);
}

TEST(MiscMath, WorldPositionInequalityTest) {
  const WorldPosition kOriginalWorldPosition(0.11f, 0.22f, 0.33f);
  const std::vector<WorldPosition> kDifferentWorldPositions{
      {-0.22f, 0.22f, 0.33f}, {0.11f, -0.33f, 0.33f}, {0.11f, 0.22f, -0.22f},
      {0.11f, 0.33f, -0.44f}, {0.22f, 0.22f, -0.44f}, {0.22f, 0.33f, -0.55f}};

  for (auto& position : kDifferentWorldPositions) {
    EXPECT_TRUE(kOriginalWorldPosition != position);
  }
}

TEST(MiscMath, ConvertAudioFromWorldPositionTest) {
  static const WorldPosition kWorldPosition(0.5f, -1.2f, 10.f);
  static const AudioPosition kExpectedAudioPosition(
      -kWorldPosition[2], -kWorldPosition[0], kWorldPosition[1]);
  AudioPosition test_position;
  ConvertAudioFromWorldPosition(kWorldPosition, &test_position);

  EXPECT_TRUE(kExpectedAudioPosition.isApprox(test_position, kEpsilonFloat));
}

TEST(MiscMath, ConvertWorldFromAudioPositionTest) {
  static const AudioPosition kAudioPosition(1.0f, 2.0f, -0.2f);
  static const WorldPosition kExpectedWorldPosition(
      -kAudioPosition[1], kAudioPosition[2], -kAudioPosition[0]);

  WorldPosition test_position;
  ConvertWorldFromAudioPosition(kAudioPosition, &test_position);

  EXPECT_TRUE(kExpectedWorldPosition.isApprox(test_position, kEpsilonFloat));
}

TEST(MiscMath, ConvertAudioFromWorldRotationTest) {
  static const WorldRotation kWorldRotation(1.0f, 0.5f, -1.2f, 10.f);
  static const AudioRotation kExpectedAudioRotation(
      kWorldRotation.w(), -kWorldRotation.x(), kWorldRotation.y(),
      -kWorldRotation.z());
  AudioRotation test_rotation;
  ConvertAudioFromWorldRotation(kWorldRotation, &test_rotation);

  EXPECT_TRUE(kExpectedAudioRotation.isApprox(test_rotation, kEpsilonFloat));
}

TEST(MiscMath, GetRelativeDirectionTest) {
  static const WorldPosition kFromPosition(0.0f, 0.0f, 0.0f);
  static const WorldPosition kFromRotationAxis(0.0f, 0.0f, 1.0f);
  static const float kFromRotationAngle = static_cast<float>(M_PI / 2.0);
  WorldRotation kFromRotation =
      WorldRotation(AngleAxisf(kFromRotationAngle, kFromRotationAxis));

  static const WorldPosition kToPosition(1.0f, 2.0f, 3.0f);

  static const WorldPosition kExpectedRelativeDirection(2.0f, -1.0f, 3.0f);
  WorldPosition test_relative_direction;
  GetRelativeDirection(kFromPosition, kFromRotation, kToPosition,
                       &test_relative_direction);

  EXPECT_TRUE(kExpectedRelativeDirection.isApprox(test_relative_direction,
                                                  kEpsilonFloat));
}

TEST(MiscMath, GetClosestPositionInAabbInsideTest) {
  static const WorldPosition kRelativeSourcePosition(0.0f, -0.2f, 0.0f);
  static const WorldPosition kRoomDimensions(1.0f, 1.0f, 1.0f);
  static const WorldPosition kExpectedAabbPosition = kRelativeSourcePosition;
  WorldPosition test_position;
  GetClosestPositionInAabb(kRelativeSourcePosition, kRoomDimensions,
                           &test_position);

  EXPECT_TRUE(kExpectedAabbPosition.isApprox(test_position, kEpsilonFloat));
}

TEST(MiscMath, GetClosestPositionInAabbOutsideTest) {
  static const WorldPosition kRelativeSourcePosition(0.2f, 0.7f, -0.5f);
  static const WorldPosition kRoomDimensions(1.0f, 1.0f, 1.0f);
  static const WorldPosition kExpectedAabbPosition(0.2f, 0.5f, -0.5f);
  WorldPosition test_position;
  GetClosestPositionInAabb(kRelativeSourcePosition, kRoomDimensions,
                           &test_position);

  EXPECT_TRUE(kExpectedAabbPosition.isApprox(test_position, kEpsilonFloat));
}

TEST(MiscMath, IsPositionInAabbInsideTest) {
  static const WorldPosition kSourcePosition(0.5f, 0.3f, 0.2f);
  static const WorldPosition kRoomPosition(0.5f, 0.5f, 0.5f);
  static const WorldPosition kRoomDimensions(1.0f, 1.0f, 1.0f);

  EXPECT_TRUE(
      IsPositionInAabb(kSourcePosition, kRoomPosition, kRoomDimensions));
}

TEST(MiscMath, IsPositionInAabbOutsideTest) {
  static const WorldPosition kSourcePosition(0.7f, 1.2f, 0.0f);
  static const WorldPosition kRoomPosition(0.5f, 0.5f, 0.5f);
  static const WorldPosition kRoomDimensions(1.0f, 1.0f, 1.0f);

  EXPECT_FALSE(
      IsPositionInAabb(kSourcePosition, kRoomPosition, kRoomDimensions));
}

TEST(MiscMath, IntegerMultiplicationOverflowDetection) {
  static const size_t kMaxValue = std::numeric_limits<size_t>::max();
  static const size_t kHalfMaxValue = kMaxValue / 2;

  // 2 * 3 == 6 should not lead to an integer overflow.
  EXPECT_FALSE(DoesIntegerMultiplicationOverflow<size_t>(2, 3, 6));

  EXPECT_FALSE(DoesIntegerMultiplicationOverflow<size_t>(kHalfMaxValue, 2,
                                                         kHalfMaxValue * 2));
  EXPECT_TRUE(
      DoesIntegerMultiplicationOverflow<size_t>(kMaxValue, 2, kMaxValue << 1));
  EXPECT_FALSE(
      DoesIntegerMultiplicationOverflow<size_t>(0, kMaxValue, 0 * kMaxValue));
  EXPECT_FALSE(
      DoesIntegerMultiplicationOverflow<size_t>(kMaxValue, 0, kMaxValue * 0));
}

TEST(MiscMath, DoesIntegerAdditionOverflow) {
  static const size_t kMaxValue = std::numeric_limits<size_t>::max();
  static const size_t kHalfMaxValue = kMaxValue / 2;

  EXPECT_FALSE(
      DoesIntegerAdditionOverflow<size_t>(kHalfMaxValue, kHalfMaxValue));
  EXPECT_TRUE(DoesIntegerAdditionOverflow<size_t>(kMaxValue, kHalfMaxValue));
  EXPECT_TRUE(DoesIntegerAdditionOverflow<size_t>(1, kMaxValue));
  EXPECT_FALSE(DoesIntegerAdditionOverflow<size_t>(kMaxValue, 0));
  EXPECT_FALSE(DoesIntegerAdditionOverflow<size_t>(0, kMaxValue));
}

TEST(MiscMath, DoesIntSafelyConvertToSizeT) {
  static const int kMaxIntValue = std::numeric_limits<int>::max();
  size_t test_size_t;
  EXPECT_TRUE(DoesIntSafelyConvertToSizeT(kMaxIntValue, &test_size_t));
  EXPECT_EQ(static_cast<size_t>(kMaxIntValue), test_size_t);
  EXPECT_TRUE(DoesIntSafelyConvertToSizeT(0, &test_size_t));
  EXPECT_EQ(0U, test_size_t);
  EXPECT_FALSE(DoesIntSafelyConvertToSizeT(-1, &test_size_t));
}

TEST(MiscMath, DoesSizeTSafelyConvertToInt) {
  static const size_t kMaxIntValue = std::numeric_limits<size_t>::max();
  int test_int;

  EXPECT_FALSE(DoesSizeTSafelyConvertToInt(kMaxIntValue, &test_int));
  EXPECT_TRUE(
      DoesSizeTSafelyConvertToInt(std::numeric_limits<int>::max(), &test_int));
  EXPECT_EQ(std::numeric_limits<int>::max(), test_int);
  EXPECT_TRUE(DoesSizeTSafelyConvertToInt(0, &test_int));
  EXPECT_EQ(0, test_int);
}

TEST(MiscMath, GreatestCommonDivisorTest) {
  const std::vector<int> a_values = {2, 10, 3, 5, 48000, 7, -2, 2, -3};
  const std::vector<int> b_values = {8, 4, 1, 10, 24000, 13, 6, -6, -9};
  const std::vector<int> expected = {2, 2, 1, 5, 24000, 1, 2, 2, 3};

  for (size_t i = 0; i < expected.size(); ++i) {
    EXPECT_EQ(expected[i], FindGcd(a_values[i], b_values[i]));
  }
}

TEST(MiscMath, NextPowTwoTest) {
  const std::vector<size_t> inputs = {2, 10, 3, 5, 48000, 7, 23, 32};
  const std::vector<size_t> expected = {2, 16, 4, 8, 65536, 8, 32, 32};

  for (size_t i = 0; i < inputs.size(); ++i) {
    EXPECT_EQ(expected[i], NextPowTwo(inputs[i]));
  }
}

TEST(MiscMath, EqualSafeEqualArraysTest) {
  const float kOriginalArray[3] = {0.11f, 0.22f, 0.33f};
  const float kSameArray[3] = {0.11f, 0.22f, 0.33f};

  EXPECT_TRUE(EqualSafe(std::begin(kOriginalArray), std::end(kOriginalArray),
                        std::begin(kSameArray), std::end(kSameArray)));
}

TEST(MiscMath, EqualSafeUnequalArraysTest) {
  const std::vector<float> kOriginalArray{0.11f, 0.22f, 0.33f};
  const std::vector<std::vector<float>> kDifferentArrays{
      {-0.22f, 0.22f, 0.33f}, {0.11f, -0.33f, 0.33f}, {0.11f, 0.22f, -0.22f},
      {0.11f, 0.33f, -0.44f}, {0.22f, 0.22f, -0.44f}, {0.22f, 0.33f, -0.55f}};

  for (auto& array : kDifferentArrays) {
    EXPECT_FALSE(EqualSafe(std::begin(kOriginalArray), std::end(kOriginalArray),
                           std::begin(array), std::end(array)));
  }
}

TEST(MiscMath, FastReciprocalSqrtTest) {
  const std::vector<float> kNumbers{130.0f, 13.0f,  1.3f,
                                    0.13f,  0.013f, 0.0013f};
  const float kSqrtEpsilon = 2e-3f;
  for (auto& number : kNumbers) {
    const float actual = std::sqrt(number);
    const float approximate = 1.0f / FastReciprocalSqrt(number);
    EXPECT_LT(std::abs(actual - approximate) / actual, kSqrtEpsilon);
  }
}

TEST(MiscMath, LinearFittingArrayDifferentSizesFails) {
  const std::vector<float> x_array{1.0f, 2.0f};
  const std::vector<float> y_array{3.0f, 4.0f, 5.0f};
  float slope = 0.0f;
  float intercept = 0.0f;
  float r_squared = 0.0f;
  EXPECT_FALSE(LinearLeastSquareFitting(x_array, y_array, &slope, &intercept,
                                        &r_squared));
}

TEST(MiscMath, LinearFittingFewerThanTwoPointsFails) {
  const std::vector<float> x_array{1.0f};
  const std::vector<float> y_array{2.0f};
  float slope = 0.0f;
  float intercept = 0.0f;
  float r_squared = 0.0f;
  EXPECT_FALSE(LinearLeastSquareFitting(x_array, y_array, &slope, &intercept,
                                        &r_squared));
}

TEST(MiscMath, LinearFittingVerticalLineFails) {
  // All points line up on the y-axis.
  const std::vector<float> x_array{0.0f, 0.0f, 0.0f};
  const std::vector<float> y_array{1.0f, 2.0f, 3.0f};
  float slope = 0.0f;
  float intercept = 0.0f;
  float r_squared = 0.0f;
  EXPECT_FALSE(LinearLeastSquareFitting(x_array, y_array, &slope, &intercept,
                                        &r_squared));
}

TEST(MiscMath, LinearFittingHorizontalLine) {
  // All points line up on the x-axis.
  const std::vector<float> x_array{1.0f, 2.0f, 3.0f};
  const std::vector<float> y_array{0.0f, 0.0f, 0.0f};
  float slope = 0.0f;
  float intercept = 0.0f;
  float r_squared = 0.0f;
  EXPECT_TRUE(LinearLeastSquareFitting(x_array, y_array, &slope, &intercept,
                                       &r_squared));
  EXPECT_FLOAT_EQ(slope, 0.0f);
  EXPECT_FLOAT_EQ(intercept, 0.0f);
  EXPECT_FLOAT_EQ(r_squared, 1.0f);
}

TEST(MiscMath, LinearFittingSlopedLine) {
  // All points line up on the line y = 2.0 x + 1.0.
  const std::vector<float> x_array{1.0f, 2.0f, 3.0f, 4.0f};
  const std::vector<float> y_array{3.0f, 5.0f, 7.0f, 9.0f};
  float slope = 0.0f;
  float intercept = 0.0f;
  float r_squared = 0.0f;
  EXPECT_TRUE(LinearLeastSquareFitting(x_array, y_array, &slope, &intercept,
                                       &r_squared));
  EXPECT_FLOAT_EQ(slope, 2.0f);
  EXPECT_FLOAT_EQ(intercept, 1.0f);
  EXPECT_FLOAT_EQ(r_squared, 1.0f);
}

TEST(MiscMath, LinearFittingSlopedLineWithError) {
  // All points lie close to the line y = 2.0 x + 1.0 with some offsets.
  const std::vector<float> x_array{1.002f, 2.001f, 2.998f, 4.003f};
  const std::vector<float> y_array{3.001f, 4.998f, 7.005f, 8.996f};
  float slope = 0.0f;
  float intercept = 0.0f;
  float r_squared = 0.0f;
  EXPECT_TRUE(LinearLeastSquareFitting(x_array, y_array, &slope, &intercept,
                                       &r_squared));

  // Expect that the fitting is close to the line with some error.
  const float error_tolerance = 1e-3f;
  EXPECT_NEAR(slope, 2.0f, error_tolerance);
  EXPECT_NEAR(intercept, 1.0f, error_tolerance);
  EXPECT_NEAR(r_squared, 1.0f, error_tolerance);
}

TEST(MiscMath, LinearFittingUncorrelatedPoints) {
  // All points evenly distributed on a circle y^2 + x^2 = 1.0, which gives
  // the worst coefficient of determination (almost zero).
  const size_t num_points = 20;
  std::vector<float> x_array(num_points, 0.0f);
  std::vector<float> y_array(num_points, 0.0f);
  for (size_t i = 0; i < num_points; ++i) {
    const float theta =
        kTwoPi * static_cast<float>(i) / static_cast<float>(num_points);
    x_array[i] = std::cos(theta);
    y_array[i] = std::sin(theta);
  }

  float slope = 0.0f;
  float intercept = 0.0f;
  float r_squared = 0.0f;
  EXPECT_TRUE(LinearLeastSquareFitting(x_array, y_array, &slope, &intercept,
                                       &r_squared));
  EXPECT_FLOAT_EQ(r_squared, 0.0f);
}

TEST(MiscMath, WorldRotation) {
  // Test rotation around single quaternion axis.
  const float kAngularRandomOffsetRad = 0.5f;
  const float kAngularDifferenceRad = 0.3f;
  Eigen::AngleAxisf rotation_a(kAngularRandomOffsetRad,
                               Eigen::Vector3f::UnitY());
  Eigen::AngleAxisf rotation_b(kAngularRandomOffsetRad + kAngularDifferenceRad,
                               Eigen::Vector3f::UnitY());
  EXPECT_FLOAT_EQ(WorldRotation(rotation_a).AngularDifferenceRad(rotation_b),
                  kAngularDifferenceRad);

  // Test rotation between axis.
  Eigen::AngleAxisf rotation_c(0.0f, Eigen::Vector3f::UnitY());
  Eigen::AngleAxisf rotation_d(kPi, Eigen::Vector3f::UnitZ());
  EXPECT_FLOAT_EQ(WorldRotation(rotation_c).AngularDifferenceRad(rotation_d),
                  kPi);
}

TEST(MiscMath, IntegerPow) {
  const float kFloatValue = 1.5f;
  const float kNegativeFloatValue = -3.3f;
  const size_t kSizeTValue = 11U;
  const int kIntValue = 5;
  const int kNegativeIntValue = -13;

  for (int exponent = 0; exponent < 5; ++exponent) {
    EXPECT_FLOAT_EQ(IntegerPow(kFloatValue, exponent),
                    std::pow(kFloatValue, static_cast<float>(exponent)));
    EXPECT_FLOAT_EQ(
        IntegerPow(kNegativeFloatValue, exponent),
        std::pow(kNegativeFloatValue, static_cast<float>(exponent)));
    EXPECT_EQ(IntegerPow(kSizeTValue, exponent),
              std::pow(kSizeTValue, exponent));
    EXPECT_EQ(IntegerPow(kIntValue, exponent), std::pow(kIntValue, exponent));
    EXPECT_EQ(IntegerPow(kNegativeIntValue, exponent),
              std::pow(kNegativeIntValue, exponent));
  }
}

}  // namespace

}  // namespace vraudio
