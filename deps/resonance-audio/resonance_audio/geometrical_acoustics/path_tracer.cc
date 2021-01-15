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

#include "geometrical_acoustics/path_tracer.h"

#include <cmath>

#include "base/logging.h"
#include "geometrical_acoustics/parallel_for.h"
#include "geometrical_acoustics/reflection_kernel.h"

namespace vraudio {

std::vector<Path> PathTracer::TracePaths(const AcousticSource& source,
                                         size_t min_num_rays, size_t max_depth,
                                         float energy_threshold) {
  // The tracing would not work if the scene is not committed.
  CHECK(scene_manager_.is_scene_committed());

  // Find the actual number of rays to trace as the next square number greater
  // or equal to |min_num_rays|.
  const size_t sqrt_num_rays = static_cast<size_t>(
      std::ceil(std::sqrt(static_cast<float>(min_num_rays))));
  const size_t num_rays = sqrt_num_rays * sqrt_num_rays;

  // In the current implementation, one ray does not spawn more than one child
  // ray, hence the number of paths is the same as the number of rays from the
  // source.
  std::vector<Path> paths(num_rays);
  std::vector<AcousticRay> rays_from_source =
      source.GenerateStratifiedRays(num_rays, sqrt_num_rays);
  const unsigned int num_threads = GetNumberOfHardwareThreads();
  ParallelFor(
      num_threads, num_rays,
      [&rays_from_source, &paths, this, max_depth,
       energy_threshold](size_t ray_index) {
        if (max_depth == 0) return;
        Path& path = paths.at(ray_index);

        // Pre-allocate memory space for better performance.
        path.rays.reserve(max_depth);

        path.rays.push_back(rays_from_source[ray_index]);
        size_t depth = 0;
        while (true) {
          AcousticRay& current_ray = path.rays.back();

          // Stop generating new rays if the current ray escapes.
          if (!current_ray.Intersect(scene_manager_.scene())) {
            break;
          }

          // Stop generating new rays if |depth| reaches |max_depth|.
          ++depth;
          if (depth >= max_depth) {
            break;
          }

          // Handle interactions with scene geometries.
          const ReflectionKernel& reflection =
              scene_manager_.GetAssociatedReflectionKernel(
                  current_ray.intersected_primitive_id());
          AcousticRay new_ray = reflection.Reflect(current_ray);

          // Stop tracing if all energies in all frequency bands of the new ray
          // are too low in energy.
          bool is_energy_high_enough = false;
          for (const float energy : new_ray.energies()) {
            if (energy >= energy_threshold) {
              is_energy_high_enough = true;
              break;
            }
          }
          if (!is_energy_high_enough) {
            break;
          }
          path.rays.push_back(new_ray);
        }
      });
  return paths;
}

}  // namespace vraudio
