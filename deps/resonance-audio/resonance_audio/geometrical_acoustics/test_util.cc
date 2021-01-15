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

#include "geometrical_acoustics/test_util.h"

#include <algorithm>
#include <random>
#include <vector>

#include "third_party/googletest/googletest/include/gtest/gtest.h"
#include "base/logging.h"
#include "geometrical_acoustics/path_tracer.h"

namespace vraudio {

namespace {
// Given N bins each having some count, validates that the counts are almost the
// same across all bins.
void ValidateUniformDistribution(const std::vector<int>& count_in_bins,
                                 const int total_count) {
  const float expected_fraction =
      1.0f / static_cast<float>(count_in_bins.size());
  const float tolerance = static_cast<float>(count_in_bins.size()) /
                          static_cast<float>(total_count);
  for (const int count : count_in_bins) {
    const float actual_fraction =
        static_cast<float>(count) / static_cast<float>(total_count);
    const float error = actual_fraction - expected_fraction;
    const float chi_square = error * error / expected_fraction;
    EXPECT_NEAR(chi_square, 0.0f, tolerance);
  }
}

// Assigns surface materials to groups of triangles, one material to one group.
void AssignSurfaceMaterialsToTriangleGroups(
    const std::vector<std::unordered_set<unsigned int>>& triangle_groups,
    const std::vector<MaterialName>& materials,
    const std::function<float()>& random_number_generator,
    SceneManager* scene_manager) {
  CHECK_EQ(triangle_groups.size(), materials.size());

  const float scattering_coefficient = 1.0f;
  std::array<float, kNumReverbOctaveBands> absorption_coefficients;
  for (size_t group = 0; group < materials.size(); ++group) {
    const size_t material_index = static_cast<size_t>(materials[group]);
    const RoomMaterial& room_material = GetRoomMaterial(material_index);
    for (size_t band = 0; band < kNumReverbOctaveBands; ++band) {
      absorption_coefficients[band] =
          room_material.absorption_coefficients[band];
    }
    ReflectionKernel reflection_kernel(absorption_coefficients,
                                       scattering_coefficient,
                                       random_number_generator);
    scene_manager->AssociateReflectionKernelToTriangles(reflection_kernel,
                                                        triangle_groups[group]);
  }
}

}  // namespace

void ValidateDistribution(const int num_samples, const int num_bins,
                          std::function<float()> cdf) {
  DCHECK_GT(num_bins, 0);
  std::vector<int> cdf_bins(num_bins, 0);
  for (int i = 0; i < num_samples; ++i) {
    const float cdf_value = cdf();
    const int cdf_bin_index = std::min(
        num_bins - 1, std::max(0, static_cast<int>(cdf_value * num_bins)));
    ++cdf_bins[cdf_bin_index];
  }
  ValidateUniformDistribution(cdf_bins, num_samples);
}

void AddTestGround(RTCScene scene) {
  unsigned int mesh_id = rtcNewTriangleMesh(scene, RTC_GEOMETRY_STATIC, 2, 4);

  // Vertices. Each vertex has 4 floats: x, y, z, and a padding float whose
  // value we do not care.
  float ground_vertices[] = {
      0.0f, 0.0f, 0.0f, 0.0f,  // Vertex_0.
      0.0f, 1.0f, 0.0f, 0.0f,  // Vertex_1.
      1.0f, 0.0f, 0.0f, 0.0f,  // Vertex_2.
      1.0f, 1.0f, 0.0f, 0.0f,  // Vertex_3.
  };

  // Triangles. Somehow Embree is a left-handed system.
  int ground_indices[] = {
      0, 1, 2,  // Triangle_0.
      1, 3, 2,  // Triangle_1.
  };
  float* const embree_vertices =
      static_cast<float*>(rtcMapBuffer(scene, mesh_id, RTC_VERTEX_BUFFER));
  std::copy(ground_vertices, ground_vertices + 16, embree_vertices);
  rtcUnmapBuffer(scene, mesh_id, RTC_VERTEX_BUFFER);
  int* const embree_indices =
      static_cast<int*>(rtcMapBuffer(scene, mesh_id, RTC_INDEX_BUFFER));
  std::copy(ground_indices, ground_indices + 6, embree_indices);
  rtcUnmapBuffer(scene, mesh_id, RTC_INDEX_BUFFER);
}

void BuildTestBoxScene(
    const Vertex& min_corner, const Vertex& max_corner,
    std::vector<Vertex>* box_vertices, std::vector<Triangle>* box_triangles,
    std::vector<std::unordered_set<unsigned int>>* box_wall_triangles) {
  const float min_x = min_corner.x;
  const float min_y = min_corner.y;
  const float min_z = min_corner.z;
  const float max_x = max_corner.x;
  const float max_y = max_corner.y;
  const float max_z = max_corner.z;
  CHECK_LT(min_x, max_x);
  CHECK_LT(min_y, max_y);
  CHECK_LT(min_z, max_z);

  if (box_vertices != nullptr) {
    *box_vertices = std::vector<Vertex>{
        {min_x, min_y, min_z}, {min_x, max_y, min_z}, {max_x, min_y, min_z},
        {max_x, max_y, min_z}, {min_x, min_y, max_z}, {min_x, max_y, max_z},
        {max_x, min_y, max_z}, {max_x, max_y, max_z}};
  }
  if (box_vertices != nullptr) {
    *box_triangles = std::vector<Triangle>{
        {0, 1, 4}, {1, 5, 4},  // wall facing the -x direction.
        {2, 6, 3}, {3, 6, 7},  // wall facing the +x direction.
        {0, 6, 2}, {0, 4, 6},  // wall facing the -y direction.
        {1, 3, 7}, {1, 7, 5},  // wall facing the +y direction.
        {0, 3, 1}, {0, 2, 3},  // wall facing the -z direction.
        {4, 7, 6}, {4, 5, 7},  // wall facing the +z direction.
    };
  }
  if (box_wall_triangles != nullptr) {
    *box_wall_triangles = std::vector<std::unordered_set<unsigned int>>{
        {0, 1}, {2, 3}, {4, 5}, {6, 7}, {8, 9}, {10, 11}};
  }
}

std::vector<Path> TracePathsInTestcene(
    size_t min_num_rays, size_t max_depth, float energy_threshold,
    const float source_position[3], const std::vector<Vertex>& scene_vertices,
    const std::vector<Triangle>& scene_triangles,
    const std::vector<std::unordered_set<unsigned int>>& scene_triangle_groups,
    const std::vector<MaterialName>& materials, SceneManager* scene_manager) {
  // Build the scene.
  scene_manager->BuildScene(scene_vertices, scene_triangles);

  // Assign materials.
  std::default_random_engine engine(0);
  std::uniform_real_distribution<float> distribution(0.0f, 1.0f);
  std::function<float()> random_number_generator = [&engine, &distribution] {
    return distribution(engine);
  };
  AssignSurfaceMaterialsToTriangleGroups(
      scene_triangle_groups, materials, random_number_generator, scene_manager);

  // Trace paths.
  AcousticSource source(Eigen::Vector3f(source_position), kUnitEnergies,
                        random_number_generator);
  PathTracer path_tracer(*scene_manager);
  return path_tracer.TracePaths(source, min_num_rays, max_depth,
                                energy_threshold);
}

std::vector<Path> GenerateUniformlyDistributedRayPaths(
    const float source_position[3], size_t min_num_rays) {
  // Use AcousticSource to generate rays in uniformly distributed directions.
  std::default_random_engine engine(0);
  std::uniform_real_distribution<float> distribution(0.0f, 1.0f);
  AcousticSource source(
      Eigen::Vector3f(source_position), kUnitEnergies,
      [&engine, &distribution] { return distribution(engine); });

  const size_t sqrt_num_rays = static_cast<size_t>(
      std::ceil(std::sqrt(static_cast<float>(min_num_rays))));
  const size_t num_rays = sqrt_num_rays * sqrt_num_rays;
  std::vector<Path> paths(num_rays);
  const std::vector<AcousticRay>& rays =
      source.GenerateStratifiedRays(num_rays, sqrt_num_rays);
  for (size_t i = 0; i < num_rays; ++i) {
    Path path;
    path.rays.push_back(rays[i]);
    paths[i] = path;
  }

  return paths;
}

void ExpectFloat3Close(const float actual_vector[3],
                       const float expected_vector[3]) {
  EXPECT_FLOAT_EQ(actual_vector[0], expected_vector[0]);
  EXPECT_FLOAT_EQ(actual_vector[1], expected_vector[1]);
  EXPECT_FLOAT_EQ(actual_vector[2], expected_vector[2]);
}

void ExpectFloat3Close(const float actual_vector[3],
                       const float expected_vector[3], float tolerance) {
  EXPECT_NEAR(actual_vector[0], expected_vector[0], tolerance);
  EXPECT_NEAR(actual_vector[1], expected_vector[1], tolerance);
  EXPECT_NEAR(actual_vector[2], expected_vector[2], tolerance);
}

void ValidateSparseFloatArray(const std::vector<float>& actual_array,
                              const std::vector<size_t>& expected_indices,
                              const std::vector<float>& expected_values,
                              float relative_error_tolerance) {
  // First construct the expected array.
  std::vector<float> expected_array(actual_array.size(), 0.0f);
  for (size_t i = 0; i < expected_indices.size(); ++i) {
    expected_array[expected_indices[i]] = expected_values[i];
  }

  // Compare with the actual array element-by-element.
  for (size_t i = 0; i < actual_array.size(); ++i) {
    EXPECT_NEAR(actual_array[i], expected_array[i],
                expected_array[i] * relative_error_tolerance);
  }
}

}  // namespace vraudio
