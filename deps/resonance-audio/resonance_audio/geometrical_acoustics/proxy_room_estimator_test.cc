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

#include <random>

#include "third_party/googletest/googletest/include/gtest/gtest.h"
#include "geometrical_acoustics/acoustic_source.h"
#include "geometrical_acoustics/test_util.h"
#include "platforms/common/room_effects_utils.h"
#include "platforms/common/room_properties.h"

namespace vraudio {

namespace {

using Eigen::Vector3f;

class ProxyRoomEstimatorTest : public testing::Test {
 public:
  ProxyRoomEstimatorTest() {
    BuildTestBoxScene({0.0f, 0.0f, 0.0f}, {1.0f, 1.0f, 1.0f}, &cube_vertices_,
                      &cube_triangles_, &cube_wall_triangles_);
  }

 protected:
  void ReflectRaysInPaths(float t_far, std::vector<Path>* paths) {
    for (Path& path : *paths) {
      // Add a reflected ray to the end of each path, whose energy is half
      // the first ray.
      AcousticRay& ray = path.rays.back();
      ray.set_t_far(t_far);
      const float reflected_origin[3] = {
          ray.origin()[0] + ray.t_far() * ray.direction()[0],
          ray.origin()[1] + ray.t_far() * ray.direction()[1],
          ray.origin()[2] + ray.t_far() * ray.direction()[2],
      };
      AcousticRay reflected_ray(reflected_origin, kZDirection, t_far,
                                AcousticRay::kInfinity, kHalfUnitEnergies,
                                AcousticRay::RayType::kSpecular, t_far);
      path.rays.push_back(reflected_ray);
    }
  }

  ProxyRoomEstimator estimator_;
  SceneManager scene_manager_;

  // Ray-tracing related fields.
  const int kNumRays = 2000;
  const int kMaxDepth = 3;
  const float kEnergyThresold = 1e-6f;
  const std::array<float, kNumReverbOctaveBands> kHalfUnitEnergies{
      {0.5f, 0.5f, 0.5f, 0.5f, 0.5f, 0.5f, 0.5f, 0.5f, 0.5f}};
  const float kSourcePosition[3] = {1.0f, 2.0f, 3.0f};
  const float kZDirection[3] = {0.0f, 0.0f, 1.0f};

  // Data describing a cube scene.
  std::vector<Vertex> cube_vertices_;
  std::vector<Triangle> cube_triangles_;
  const float cube_center_[3] = {0.5f, 0.5f, 0.5f};
  std::vector<MaterialName> wall_materials_;

