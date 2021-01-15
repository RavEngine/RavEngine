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

// Utilities useful for testing Geometrical Acoustics related classes.
#ifndef RESONANCE_AUDIO_GEOMETRICAL_ACOUSTICS_TEST_UTIL_H_
#define RESONANCE_AUDIO_GEOMETRICAL_ACOUSTICS_TEST_UTIL_H_

#include <cstddef>
#include <functional>
#include <unordered_set>
#include <vector>

#include "embree2/rtcore.h"
#include "api/resonance_audio_api.h"
#include "base/constants_and_types.h"
#include "geometrical_acoustics/mesh.h"
#include "geometrical_acoustics/path.h"
#include "geometrical_acoustics/scene_manager.h"
#include "platforms/common/room_effects_utils.h"

namespace vraudio {

// Array of |kNumReverbOctaveBands| of unit energies. Useful to set as initial
// energies of sound sources in tests.
static const std::array<float, kNumReverbOctaveBands> kUnitEnergies{
    {1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f}};

// Validates a distribution given the cumulative distribution function (CDF).
// For any probability density function (PDF), its CDF should be uniformly
// distributed. So we generate some samples, gather them into bins based on
// their CDF values, and verify that the number of samples are almost the
// same across all bins.
//
// @param num_samples Number of samples to generate.
// @param num_bins Number of bins to gather the samples.
// @param cdf Cumulative distribution function of the distribution to test.
void ValidateDistribution(const int num_samples, const int num_bins,
                          std::function<float()> cdf);

// Adds a ground (a rectangle consisting of two triangles) to a test scene.
//
// @param scene Test scene to which the ground is added.
void AddTestGround(RTCScene scene);

// Builds a box scene with 8 vertices, 12 triangles, and 6 walls.
//
// @param min_corner Corner of the box with minimum x-, y-, z-components.
// @param max_corner Corner of the box with maximum x-, y-, z-components.
// @param box_vertices Output vertices of the box. Not filled if nullptr is
//     passed in.
// @param box_triangles Output triangles of the box. Not filled if nullptr is
//     passed in.
// @param box_wall_triangles Output wall-to-triangles mapping, with six walls
//     each having two triangles. Not filled if nullptr is passed in.
void BuildTestBoxScene(
    const Vertex& min_corner, const Vertex& max_corner,
    std::vector<Vertex>* box_vertices, std::vector<Triangle>* box_triangles,
    std::vector<std::unordered_set<unsigned int>>* box_wall_triangles);

// Traces ray paths in a created test scene.
//
// @param num_rays Number of rays.
// @param max_depth Maximum depth of tracing performed along a path.
// @param energy_threshold Energy threshold below which the tracing stops.
// @param source_position Position from which to shoot rays.
// @param scene_vertices Vertices of the scene.
// @param scene_triangles Triangles of the scene.
// @param scene_triangle_groups Groups of triangles that share the same
//     material.
// @param materials Materials for each triangle group.
// @param scene_manager Scene manager that holds the created test scene.
// @return Vector of traced paths.
std::vector<Path> TracePathsInTestcene(
    size_t num_rays, size_t max_depth, float energy_threshold,
    const float source_position[3], const std::vector<Vertex>& scene_vertices,
    const std::vector<Triangle>& scene_triangles,
    const std::vector<std::unordered_set<unsigned int>>& scene_triangle_groups,
    const std::vector<MaterialName>& materials, SceneManager* scene_manager);

// Generated ray paths in uniformly distributed directions.
//
// @param source_position Position from which to shoot rays.
// @param min_num_rays Minimum number of rays generated.
// @return Vector of traced paths.
std::vector<Path> GenerateUniformlyDistributedRayPaths(
    const float source_position[3], size_t min_num_rays);

// Compares two float vectors[3]s and expect that their components are close.
//
// @param actual_vector Actual vector.
// @param expected_vector Expected vector.
void ExpectFloat3Close(const float actual_vector[3],
                       const float expected_vector[3]);

// Same as above but with a user-specified tolerance.
//
// @param actual_vector Actual vector.
// @param expected_vector Expected vector.
// @param tolerance Tolerance for comparisons.
void ExpectFloat3Close(const float actual_vector[3],
                       const float expected_vector[3], float tolerance);

// Validates a sparse float array using the indices and values of its
// non-zero elements.
//
// @param actual_array Actual array to be validated.
// @param expected_indices Indices of the non-zero elements of the expected
//     array.
// @param expected_values Values of the non-zero elements of the expected array.
// @param relative_error_tolerance Tolerance of relative errors for element
//     comparisons.
void ValidateSparseFloatArray(const std::vector<float>& actual_array,
                              const std::vector<size_t>& expected_indices,
                              const std::vector<float>& expected_values,
                              float relative_error_tolerance);
}  // namespace vraudio

#endif  // RESONANCE_AUDIO_GEOMETRICAL_ACOUSTICS_TEST_UTIL_H_
