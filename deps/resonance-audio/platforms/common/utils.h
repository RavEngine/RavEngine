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

#ifndef RESONANCE_AUDIO_PLATFORM_COMMON_UTILS_H_
#define RESONANCE_AUDIO_PLATFORM_COMMON_UTILS_H_

#include "Eigen/Dense"

namespace vraudio {

// Flips the z-axis of a transformation matrix, which effectively allows
// switching between the left-handed and right-handed coordinate systems.
//
// @param matrix 4x4 transformation matrix.
// @return 4x4 transformation matrix with the z-axis flipped.
void FlipZAxis(Eigen::Matrix4f* matrix);

// Returns the position vector of a transformation matrix.
//
// @param matrix 4x4 transformation matrix.
// @return 3D position vector.
Eigen::Vector3f GetPosition(const Eigen::Matrix4f& matrix);

// Returns the rotation quaternion of a transformation matrix.
//
// @param matrix 4x4 transformation matrix.
// @return Quaternion representation of the rotation.
Eigen::Quaternionf GetQuaternion(const Eigen::Matrix4f& matrix);

// Returns a transformation matrix from position, forward & up vectors.
//
// @param position Position vector.
// @param forward Forward direction vector describing rotation.
// @param up Up direction vector describing rotation.
// @return 4x4 transformation matrix.
Eigen::Matrix4f GetTransformMatrix(const Eigen::Vector3f& position,
                                   const Eigen::Vector3f& forward,
                                   const Eigen::Vector3f& up);

}  // namespace vraudio

#endif  // RESONANCE_AUDIO_PLATFORM_COMMON_UTILS_H_
