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

#ifndef RESONANCE_AUDIO_GEOMETRICAL_ACOUSTICS_SPHERE_H_
#define RESONANCE_AUDIO_GEOMETRICAL_ACOUSTICS_SPHERE_H_

#include "embree2/rtcore.h"
#include "embree2/rtcore_ray.h"

namespace vraudio {

// A user defined sphere geometry that enables Embree to test ray intersections.
struct RTCORE_ALIGN(16) Sphere {
  // Center of the sphere.
  float center[3];

  // Radius of the sphere.
  float radius;

  // Geometry Id. This will be reported by ray intersections.
  unsigned int geometry_id;
};

// The following functions are to be called by Embree internally to enable fast
// ray-sphere intersections tests.

// Calculates the RTCBounds (essentially a bounding  box) of a given sphere.
//
// @param sphere Sphere of interest.
// @param output_bounds Output RTCBounds of the sphere.
inline void SphereBounds(const Sphere& sphere, RTCBounds* output_bounds) {
  output_bounds->lower_x = sphere.center[0] - sphere.radius;
  output_bounds->lower_y = sphere.center[1] - sphere.radius;
  output_bounds->lower_z = sphere.center[2] - sphere.radius;
  output_bounds->upper_x = sphere.center[0] + sphere.radius;
  output_bounds->upper_y = sphere.center[1] + sphere.radius;
  output_bounds->upper_z = sphere.center[2] + sphere.radius;
}

// Tests whether a sphere and a ray intersects and sets the related data for
// the ray when an intersection is found:
// - |ray.tfar| will be set to corresponding to the first intersection point.
// - |ray.geomID| will be set to the geometry id of the sphere.
// - |ray.Ng| will be set to the normal at the first intersection point,
//   pointing radially outward
//
// Because this function will be called internally by Embree, it works on
// Embree's native type RTCRay as opposed to AcousticRay.
//
// @param spheres Sphere of interest.
// @param ray RTCRay with which intersections are tested and related data set.
void SphereIntersection(const Sphere& sphere, RTCRay* ray);

}  // namespace vraudio

#endif  // RESONANCE_AUDIO_GEOMETRICAL_ACOUSTICS_SPHERE_H_
