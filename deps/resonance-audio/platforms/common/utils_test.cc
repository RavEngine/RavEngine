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

#include "platforms/common/utils.h"

#include "third_party/googletest/googletest/include/gtest/gtest.h"
#include "base/audio_buffer.h"
#include "base/constants_and_types.h"

namespace vraudio {

namespace {

// Tests that flipping the z-axis of an arbitrary transformation matrix using
// the |FlipZAxis| method gives the expected transformation matrix.
TEST(UtilsTest, FlipZAxisTest) {
  // Test input values for an arbitrary 4x4 matrix.
  const float kInput[] = {1.0f,  2.0f,  3.0f,  4.0f,  5.0f,  6.0f,
                          7.0f,  8.0f,  9.0f,  10.0f, 11.0f, 12.0f,
                          13.0f, 14.0f, 15.0f, 16.0f};
  // Z-axis flip matrix is the TRS matrix of Scale(1, 1, -1).
  const auto kFlipZMatrix((Eigen::Matrix4f() << 1.0f, 0.0f, 0.0f, 0.0f, 0.0f,
                           1.0f, 0.0f, 0.0f, 0.0f, 0.0f, -1.0f, 0.0f, 0.0f,
                           0.0f, 0.0f, 1.0f)
                              .finished());

  // Compute the expected transformation matrix for given |kInput|.
  const auto expected = kFlipZMatrix * Eigen::Matrix4f(kInput) * kFlipZMatrix;
  // Compute the flipped transformation matrix of given |kInput|.
  Eigen::Matrix4f output(kInput);
  FlipZAxis(&output);

  // Verify that the output matrix has expected number of rows & columns.
  EXPECT_EQ(expected.rows(), output.rows());
  EXPECT_EQ(expected.cols(), output.cols());
  // Compare equality for all the elements in the resulting matrices.
  const size_t num_cols = output.cols();
  const size_t num_rows = output.rows();
  for (size_t col = 0; col < num_cols; ++col) {
    for (size_t row = 0; row < num_rows; ++row) {
      EXPECT_EQ(expected(row, col), output(row, col));
    }
  }
}

// Tests that the |GetTransformMatrix| method gives the expected transformation
// matrix, in which the properties remain unchanged when returned via the
// |GetPosition| and |GetQuaternion| methods.
TEST(UtilsTest, GetTransformMatrixAndBackTest) {
  // Arbitrary position.
  const Eigen::Vector3f kInputPosition(1.0f, 5.0f, -10.0f);
  // Arbitrary directions calculated from the orientation of euler angles
  // (45, 90, -90) in degrees.
  const Eigen::Vector3f kInputForward(kInverseSqrtTwo, -kInverseSqrtTwo, 0.0f);
  const Eigen::Vector3f kInputUp(0.0f, 0.0f, -1.0f);
  // Expected values for the transformation matrix from the given
  // |kInputPosition|, |kInputForward| & |kInputUp| vectors.
  const float kExpectedTransform[] = {
      -kInverseSqrtTwo, -kInverseSqrtTwo, 0.0f, 0.0f, 0.0f, 0.0f, -1.0f,  0.0f,
      kInverseSqrtTwo,  -kInverseSqrtTwo, 0.0f, 0.0f, 1.0f, 5.0f, -10.0f, 1.0f};

  // Calculate the transform matrix.
  const auto output_transform =
      GetTransformMatrix(kInputPosition, kInputForward, kInputUp);

  // Test if the resulting transformation matrix has the expected values.
  const size_t num_cols = output_transform.cols();
  const size_t num_rows = output_transform.rows();
  const Eigen::Matrix4f expected_transform(kExpectedTransform);
  for (size_t col = 0; col < num_cols; ++col) {
    for (size_t row = 0; row < num_rows; ++row) {
      EXPECT_EQ(expected_transform(row, col), output_transform(row, col));
    }
  }

  // Verify that the position vector remains unchanged.
  const auto output_position = GetPosition(output_transform);
  EXPECT_TRUE(kInputPosition.isApprox(output_position, kEpsilonFloat));

  // Verify that the forward & up direction vectors remain unchanged.
  const Eigen::Vector3f kForward(0.0f, 0.0f, 1.0f);
  const Eigen::Vector3f kUp(0.0f, 1.0f, 0.0f);
  const auto output_quaternion = GetQuaternion(output_transform);

  const auto output_forward = (output_quaternion * kForward).normalized();
  EXPECT_TRUE(kInputForward.isApprox(output_forward, kEpsilonFloat));

  const auto output_up = (output_quaternion * kUp).normalized();
  EXPECT_TRUE(kInputUp.isApprox(output_up, kEpsilonFloat));
}

}  // namespace

}  // namespace vraudio
