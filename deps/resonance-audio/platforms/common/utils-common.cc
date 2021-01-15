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

namespace vraudio {

void FlipZAxis(Eigen::Matrix4f* matrix) {
  // This operation is equivalent to:
  // matrix = flipZ * matrix * flipZ
  // where flipZ is the scale(1, 1, -1) matrix.
  (*matrix)(0, 2) *= -1.0f;
  (*matrix)(1, 2) *= -1.0f;
  (*matrix)(2, 0) *= -1.0f;
  (*matrix)(2, 1) *= -1.0f;
  (*matrix)(2, 3) *= -1.0f;
  (*matrix)(3, 2) *= -1.0f;
}

Eigen::Quaternionf GetQuaternion(const Eigen::Matrix4f& matrix) {
  const Eigen::Matrix3f rotation_matrix = matrix.block(0, 0, 3, 3);
  Eigen::Quaternionf quaternion(rotation_matrix);
  return quaternion.normalized();
}

Eigen::Vector3f GetPosition(const Eigen::Matrix4f& matrix) {
  return Eigen::Vector3f(matrix.col(3).head<3>());
}

Eigen::Matrix4f GetTransformMatrix(const Eigen::Vector3f& position,
                                   const Eigen::Vector3f& forward,
                                   const Eigen::Vector3f& up) {
  Eigen::Matrix4f transform_matrix;
  // Compose the homogeneous vectors for the transformation matrix.
  const Eigen::Vector3f right = up.cross(forward);
  const Eigen::Vector4f position_4(position.x(), position.y(), position.z(),
                                   1.0f);
  const Eigen::Vector4f forward_4(forward.x(), forward.y(), forward.z(), 0.0f);
  const Eigen::Vector4f up_4(up.x(), up.y(), up.z(), 0.0f);
  const Eigen::Vector4f right_4(right.x(), right.y(), right.z(), 0.0f);
  // Fill in the transformation matrix.
  transform_matrix << right_4, up_4, forward_4, position_4;
  return transform_matrix;
}

}  // namespace vraudio
