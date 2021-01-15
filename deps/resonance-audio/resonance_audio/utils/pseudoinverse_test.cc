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

#include "utils/pseudoinverse.h"

#include "third_party/googletest/googletest/include/gtest/gtest.h"
#include "base/constants_and_types.h"

namespace vraudio {

namespace {

// Tests that the pseudoinverse of an invertible square matrix is equal to the
// inverse of that matrix.
TEST(PseudoinverseTest, SquareInverse) {
  Eigen::Matrix<double, 5, 5> invertible_matrix;
  invertible_matrix << 0.8478, 0.1676, 0.1961, 0.2654, 0.7662, 0.0279, 0.5309,
      0.2043, 0.4947, 0.0918, 0.1367, 0.4714, 0.7113, 0.246, 0.8048, 0.9617,
      0.4378, 0.0259, 0.536, 0.9565, 0.1541, 0.6275, 0.8471, 0.1133, 0.8074;
  ASSERT_NE(0.0, invertible_matrix.determinant());

  auto pseudoinverse = Pseudoinverse(invertible_matrix);
  auto inverse = invertible_matrix.inverse();

  EXPECT_TRUE(pseudoinverse.isApprox(inverse, kEpsilonDouble))
      << "Pseudoinverse: \n"
      << pseudoinverse << " should be within " << kEpsilonDouble
      << " of inverse: \n"
      << inverse;
}

// Tests that the pseudoinverse of a full-rank matrix with more rows than
// columns works successfully.
TEST(PseudoinverseTest, PseudoinverseMoreRows) {
  Eigen::Matrix<double, 5, 4> invertible_matrix;
  invertible_matrix << 0.8478, 0.1676, 0.1961, 0.2654, 0.0279, 0.5309, 0.2043,
      0.4947, 0.1367, 0.4714, 0.7113, 0.246, 0.9617, 0.4378, 0.0259, 0.536,
      0.1541, 0.6275, 0.8471, 0.1133;

  auto pseudoinverse = Pseudoinverse(invertible_matrix);
  auto should_be_identity = pseudoinverse * invertible_matrix;

  EXPECT_TRUE(should_be_identity.isApprox(
      decltype(should_be_identity)::Identity(should_be_identity.rows(),
                                             should_be_identity.cols()),
      kEpsilonDouble))
      << "Matrix should be within " << kEpsilonDouble
      << " of an identity matrix: \n"
      << should_be_identity;
}

// Tests that the pseudoinverse of a full-rank matrix with more columns than
// rows works successfully.
TEST(PseudoinverseTest, PseudoinverseMoreColumns) {
  Eigen::Matrix<double, 4, 5> invertible_matrix;
  invertible_matrix << 0.8478, 0.1676, 0.1961, 0.2654, 0.7662, 0.0279, 0.5309,
      0.2043, 0.4947, 0.0918, 0.1367, 0.4714, 0.7113, 0.246, 0.8048, 0.9617,
      0.4378, 0.0259, 0.536, 0.9565;

  auto pseudoinverse = Pseudoinverse(invertible_matrix);
  auto should_be_identity = invertible_matrix * pseudoinverse;

  EXPECT_TRUE(should_be_identity.isApprox(
      decltype(should_be_identity)::Identity(should_be_identity.rows(),
                                             should_be_identity.cols()),
      kEpsilonDouble))
      << "Matrix should be within " << kEpsilonDouble
      << " of an identity matrix: \n"
      << should_be_identity;
}

}  // namespace

}  // namespace vraudio
