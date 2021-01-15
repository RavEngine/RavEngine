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

#include "geometrical_acoustics/impulse_response_computer.h"

#include <utility>

#include "Eigen/Core"
#include "base/logging.h"
#include "geometrical_acoustics/parallel_for.h"
#include "geometrical_acoustics/reflection_kernel.h"

namespace vraudio {

using Eigen::Vector3f;

ImpulseResponseComputer::ImpulseResponseComputer(
    float listener_sphere_radius, float sampling_rate,
    std::unique_ptr<std::vector<AcousticListener>> listeners,
    SceneManager* scene_manager)
    : listeners_(std::move(listeners)),
      collection_kernel_(listener_sphere_radius, sampling_rate),
      scene_manager_(scene_manager),
      finalized_(false),
      num_total_paths_(0) {
  CHECK_NOTNULL(listeners_.get());
  scene_manager->BuildListenerScene(*listeners_, listener_sphere_radius);
}

ImpulseResponseComputer::~ImpulseResponseComputer() {}

void ImpulseResponseComputer::CollectContributions(
    const std::vector<Path>& paths_batch) {
  // Do not collect anymore if finalized.
  if (finalized_) {
    return;
  }

  CHECK(scene_manager_->is_scene_committed());
  CHECK(scene_manager_->is_listener_scene_committed());

  const unsigned int num_threads = GetNumberOfHardwareThreads();
  ParallelFor(
      num_threads, paths_batch.size(), [&paths_batch, this](size_t path_index) {
        const Path& path = paths_batch.at(path_index);
        const AcousticRay* previous_ray = nullptr;
        AcousticRay diffuse_rain_ray;
        for (const AcousticRay& ray : path.rays) {
          // Handle diffuse reflections separately using the diffuse rain
          // algorithm. For details, see "A Fast Reverberation Estimator for
          // Virtual Environments" by Schroder et al., 2007.
          if (ray.type() == AcousticRay::RayType::kDiffuse &&
              previous_ray != nullptr) {
            // Connect each listener from the reflection point.
            for (AcousticListener& listener : *listeners_) {
              const ReflectionKernel& reflection =
                  scene_manager_->GetAssociatedReflectionKernel(
                      previous_ray->intersected_primitive_id());

              float reflection_pdf = 0.0f;
              reflection.ReflectDiffuseRain(*previous_ray, ray,
                                            listener.position, &reflection_pdf,
                                            &diffuse_rain_ray);

              if (!diffuse_rain_ray.Intersect(scene_manager_->scene())) {
                // Get PDF from the reflection kernel that the previous ray
                // intersects.
                collection_kernel_.CollectDiffuseRain(
                    diffuse_rain_ray, 1.0f /* weight */, reflection_pdf,
                    &listener);
              }
            }

            previous_ray = &ray;
            continue;
          }

          // |ray| may intersect multiple listener spheres. We handle the
          // intersections one-by-one as following:
          // 1. Find the first intersected sphere S. The ray's data will be
          //    modified so that ray.t_far() corresponds to the intersection
          //    point. Terminate if no intersection is found.
          // 2. Collect contribution to the listener associated to S.
          // 3. Spawn a sub-ray that starts at the intersection point (moved
          //    slightly inside S) and repeat 1.
          //
          // Since the origin of the new sub-ray is inside sphere S, it does not
          // intersect with S again (according to our definition of
          // intersections; see Sphere::SphereIntersection()).
          //
          // Each of the sub-rays has the same origin and direction as the
          // original ray, but with t_near and t_far partitioning the interval
          // between the original ray's t_near and t_far. For example:
          //
          // ray:       t_near = 0.0 ------------------------------> t_far = 5.0
          // sub_ray_1: t_near = 0.0 -------> t_far = 2.0
          // sub_ray_2:                      t_near = 2.0 ---------> t_far = 5.0
          AcousticRay sub_ray(ray.origin(), ray.direction(), ray.t_near(),
                              ray.t_far(), ray.energies(), ray.type(),
                              ray.prior_distance());

          // Norm of |ray.direction|. Useful in computing
          // |sub_ray.prior_distance| later.
          const float ray_direction_norm = Vector3f(ray.direction()).norm();

          // In theory with sufficient AcousticRay::kRayEpsilon, the same sphere
          // should not be intersected twice by the same ray segment. In
          // practice, due to floating-point inaccuracy, this might still
          // happen. We explicitly prevent double-counting by checking whether
          // the intersected sphere id has changed.
          unsigned int previous_intersected_sphere_id = RTC_INVALID_GEOMETRY_ID;

          // To prevent an infinite loop, terminate if one ray segment has more
          // intersections than the number of listeners, which is an upper bound
          // of the number of actually contributing sub-rays.
          size_t num_intersections = 0;
          while (sub_ray.Intersect(scene_manager_->listener_scene()) &&
                 num_intersections < listeners_->size()) {
            const unsigned int sphere_id = sub_ray.intersected_geometry_id();
            if (sphere_id != previous_intersected_sphere_id) {
              AcousticListener& listener = listeners_->at(
                  scene_manager_->GetListenerIndexFromSphereId(sphere_id));
              collection_kernel_.Collect(sub_ray, 1.0f /* weight */, &listener);
            } else {
              LOG(WARNING) << "Double intersection with sphere[" << sphere_id
                           << "]; contribution skipped. Consider increasing "
                           << "AcousticRay::kRayEpsilon";
            }
            previous_intersected_sphere_id = sphere_id;

            // Spawn a new sub-ray whose t_near corresponds to the intersection
            // point. The new sub-ray's |prior_distance| field is extended by
            // the distance traveled by the old sub-ray up to the intersection
            // point.
            const float new_prior_distance =
                sub_ray.prior_distance() +
                (sub_ray.t_far() - sub_ray.t_near()) * ray_direction_norm;
            sub_ray = AcousticRay(ray.origin(), ray.direction(),
                                  sub_ray.t_far() + AcousticRay::kRayEpsilon,
                                  ray.t_far(), ray.energies(), ray.type(),
                                  new_prior_distance);
            ++num_intersections;
          }

          previous_ray = &ray;
        }
      });
  num_total_paths_ += paths_batch.size();
}

const std::vector<AcousticListener>&
ImpulseResponseComputer::GetFinalizedListeners() {
  DCHECK_GT(num_total_paths_, 0U);

  if (!finalized_) {
    // For a Monte Carlo method that estimates a value with N samples,
    // <estimated value> = 1/N * sum(<value of a sample>). We apply the
    // 1/N weight after all impulse responses for all listeners are collected.
    const float monte_carlo_weight =
        1.0f / static_cast<float>(num_total_paths_);
    for (AcousticListener& listener : *listeners_) {
      for (std::vector<float>& responses_in_band :
           listener.energy_impulse_responses) {
        for (float& response : responses_in_band) {
          response *= monte_carlo_weight;
        }
      }
    }
    finalized_ = true;
  }

  return *listeners_;
}

}  // namespace vraudio