  // Triangles for six walls of a cube. Useful for assigning surface materials.
  std::vector<std::unordered_set<unsigned int>> cube_wall_triangles_;
};

// Tests that estimating from an empty paths batch fails.
TEST_F(ProxyRoomEstimatorTest, EstimateFromEmptyPathsFails) {
  // An empty paths batch.
  std::vector<Path> empty_paths_batch;
  estimator_.CollectHitPointData(empty_paths_batch);

  // Expect that the estimation function returns false.
  const float outlier_portion = 0.0f;
  RoomProperties room_properties;
  EXPECT_FALSE(
      estimator_.EstimateCubicProxyRoom(outlier_portion, &room_properties));
}

// Tests that if all paths in a batch escape the scene before hitting anything,
// the estimation fails.
TEST_F(ProxyRoomEstimatorTest, EstimateFromEscapedPathsFails) {
  // A paths batch in which all rays escape, i.e., have infinite |t_far|.
  std::vector<Path> escaped_paths_batch;
  for (size_t i = 0; i < 10; ++i) {
    Path path;
    path.rays.emplace_back(kSourcePosition,                  // origin
                           kZDirection,                      // direction
                           0.0f,                             // t_near
                           AcousticRay::kInfinity,           // t_far
                           kUnitEnergies,                    // energies
                           AcousticRay::RayType::kSpecular,  // ray_type
                           0.0f                              // prior_distance
    );
    escaped_paths_batch.push_back(path);
  }
  estimator_.CollectHitPointData(escaped_paths_batch);

  // Expect that the estimation function returns false.
  const float outlier_portion = 0.0f;
  RoomProperties room_properties;
  EXPECT_FALSE(
      estimator_.EstimateCubicProxyRoom(outlier_portion, &room_properties));
}

// Tests that if the scene itself is a cube, then the estimated proxy room
// can recover the same cube with a certain error below tolerance.
TEST_F(ProxyRoomEstimatorTest, EstimateCubeFromCubeScene) {
  wall_materials_ = std::vector<MaterialName>{
      MaterialName::kAcousticCeilingTiles,
      MaterialName::kWoodPanel,
      MaterialName::kConcreteBlockPainted,
      MaterialName::kGlassThin,
      MaterialName::kGrass,
      MaterialName::kMarble,
  };

  // Trace rays in a cube scene.
  std::vector<Path> paths = TracePathsInTestcene(
      kNumRays, kMaxDepth, kEnergyThresold, cube_center_, cube_vertices_,
      cube_triangles_, cube_wall_triangles_, wall_materials_, &scene_manager_);

  // Estimate a proxy room.
  estimator_.CollectHitPointData(paths);
  const float outlier_portion = 0.0f;
  RoomProperties room_properties;
  EXPECT_TRUE(
      estimator_.EstimateCubicProxyRoom(outlier_portion, &room_properties));

  // Expect that the estimated proxy room is the same cube, with a certain
  // error in position and dimensions.
  const float position_tolerance = 0.05f;
  ExpectFloat3Close(room_properties.position, cube_center_, position_tolerance);
  const float expected_dimensions[3] = {1.0f, 1.0f, 1.0f};
  const float dimensions_tolerance = 0.01f;
  ExpectFloat3Close(room_properties.dimensions, expected_dimensions,
                    dimensions_tolerance);

  // Expect that the estimated surface materials are the same as the original
  // cube.
  for (size_t wall = 0; wall < 6; ++wall) {
    EXPECT_EQ(room_properties.material_names[wall], wall_materials_[wall]);
  }
}

// Tests that if the scene is a unit sphere, then the estimated proxy room
// is a cube whose center is the same as the sphere. The dimensions cannot
// be exactly the same as the sphere, but can be expected to be within
// a range.
TEST_F(ProxyRoomEstimatorTest, EstimateCubeFromUnitSphereScene) {
  const size_t min_num_paths = 1000;
  std::vector<Path> paths =
      GenerateUniformlyDistributedRayPaths(kSourcePosition, min_num_paths);
  ReflectRaysInPaths(1.0f, &paths);

  estimator_.CollectHitPointData(paths);

  const float outlier_portion = 0.0f;
  RoomProperties room_properties;
  EXPECT_TRUE(
      estimator_.EstimateCubicProxyRoom(outlier_portion, &room_properties));

  // Expect the estimated proxy room is centered at the source position.
  const float position_tolerance = 0.005f;
  ExpectFloat3Close(room_properties.position, kSourcePosition,
                    position_tolerance);

  // Because the unit sphere is not a cube, the estimated proxy room will have
  // dimensions between
  // 1. the smallest cube enclosing the sphere, and
  // 2. the largest cube inside the sphere.
  // Specifically, the proxy room will have dimensions between 1.0 and 2.0, or
  // 1.5 +- 0.5.
  const float expected_dimensions[3] = {1.5f, 1.5f, 1.5f};
  const float dimensions_tolerance = 0.5f;
  ExpectFloat3Close(room_properties.dimensions, expected_dimensions,
                    dimensions_tolerance);

  // Expect that the estimated surface materials are all kConcreteBlockCoarse,
  // which is closest to absorbing half of the energy across all frequency
  // bands.
  for (size_t wall = 0; wall < 6; ++wall) {
    EXPECT_EQ(room_properties.material_names[wall],
              MaterialName::kConcreteBlockCoarse);
  }
}

// Tests that if the scene is a cube with some openings (no walls in some
// directions), then the estimated proxy room will have transparent (fully
// absorbent) walls corresponding to those directions.
TEST_F(ProxyRoomEstimatorTest, EstimateOpenWallsAsTransparent) {
  // Remove the last four triangles (corresponding to the two walls facing
  // the -z and +z directions) from the cube scene.
  cube_triangles_.pop_back();
  cube_triangles_.pop_back();
  cube_triangles_.pop_back();
  cube_triangles_.pop_back();

  wall_materials_ = std::vector<MaterialName>{
      MaterialName::kAcousticCeilingTiles,
      MaterialName::kWoodPanel,
      MaterialName::kConcreteBlockPainted,
      MaterialName::kGlassThin,
  };

  cube_wall_triangles_.pop_back();
  cube_wall_triangles_.pop_back();

  // Trace rays in the semi-open cube scene.
  std::vector<Path> paths = TracePathsInTestcene(
      kNumRays, kMaxDepth, kEnergyThresold, cube_center_, cube_vertices_,
      cube_triangles_, cube_wall_triangles_, wall_materials_, &scene_manager_);

  // Estimate a proxy room.
  estimator_.CollectHitPointData(paths);
  const float outlier_portion = 0.0f;
  RoomProperties room_properties;
  EXPECT_TRUE(
      estimator_.EstimateCubicProxyRoom(outlier_portion, &room_properties));

  // Expect that the estimated proxy room is the same as the original cube.
  const float position_tolerance = 0.005f;
  ExpectFloat3Close(room_properties.position, cube_center_, position_tolerance);
  const float expected_dimensions[3] = {1.0f, 1.0f, 1.0f};
  const float dimensions_tolerance = 0.001f;
  ExpectFloat3Close(room_properties.dimensions, expected_dimensions,
                    dimensions_tolerance);

  // Expect that the estimated surface materials are the same as the original
  // cube for the four remaining walls.
  for (size_t wall = 0; wall < 4; ++wall) {
    EXPECT_EQ(room_properties.material_names[wall], wall_materials_[wall]);
  }

  // Expect that the estimated surface material of the walls corresponding
  // to the open directions as transparent, because rays in these directions
  // all escape and are considered completely absorbed.
  for (size_t wall = 4; wall < 6; ++wall) {
    EXPECT_EQ(room_properties.material_names[wall], MaterialName::kTransparent);
  }
}

// Tests if the scene is a cube, but some hit points are too far or too close
// to the origin, those points can be considered as outliers (if a proper
// |outlier_portion| is set) and will not affect the estimation result.
// Therefore the estimated proxy room is still the same cube.
TEST_F(ProxyRoomEstimatorTest, EstimateCubeIgnoringOutliers) {
  wall_materials_ = std::vector<MaterialName>{
      MaterialName::kAcousticCeilingTiles,
      MaterialName::kWoodPanel,
      MaterialName::kConcreteBlockPainted,
      MaterialName::kGlassThin,
      MaterialName::kGrass,
      MaterialName::kMarble,
  };

  // Trace rays in a cube scene.
  std::vector<Path> paths = TracePathsInTestcene(
      kNumRays, kMaxDepth, kEnergyThresold, cube_center_, cube_vertices_,
      cube_triangles_, cube_wall_triangles_, wall_materials_, &scene_manager_);

  // Modify the traced paths so that some hit points are too close or too far.
  const size_t num_points_too_close = 100;
  const float distance_too_close = 0.1f;
  for (size_t i = 0; i < num_points_too_close; ++i) {
    paths[0].rays[0].tfar = distance_too_close;
  }
  const size_t num_points_too_far = 100;
  const float distance_too_far = 10.0f;
  for (size_t i = num_points_too_close;
       i < num_points_too_far + num_points_too_close; ++i) {
    paths[0].rays[0].tfar = distance_too_far;
  }

  // Estimate a proxy room with a |outlier_portion| carefully chosen such that
  // the hit points too close or too far do not affect the estimation result.
  estimator_.CollectHitPointData(paths);
  const float outlier_portion =
      static_cast<float>(num_points_too_close + num_points_too_far) /
      static_cast<float>(paths.size()) / 2.0f;
  RoomProperties room_properties;
  EXPECT_TRUE(
      estimator_.EstimateCubicProxyRoom(outlier_portion, &room_properties));

  // Expect that the estimated proxy room is still the same as the original
  // cube, even in the presence of the hit points too close or too far.
  const float position_tolerance = 0.05f;
  ExpectFloat3Close(room_properties.position, cube_center_, position_tolerance);
  const float expected_dimensions[3] = {1.0f, 1.0f, 1.0f};
  const float dimensions_tolerance = 0.01f;
  ExpectFloat3Close(room_properties.dimensions, expected_dimensions,
                    dimensions_tolerance);

  // Expect that the estimated surface materials are the same as the original
  // cube.
  for (size_t wall = 0; wall < 6; ++wall) {
    EXPECT_EQ(room_properties.material_names[wall], wall_materials_[wall]);
  }
}

}  // namespace

}  // namespace vraudio
