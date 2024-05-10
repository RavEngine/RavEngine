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

#ifndef RESONANCE_AUDIO_GEOMETRICAL_ACOUSTICS_PROXY_ROOM_ESTIMATOR_H_
#define RESONANCE_AUDIO_GEOMETRICAL_ACOUSTICS_PROXY_ROOM_ESTIMATOR_H_

#include <array>
#include <utility>
#include <vector>

#include "Eigen/Core"
#include "api/resonance_audio_api.h"
#include "base/constants_and_types.h"
#include "base/misc_math.h"
#include "geometrical_acoustics/path.h"
#include "platforms/common/room_properties.h"

namespace vraudio {

// A class that estimates a "proxy room" from traced sound propagation paths.
// A proxy room is used to model dynamic early reflections as if they are
// reflected from a box-shaped room, even though the real scene geometry is
// arbitrarily complex. This is to complement the pre-computed late reverb
// effects, and it takes the same input (i.e., the ray tracing results) as the
// reverb pre-computation. The proxy room is estimated from the first order
// ray paths (i.e., those from the source to the first hit points), and the
// estimation has two stages:
//   1. Fitting the geometry (currently only as an axis-aligned cube).
//   2. Fitting the surface materials on the six walls.
class ProxyRoomEstimator {
 public:
  ProxyRoomEstimator() = default;

  // ProxyRoomEstimator is neither copyable nor movable.
  ProxyRoomEstimator(const ProxyRoomEstimator&) = delete;
  ProxyRoomEstimator& operator=(const ProxyRoomEstimator&) = delete;

  // Collects hit point data from traced ray paths, batch-by-batch.
  //
  // @param paths_batch A batch of ray paths.
  void CollectHitPointData(const std::vector<Path>& paths_batch);

  // Estimates a cube-shaped proxy room from collected hit points. Hit points
  // are sorted according to their traveled distance. In order to make the
  // estimation more robust, we discard "outlier" hit points, i.e., those whose
  // traveled distances are too large or too small.
  //
  // @param outlier_portion What portion of the hit points are considered as
  //     outliers. For example, a value of 0.1 means that the hit points whose
  //     distances are in the top 10% and bottom 10% are considered as outliers
  //     and discarded. The value must be in the range of [0, 0.5].
  // @param room_properties Room properties of the estimated axis-aligned cube-
  //     shaped room, each wall having an estimated surface material.
  // @return True if the estimation is successful.
  bool EstimateCubicProxyRoom(float outlier_portion,
                              RoomProperties* room_properties);

 private:
  class CoefficientsVector : public Eigen::Matrix<float, kNumReverbOctaveBands,
                                                  1, Eigen::DontAlign> {
   public:
    // Inherits all constructors with 1-or-more arguments. Necessary because
    // MSVC12 doesn't support inheriting constructors.
    template <typename Arg1, typename... Args>
    CoefficientsVector(const Arg1& arg1, Args&&... args)
        : Matrix(arg1, std::forward<Args>(args)...) {}

    // Constructs a zero vector.
    CoefficientsVector() { setZero(); }
  };

  // A struct to contain data necessary for estimating a proxy room.
  struct HitPointData {
    // Origin of the ray that creates this hit point.
    WorldPosition origin;

    // Direction of the ray that creates this hit point.
    WorldPosition direction;

    // Ray parameter t corresponding to the hit point. If the ray escaped the
    // scene and did not hit anything, then |t_far| takes the value of
    // |AcousticRay::kInfinity|. Escaped rays are still useful in estimating
    // surface materials, because they can be considered completely absorbed
    // and should increase the effective absorption coefficients.
    float t_far;

    // Absorption coefficients of the surface of the hit point across the
    // frequency bands.
    std::array<float, kNumReverbOctaveBands> absorption_coefficients;
  };

  // Collects one hit point from one traced ray path.
  //
  // @param path Traced ray path.
  // @return Collected hit point.
  HitPointData CollectHitPointDataFromPath(const Path& path);

  // Estimates the geometry of the cube.
  //
  // @param outlier_portion Portion of all hit points to be discarded. See
  //     EstimateCube() above.
  // @param position Output center position of the estimated cube.
  // @param dimensions Output dimensions of the estimated cube.
  // @return True if the estimation is successful.
  bool EstimateCubeGeometry(float outlier_portion, float* position,
                            float* dimensions, float* rotation);

  // Groups hit points by which walls they lie on in an assumed axis-aligned
  // room.
  //
  // @param room_position Center position of the assumed axis-aligned room.
  // @param room_dimensions Dimensions of the assumed axis-aligned room.
  // @return An array of six elements, each being a vector of hit points
  //     on one of the six walls.
  std::array<std::vector<HitPointData>, kNumRoomSurfaces> GroupHitPointsByWalls(
      const WorldPosition& room_position, const WorldPosition& room_dimensions);

  // Compute the hit point positions and distances (measured along the normal
  // direction of the walls that they hit) from hit points on walls.
  //
  // @param hit_points_on_walls Hit points on walls.
  // @return A vector of {hit point position, distance} pairs.
  std::vector<std::pair<WorldPosition, float>>
  ComputeDistancesAndPositionsFromHitPoints(
      const std::array<std::vector<HitPointData>, kNumRoomSurfaces>&
          hit_points_on_walls);

  // Estimates the surface materials on the six walls of the proxy room.
  //
  // @param room_position Center position of the estimated proxy room.
  // @param room_dimensions Dimensions of the estimated proxy room.
  // @param material_names Names of the estimated surface materials.
  void EstimateSurfaceMaterials(const WorldPosition& room_position,
                                const WorldPosition& room_dimensions,
                                MaterialName* material_names);

  // Collected hit points.
  std::vector<HitPointData> hit_points_;
};

}  // namespace vraudio

#endif  // RESONANCE_AUDIO_GEOMETRICAL_ACOUSTICS_PROXY_ROOM_ESTIMATOR_H_
