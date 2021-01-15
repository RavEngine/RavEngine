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

#include "geometrical_acoustics/sphere.h"

#include <cmath>
#include <utility>

#include "Eigen/Core"

namespace vraudio {

using Eigen::Vector3f;

namespace {
// Fills intersection data in an RTCRay.
//
// @param sphere Sphere that intersects with the ray.
// @param t0 Ray parameter corresponding to the first intersection point.
// @param t1 Ray parameter corresponding to the second intersection point.
// @param ray RTCRay that potentially intersects with |sphere|, whose data
//     fields are to be filled.
inline void FillRaySphereIntersectionData(const Sphere& sphere, float t0,
                                          float t1, RTCRay* ray) {
  // For convenience we enforce that t0 <= t1.
  if (t0 > t1) {
    std::swap(t0, t1);
  }

  // In our application we only consider "intersecting" if the ray starts and
  // ends outside the sphere, i.e. only if ray.tnear < t0 and ray.tfar > t1.
  if ((ray->tnear >= t0) || (ray->tfar <= t1)) {
    return;
  }

  // Set intersection-related data.
  ray->tfar = t0;
  ray->geomID = sphere.geometry_id;

  // ray.Ng is the normal at the intersection point. For a sphere the normal is
  // always pointing radially outward, i.e. normal = p - sphere.center, where
  // p is the intersection point. Of the two intersection points, We use the
  // first.
  ray->Ng[0] = ray->org[0] + t0 * ray->dir[0] - sphere.center[0];
  ray->Ng[1] = ray->org[1] + t0 * ray->dir[1] - sphere.center[1];
  ray->Ng[2] = ray->org[2] + t0 * ray->dir[2] - sphere.center[2];
}

}  // namespace

void SphereIntersection(const Sphere& sphere, RTCRay* ray) {
  // The intersection is tested by finding if there exists a point p that is
  // both on the ray and on the sphere:
  // - Point on the ray: p = ray.origin + t * ray.direction, where t is a
  //   parameter.
  // - Point on the sphere: || p - sphere.center || = sphere.radius.
  //
  // Solving the above two equations for t leads to solving a quadratic
  // equation:
  //     at^2 + bt + c = 0,
  // where
  //     a = || ray.direction, ray.direction ||^2
  //     b = 2 * Dot(ray.direction, ray.origin - sphere.center)
  //     c = || ray.origin - sphere.center ||^2.
  // The two possible solutions to this quadratic equation are:
  //     t0 = - b - sqrt(b^2 - 4ac) / 2a
  //     t1 = - b + sqrt(b^2 - 4ac) / 2a.
  // The existence of real solutions can be tested if a discriminant value,
  // b^2 - 4ac, is greater than 0 (we treat discriminant == 0, which corresponds
  // to the ray touching the sphere at a single point, as not intersecting).

  // Vector pointing from the sphere's center to the ray's origin, i.e.
  // (ray.origin - sphere.center) in the above equations.
  const Vector3f center_ray_origin_vector(ray->org[0] - sphere.center[0],
                                          ray->org[1] - sphere.center[1],
                                          ray->org[2] - sphere.center[2]);
  const Vector3f ray_direction(ray->dir);
  const float a = ray_direction.squaredNorm();
  const float b = 2.0f * center_ray_origin_vector.dot(ray_direction);
  const float c =
      center_ray_origin_vector.squaredNorm() - sphere.radius * sphere.radius;
  const float discriminant = b * b - 4.0f * a * c;

  // No intersection; do nothing.
  if (discriminant <= 0.0f) {
    return;
  }

  // Solve for t0 and t1. As suggested in "Physically Based Rendering" by Pharr
  // and Humphreys, directly computing - b +- sqrt(b^2 - 4ac) / 2a gives poor
  // numeric precision when b is close to +- sqrt(b^2 - 4ac), and a more stable
  // form is used instead:
  //    t0 = q / a
  //    t1 = c / q,
  // where
  //    q = -(b - sqrt(b^2 - 4ac)) / 2 for b < 0
  //        -(b + sqrt(b^2 - 4ac)) / 2 otherwise.
  const float sqrt_discriminant = std::sqrt(discriminant);
  const float q = (b < 0.0f) ? -0.5f * (b - sqrt_discriminant)
                             : -0.5f * (b + sqrt_discriminant);
  const float t0 = q / a;
  const float t1 = c / q;

  FillRaySphereIntersectionData(sphere, t0, t1, ray);
}

}  // namespace vraudio
