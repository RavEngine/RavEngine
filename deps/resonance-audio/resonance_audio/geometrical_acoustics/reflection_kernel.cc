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

#include "geometrical_acoustics/reflection_kernel.h"

#include "base/logging.h"
#include "geometrical_acoustics/sampling.h"

namespace vraudio {

using Eigen::Vector3f;

AcousticRay ReflectionKernel::Reflect(const AcousticRay& incident_ray) const {
  // This function currently uses |random_number_generator| (which returns
  // uniformly distributed numbers) to naively sample 1D and 2D points.

  AcousticRay::RayType reflected_ray_type =
      (random_number_generator_() >= scattering_coefficient_)
          ? AcousticRay::RayType::kSpecular
          : AcousticRay::RayType::kDiffuse;

  // Compute the reflected direction.
  Vector3f reflected_direction;
  const Vector3f& unit_normal =
      Vector3f(incident_ray.intersected_geometry_normal()).normalized();
  const Vector3f& incident_direction = Vector3f(incident_ray.direction());
  switch (reflected_ray_type) {
    case AcousticRay::RayType::kSpecular:
      reflected_direction =
          incident_direction -
          2.0f * incident_direction.dot(unit_normal) * unit_normal;
      break;
    case AcousticRay::RayType::kDiffuse:
      // A Lambertian reflection.
      reflected_direction = CosineSampleHemisphere(
          random_number_generator_(), random_number_generator_(), unit_normal);
      break;
  }
  const Vector3f reflected_origin = Vector3f(incident_ray.origin()) +
                                    incident_ray.t_far() * incident_direction;
  const float reflected_ray_prior_distance =
      incident_ray.prior_distance() +
      incident_ray.t_far() * incident_direction.norm();

  // New energies for each frequency band.
  CHECK_EQ(reflection_coefficients_.size(), incident_ray.energies().size());
  std::array<float, kNumReverbOctaveBands> new_energies =
      incident_ray.energies();

  for (size_t i = 0; i < new_energies.size(); ++i) {
    new_energies[i] *= reflection_coefficients_[i];
  }

  return AcousticRay(reflected_origin.data(), reflected_direction.data(),
                     AcousticRay::kRayEpsilon, AcousticRay::kInfinity,
                     new_energies, reflected_ray_type,
                     reflected_ray_prior_distance);
}

void ReflectionKernel::ReflectDiffuseRain(
    const AcousticRay& incident_ray, const AcousticRay& reference_reflected_ray,
    const Eigen::Vector3f& listener_position, float* direction_pdf,
    AcousticRay* diffuse_rain_ray) const {
  const Vector3f reflection_point_to_listener =
      listener_position - Vector3f(reference_reflected_ray.origin());
  const Vector3f reflection_point_to_listener_direction =
      reflection_point_to_listener.normalized();
  const float diffuse_rain_ray_t_far =
      reflection_point_to_listener.norm() + reference_reflected_ray.t_near();

  *direction_pdf = CosineSampleHemispherePdf(
      Vector3f(incident_ray.intersected_geometry_normal()).normalized(),
      reflection_point_to_listener_direction);

  diffuse_rain_ray->set_origin(reference_reflected_ray.origin());
  diffuse_rain_ray->set_direction(
      reflection_point_to_listener_direction.data());
  diffuse_rain_ray->set_t_near(reference_reflected_ray.t_near());
  diffuse_rain_ray->set_t_far(diffuse_rain_ray_t_far);
  diffuse_rain_ray->set_energies(reference_reflected_ray.energies());
  diffuse_rain_ray->set_type(reference_reflected_ray.type());
  diffuse_rain_ray->set_prior_distance(
      reference_reflected_ray.prior_distance());
}

}  // namespace vraudio
