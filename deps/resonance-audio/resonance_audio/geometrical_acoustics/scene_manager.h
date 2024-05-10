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

#ifndef RESONANCE_AUDIO_GEOMETRICAL_ACOUSTICS_SCENE_MANAGER_H_
#define RESONANCE_AUDIO_GEOMETRICAL_ACOUSTICS_SCENE_MANAGER_H_

#include <functional>
#include <unordered_map>
#include <unordered_set>
#include <vector>

#include "embree2/rtcore.h"
#include "embree2/rtcore_ray.h"
#include "base/logging.h"
#include "geometrical_acoustics/acoustic_listener.h"
#include "geometrical_acoustics/mesh.h"
#include "geometrical_acoustics/reflection_kernel.h"

namespace vraudio {

// A class to manage a scene modeling an acoustic environment and a scene
// modeling listeners.
class SceneManager {
 public:
  SceneManager();
  virtual ~SceneManager();

  // Builds a scene with triangles describing the acoustic environment.
  //
  // @param vertex_buffer  A vector of vertices.
  // @param triangle_buffer A vector of triangles. The triangles should be
  //     right-handed, i.e. if a triangle is defined by {v0, v1, v2},
  //     then its normal is along the right-handed cross product of (v1 - v0)
  //     and (v2 - v0).
  void BuildScene(const std::vector<Vertex>& vertex_buffer,
                  const std::vector<Triangle>& triangle_buffer);

  // Associates a reflection kernel to a set of triangles.
  //
  // @param reflection Reflection kernel.
  // @param triangle_indices Indices of triangles to be associated with
  //     the reflection .
  // @return True on success.
  bool AssociateReflectionKernelToTriangles(
      const ReflectionKernel& reflection_kernel,
      const std::unordered_set<unsigned int>& triangle_indices);

  // Gets the reflection kernel associated to a triangle.
  //
  // @param triangle_index The index of the triangle to get the reflection
  //     kernel from.
  // @return The reflection kernel associated to the triangle; a default
  //     reflection if no reflection kernel is associated to the triangle.
  const ReflectionKernel& GetAssociatedReflectionKernel(
      unsigned int triangle_index) const {
    if (triangle_to_reflection_map_.count(triangle_index) == 0) {
      return kDefaultReflection;
    }
    return reflections_.at(triangle_to_reflection_map_.at(triangle_index));
  }

  RTCScene scene() const { return scene_; }
  bool is_scene_committed() const { return is_scene_committed_; }
  size_t num_vertices() const { return num_vertices_; }
  size_t num_triangles() const { return num_triangles_; }

  // Builds a scene with only listener spheres, one sphere per listener.
  //
  // @param listeners Vector of AcousticListeners.
  // @param listener_sphere_radius Radius of listener spheres (m).
  void BuildListenerScene(const std::vector<AcousticListener>& listeners,
                          float listener_sphere_radius);

  // Gets the listener index associated to a sphere id.
  //
  // @param sphere_id Sphere id.
  // @return listener_index Listener index associated to the sphere id.
  size_t GetListenerIndexFromSphereId(unsigned int sphere_id) {
    DCHECK_GT(sphere_to_listener_map_.count(sphere_id), 0);
    return sphere_to_listener_map_.at(sphere_id);
  }

  RTCScene listener_scene() const { return listener_scene_; }
  bool is_listener_scene_committed() const {
    return is_listener_scene_committed_;
  }

 private:
  // Perfect absorption. No randomness needed.
  const std::array<float, kNumReverbOctaveBands>
      kPerfectReflectionCoefficients = {
          {1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f}};
  const ReflectionKernel kDefaultReflection = ReflectionKernel(
      kPerfectReflectionCoefficients, 0.0f, []() { return 0.5f; });

  // Embree device.
  RTCDevice device_;

  // Scene modeling an acoustic environment.
  RTCScene scene_;
  bool is_scene_committed_ = false;
  size_t num_vertices_ = 0;
  size_t num_triangles_ = 0;
  std::vector<ReflectionKernel> reflections_;
  std::unordered_map<unsigned int, size_t> triangle_to_reflection_map_;

  // Scene with only listener spheres.
  RTCScene listener_scene_;
  bool is_listener_scene_committed_ = false;

  // Map from a sphere's id to the index of a listener.
  std::unordered_map<unsigned int, size_t> sphere_to_listener_map_;
};

}  // namespace vraudio

#endif  // RESONANCE_AUDIO_GEOMETRICAL_ACOUSTICS_SCENE_MANAGER_H_
