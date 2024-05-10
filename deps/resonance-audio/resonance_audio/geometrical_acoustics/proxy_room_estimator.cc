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

#include "geometrical_acoustics/proxy_room_estimator.h"

#include <algorithm>
#include <cmath>

#include "Eigen/Core"
#include "base/logging.h"
#include "platforms/common/room_effects_utils.h"

namespace vraudio {

namespace {

// Normals of the walls of an axis-aligned cube. Walls are indexed like this:
//  0: Left   (-x)
//  1: Right  (+x)
//  2: Bottom (-y)
//  3: Top    (+y)
//  4: Back   (-z)
//  5: Front  (+z)
static const float kCubeWallNormals[kNumRoomSurfaces][3] = {
    {-1.0f, 0.0f, 0.0f}, {1.0f, 0.0f, 0.0f},  {0.0f, -1.0f, 0.0f},
    {0.0f, 1.0f, 0.0f},  {0.0f, 0.0f, -1.0f}, {0.0f, 0.0f, 1.0f}};

// A helper function to determine if a ray (shooting from |ray_origin| in the
// |ray_direction| direction) intersects with a plane (whose normal is
// |plane_normal| and whose distance to the origin is
// |plane_distance_to_origin|). When returning true, the intersection point is
// stored in |intersection|.
bool PlaneIntersection(const WorldPosition& plane_normal,
                       float plane_distance_to_origin,
                       const WorldPosition& ray_origin,
                       const WorldPosition& ray_direction,
                       WorldPosition* intersection) {
  // Do not count if the direction is too close to parallel to the plane (i.e.,
  // the dot product of |direction| and |plane_normal|, |d_dot_n| is too close
  // to zero).
  const float d_dot_n = ray_direction.dot(plane_normal);
  if (std::abs(d_dot_n) < vraudio::kEpsilonFloat) {
    return false;
  }

  // Find the intersection point.
  const float t =
      (plane_distance_to_origin - ray_origin.dot(plane_normal)) / d_dot_n;

  // Do not count back-intersection, i.e., the intersection is in the negative
  // |ray_direction|.
  if (t < 0.0f) {
    return false;
  }

  *intersection = ray_origin + t * ray_direction;
  return true;
}

// A helper function to find the index of the wall with which the ray
// intersects. Returns true if such wall is found.
bool FindIntersectingWallIndex(const WorldPosition& dimensions,
                               const WorldPosition& origin,
                               const WorldPosition& direction,
                               size_t* wall_index) {
  // The ray intersects a wall if:
  // 1. It intersects the plane of the wall.
  // 2. The intersection point lies in the bounding box of the wall.
  const float dx = dimensions[0] * 0.5f;
  const float dy = dimensions[1] * 0.5f;
  const float dz = dimensions[2] * 0.5f;

  // Data used to test plane intersections (1. above): normals and distances
  // of all the walls.
  const float wall_distances[kNumRoomSurfaces] = {dx, dx, dy, dy, dz, dz};

  // Data used to test whether the intersecting point is in the bounding box
  // of the wall (2. above). The bounding box has a small thickness
  // |wall_thickness| along the normal direction of the wall.
  const float wall_thickness = AcousticRay::kRayEpsilon;

  // The centers and corresponding dimensions defining the bounding boxes of
  // all the walls.
  const WorldPosition wall_centers[kNumRoomSurfaces] = {
      {-dx, 0.0f, 0.0f}, {+dx, 0.0f, 0.0f}, {0.0f, -dy, 0.0f},
      {0.0f, +dy, 0.0f}, {0.0f, 0.0f, -dz}, {0.0f, 0.0f, +dz},
  };
  const WorldPosition wall_dimensions[kNumRoomSurfaces] = {
      {wall_thickness, dimensions[1], dimensions[2]},
      {wall_thickness, dimensions[1], dimensions[2]},
      {dimensions[0], wall_thickness, dimensions[2]},
      {dimensions[0], wall_thickness, dimensions[2]},
      {dimensions[0], dimensions[1], wall_thickness},
      {dimensions[0], dimensions[1], wall_thickness},
  };

  // Iterate through all the walls to find the one that the ray intersects with.
  for (size_t wall = 0; wall < kNumRoomSurfaces; ++wall) {
    WorldPosition intersection;
    if (PlaneIntersection(WorldPosition(kCubeWallNormals[wall]),
                          wall_distances[wall], origin, direction,
                          &intersection) &&
        IsPositionInAabb(intersection, wall_centers[wall],
                         wall_dimensions[wall])) {
      *wall_index = wall;
      return true;
    }
  }

  return false;
}

// A helper function to sort a vector of {position, distance} pairs according
// to the distances and exclude outliers.
std::vector<std::pair<WorldPosition, float>>
SortPositionsAndDistancesAndFindInliers(
    float outlier_portion, const std::vector<std::pair<WorldPosition, float>>&
                               positions_and_distances) {
  std::vector<std::pair<WorldPosition, float>>
      sorted_inlier_positions_and_distances(positions_and_distances);

  std::sort(sorted_inlier_positions_and_distances.begin(),
            sorted_inlier_positions_and_distances.end(),
            [](const std::pair<WorldPosition, float>& position_and_distance_1,
               const std::pair<WorldPosition, float>& position_and_distance_2) {
              return position_and_distance_1.second <
                     position_and_distance_2.second;
            });

  const float total_size =
      static_cast<float>(sorted_inlier_positions_and_distances.size());
  const size_t inliner_begin =
      static_cast<size_t>(std::floor(total_size * outlier_portion));
  const size_t inliner_end =
      static_cast<size_t>(std::ceil(total_size * (1.0f - outlier_portion)));

  std::copy(sorted_inlier_positions_and_distances.begin() + inliner_begin,
            sorted_inlier_positions_and_distances.begin() + inliner_end,
            sorted_inlier_positions_and_distances.begin());
  sorted_inlier_positions_and_distances.resize(inliner_end - inliner_begin);

  return sorted_inlier_positions_and_distances;
}

// A helper function to compute the average position and distance from a
// vector of positions and distances.
void ComputeAveragePositionAndDistance(
    const std::vector<std::pair<WorldPosition, float>>& positions_and_distances,
    WorldPosition* average_position, float* average_distance) {
  DCHECK(!positions_and_distances.empty());
  WorldPosition sum_positions(0.0f, 0.0f, 0.0f);
  float sum_distances = 0.0f;
  for (const std::pair<WorldPosition, float>& position_and_distance :
       positions_and_distances) {
    sum_positions += position_and_distance.first;
    sum_distances += position_and_distance.second;
  }

  const float inverse_size =
      1.0f / static_cast<float>(positions_and_distances.size());
  *average_position = sum_positions * inverse_size;
  *average_distance = sum_distances * inverse_size;
}

}  // namespace

void ProxyRoomEstimator::CollectHitPointData(
    const std::vector<Path>& paths_batch) {
  // The size of already collected hit points before this batch.
  const size_t previous_size = hit_points_.size();
  hit_points_.resize(previous_size + paths_batch.size());

  // Collect one hit point per path.
  for (size_t path_index = 0; path_index < paths_batch.size(); ++path_index) {
    const size_t hit_point_index = path_index + previous_size;
    const Path& path = paths_batch.at(path_index);
    hit_points_[hit_point_index] = CollectHitPointDataFromPath(path);
  }
}

bool ProxyRoomEstimator::EstimateCubicProxyRoom(
    float outlier_portion, RoomProperties* room_properties) {
  // Check that |outlier_portion| is in the range of [0, 0.5].
  CHECK_GE(outlier_portion, 0.0f);
  CHECK_LE(outlier_portion, 0.5f);

  // Estimation fails if there is no hit point.
  if (hit_points_.empty()) {
    return false;
  }

  // The geometry part (position, dimensions, and rotation) of the proxy room.
  if (!EstimateCubeGeometry(outlier_portion, room_properties->position,
                            room_properties->dimensions,
                            room_properties->rotation)) {
    LOG(WARNING) << "Unable to estimate a cube without a hit point with finite "
                 << "|t_far|; returning a default cube.";
    return false;
  }

  // The surface material part of the proxy room.
  EstimateSurfaceMaterials(WorldPosition(room_properties->position),
                           WorldPosition(room_properties->dimensions),
                           room_properties->material_names);
  return true;
}

ProxyRoomEstimator::HitPointData
ProxyRoomEstimator::CollectHitPointDataFromPath(const Path& path) {
  CHECK(!path.rays.empty());
  HitPointData hit_point;

  // Common data.
  const AcousticRay& first_order_ray = path.rays[0];
  hit_point.origin = WorldPosition(first_order_ray.origin());
  hit_point.direction = WorldPosition(first_order_ray.direction());

  // Treating escaped and non-escaped rays differently for their |t_far| and
  // |absorption_coefficients|.
  if (path.rays.size() > 1) {
    hit_point.t_far = first_order_ray.t_far();

    // Figure out the absorption coefficients by examining the incoming and the
    // reflected energy.
    const AcousticRay& second_order_ray = path.rays[1];
    for (size_t i = 0; i < kNumReverbOctaveBands; ++i) {
      const float incoming_energy = first_order_ray.energies().at(i);
      const float reflected_energy = second_order_ray.energies().at(i);
      CHECK_GT(incoming_energy, 0.0f);
      CHECK_GE(incoming_energy, reflected_energy);

      hit_point.absorption_coefficients[i] =
          1.0f - (reflected_energy / incoming_energy);
    }
  } else {
    hit_point.t_far = AcousticRay::kInfinity;

    // Escaped rays are considered as completely absorbed.
    for (size_t i = 0; i < kNumReverbOctaveBands; ++i) {
      hit_point.absorption_coefficients[i] = 1.0f;
    }
  }

  return hit_point;
}

bool ProxyRoomEstimator::EstimateCubeGeometry(float outlier_portion,
                                              float* position,
                                              float* dimensions,
                                              float* rotation) {
  // First group the hit points according to which cube wall it would hit,
  // assuming an initial guess of a cube that
  // 1. centers at the origin of the first ray,
  // 2. has unit dimensions, and
  // 3. has no rotation.
  const WorldPosition cube_dimensions(1.0f, 1.0f, 1.0f);
  const std::array<std::vector<HitPointData>, kNumRoomSurfaces>
      hit_points_on_walls =
          GroupHitPointsByWalls(hit_points_[0].origin, cube_dimensions);

  // Compute the positions and distances of from the hit points on walls.
  const std::vector<std::pair<WorldPosition, float>> positions_and_distances =
      ComputeDistancesAndPositionsFromHitPoints(hit_points_on_walls);

  // Cannot estimate if there are no valid positions and distances.
  if (positions_and_distances.empty()) {
    return false;
  }

  // Sort the |positions_and_distances| according to the distances and also
  // filter out outliers (whose distances are too large or too small).
  const std::vector<std::pair<WorldPosition, float>>
      sorted_inlier_positions_and_distances =
          SortPositionsAndDistancesAndFindInliers(outlier_portion,
                                                  positions_and_distances);

  // Cannot estimate if there are no inlying positions and distances.
  if (sorted_inlier_positions_and_distances.empty()) {
    return false;
  }

  // Take the average distance and origin of the inlier hit points.
  WorldPosition average_hit_point_poisition;
  float average_distance;
  ComputeAveragePositionAndDistance(sorted_inlier_positions_and_distances,
                                    &average_hit_point_poisition,
                                    &average_distance);

  // Use twice the average distance as the cube's dimensions.
  const float estimated_cube_dimension = 2.0f * average_distance;
  dimensions[0] = estimated_cube_dimension;
  dimensions[1] = estimated_cube_dimension;
  dimensions[2] = estimated_cube_dimension;

  // Use the average hit point position as the cube's position.
  position[0] = average_hit_point_poisition[0];
  position[1] = average_hit_point_poisition[1];
  position[2] = average_hit_point_poisition[2];

  // No rotation.
  rotation[0] = 0.0f;
  rotation[1] = 0.0f;
  rotation[2] = 0.0f;
  rotation[3] = 1.0f;

  return true;
}

std::array<std::vector<ProxyRoomEstimator::HitPointData>, kNumRoomSurfaces>
ProxyRoomEstimator::GroupHitPointsByWalls(
    const WorldPosition& room_position, const WorldPosition& room_dimensions) {
  const WorldPosition kOrigin(0.0f, 0.0f, 0.0f);

  // No rotation for an axis-aligned room.
  const WorldRotation kNoRotation(1.0f, 0.0f, 0.0f, 0.0f);

  // Reserve memory space for hit points on walls.
  std::array<std::vector<HitPointData>, kNumRoomSurfaces> hit_points_on_walls;
  for (std::vector<HitPointData>& hit_points_on_wall : hit_points_on_walls) {
    hit_points_on_wall.reserve(hit_points_.size());
  }

  for (const HitPointData& hit_point : hit_points_) {
    // First transform the ray's origin and direction to the local space
    // of the cube centered at |position|.
    WorldPosition local_direction;
    GetRelativeDirection(kOrigin, kNoRotation, hit_point.direction,
                         &local_direction);
    WorldPosition local_origin;
    GetRelativeDirection(room_position, kNoRotation, hit_point.origin,
                         &local_origin);

    // Find the wall that the ray intersects.
    size_t wall_index;
    if (FindIntersectingWallIndex(room_dimensions, local_origin,
                                  local_direction, &wall_index)) {
      hit_points_on_walls[wall_index].push_back(hit_point);
    }
  }

  return hit_points_on_walls;
}

std::vector<std::pair<WorldPosition, float>>
ProxyRoomEstimator::ComputeDistancesAndPositionsFromHitPoints(
    const std::array<std::vector<HitPointData>, kNumRoomSurfaces>&
        hit_points_on_walls) {
  std::vector<std::pair<WorldPosition, float>> positions_and_distances;
  for (size_t wall = 0; wall < kNumRoomSurfaces; ++wall) {
    const WorldPosition wall_normal(kCubeWallNormals[wall]);
    for (const HitPointData& hit_point : hit_points_on_walls[wall]) {
      if (hit_point.t_far == AcousticRay::kInfinity) {
        continue;
      }

      std::pair<WorldPosition, float> position_and_distance;
      position_and_distance.first =
          hit_point.origin + (hit_point.t_far * hit_point.direction);
      position_and_distance.second =
          (position_and_distance.first - hit_point.origin).dot(wall_normal);
      positions_and_distances.push_back(position_and_distance);
    }
  }

  return positions_and_distances;
}

void ProxyRoomEstimator::EstimateSurfaceMaterials(
    const WorldPosition& room_position, const WorldPosition& room_dimensions,
    MaterialName* material_names) {
  // Compute the average absorption coefficients on all the walls.
  CoefficientsVector average_absorption_coefficients[kNumRoomSurfaces];

  // Now that we have better estimates of the proxy room's position, dimensions,
  // and rotation, re-group the hit points using these estimates.
  const std::array<std::vector<HitPointData>, kNumRoomSurfaces>&
      hit_points_on_walls =
          GroupHitPointsByWalls(room_position, room_dimensions);
  for (size_t wall = 0; wall < kNumRoomSurfaces; ++wall) {
    const std::vector<HitPointData>& hit_points = hit_points_on_walls[wall];
    for (const HitPointData& hit_point : hit_points) {
      average_absorption_coefficients[wall] +=
          CoefficientsVector(hit_point.absorption_coefficients.data());
    }
  }

  for (size_t wall = 0; wall < kNumRoomSurfaces; ++wall) {
    if (!hit_points_on_walls[wall].empty()) {
      average_absorption_coefficients[wall] /=
          static_cast<float>(hit_points_on_walls[wall].size());
    } else {
      // If no hit point is found on this wall, we consider all energy absorbed
      // in this direction and the absorption coefficient being 1.0.
      average_absorption_coefficients[wall].fill(1.0f);
    }
  }

  // Find the closest surface material. The distance between two materials is
  // defined as the simple Euclidean distance.
  for (size_t wall = 0; wall < kNumRoomSurfaces; ++wall) {
    MaterialName min_distance_material_name = MaterialName::kTransparent;
    float min_material_distance = std::numeric_limits<float>::infinity();

    // We want to exclude kUniform from fitting.
    const size_t num_materials_to_fit =
        static_cast<size_t>(MaterialName::kNumMaterialNames) - 1;
    for (size_t material_index = 0; material_index < num_materials_to_fit;
         ++material_index) {
      const RoomMaterial& material = GetRoomMaterial(material_index);
      const float material_distance =
          (average_absorption_coefficients[wall] -
           CoefficientsVector(material.absorption_coefficients))
              .norm();
      if (material_distance < min_material_distance) {
        min_material_distance = material_distance;
        min_distance_material_name = material.name;
      }
    }
    material_names[wall] = min_distance_material_name;
    LOG(INFO) << "Wall[" << wall << "] material= " << min_distance_material_name
              << " distance= " << min_material_distance;
  }
}

}  // namespace vraudio
