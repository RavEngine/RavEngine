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

#include "geometrical_acoustics/collection_kernel.h"

#include <array>
#include <cmath>
#include <vector>

#include "Eigen/Core"
#include "base/constants_and_types.h"
#include "base/logging.h"

namespace vraudio {

using Eigen::Vector3f;

namespace {

// Adds a response to the output |energy_impulse_responses| array at an index
// computed based on |total_source_listener_distance| and
// |distance_to_impulse_response_index|. The values to be added are |energies|
// multiplied by |energy_factor|.
void AddResponse(float total_source_listener_distance,
                 float distance_to_impulse_response_index, float energy_factor,
                 const std::array<float, kNumReverbOctaveBands>& energies,
                 std::array<std::vector<float>, kNumReverbOctaveBands>*
                     energy_impulse_responses) {
  const size_t impulse_response_index = static_cast<size_t>(
      total_source_listener_distance * distance_to_impulse_response_index);

  // It is OK if |impulse_response_index| exceeds the size of the listener's
  // impulse response array (e.g. if the user only cares about the first second
  // of the response), in which case we simply discard the contribution.
  if (impulse_response_index >= (*energy_impulse_responses)[0].size()) {
    return;
  }

  for (size_t i = 0; i < energy_impulse_responses->size(); ++i) {
    (*energy_impulse_responses)[i][impulse_response_index] +=
        energy_factor * energies.at(i);
  }
}

}  // namespace

// In our implementation, |sphere_size_energy_factor_| is defined such that a
// listener 1.0 meter away from the source would give an attenuation of 1.0.
// Therefore the value is computed by:
//   4.0 * PI * 1.0^2 / (PI * R^2) = 4.0 / R^2.
CollectionKernel::CollectionKernel(float listener_sphere_radius,
                                   float sampling_rate)
    : sphere_size_energy_factor_(4.0f / listener_sphere_radius /
                                 listener_sphere_radius),
      distance_to_impulse_response_index_(sampling_rate / kSpeedOfSound) {}

void CollectionKernel::Collect(const AcousticRay& ray, float weight,
                               AcousticListener* listener) const {
  CHECK_EQ(ray.energies().size(), listener->energy_impulse_responses.size());

  // Collect the energy contribution to the listener's impulse response at the
  // arrival time.
  // The distance to listener on this ray is approximated by projecting
  // (listener.position - sub_ray's starting point) onto the ray direction.
  const Vector3f ray_direction(ray.direction());
  const Vector3f ray_starting_point =
      Vector3f(ray.origin()) + ray.t_near() * ray_direction;
  const float distance_to_listener_on_ray =
      (listener->position - ray_starting_point).dot(ray_direction.normalized());
  AddResponse(ray.prior_distance() + distance_to_listener_on_ray,
              distance_to_impulse_response_index_,
              weight * sphere_size_energy_factor_, ray.energies(),
              &listener->energy_impulse_responses);
}

// In a diffuse rain algorithm, instead of relying on the Monte Carlo process
// to estimate expected energy gathered by the sphere, we directly multiply
// the probability of a ray intersecting the sphere to the energy to be
// collected, thus ensuring the expected gathered energies are the same (see
// also [internal ref]
//
//   <Energy by Monte Carlo process> = <Energy by diffuse rain>
//   sum_i (Prob[ray_i intersects sphere] * Energy_i) =
//       sum_i (factor_i * Energy_i)
//
//   So factor_i = Prob[ray_i intersects sphere]
//               ~ PDF(ray_i in the direction pointing to the listener) *
//                    (projected solid angle of the listener sphere)
//               ~ PDF * PI * R^2 / (4.0 * PI * D^2)
//               = PDF * R^2 / (4.0 * D^2),
//
//   where PDF is the probability density function, R the radius of the
//   listener sphere, and D the distance between the listener and the
//   reflection point.
//
// Combining |factor_i| with |sphere_size_energy_factor_| = 4.0 / R^2,
// the total energy factor that needs to be multiplied to the energies on a
// diffuse-rain ray is therefore (PDF / D^2).
void CollectionKernel::CollectDiffuseRain(const AcousticRay& diffuse_rain_ray,
                                          float weight, float direction_pdf,
                                          AcousticListener* listener) const {
  // Since a diffuse-rain ray already connects to the listener, and its
  // direction already normalized, the distance to listener is its
  // t_far - t_near.
  const float distance_to_listener_on_ray =
      diffuse_rain_ray.t_far() - diffuse_rain_ray.t_near();
  const float diffuse_rain_energy_factor =
      direction_pdf / distance_to_listener_on_ray / distance_to_listener_on_ray;
  AddResponse(diffuse_rain_ray.prior_distance() + distance_to_listener_on_ray,
              distance_to_impulse_response_index_,
              weight * diffuse_rain_energy_factor, diffuse_rain_ray.energies(),
              &listener->energy_impulse_responses);
}

}  // namespace vraudio
