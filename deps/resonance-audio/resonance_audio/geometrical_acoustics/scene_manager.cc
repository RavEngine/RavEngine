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

#include "geometrical_acoustics/scene_manager.h"

#include "base/aligned_allocator.h"
#include "geometrical_acoustics/sphere.h"

namespace vraudio {

namespace {

// A function adapter from SphereBounds() to RTCBoundsFunc in order to be
// passed to rtcSetBoundsFunction().
// The signature of RTCBoundsFunc does not comply with Google's C++ style,

static void EmbreeSphereBoundsFunction(void* user_data, size_t index,
                                       RTCBounds& output_bounds
) {
  Sphere* spheres = static_cast<Sphere*>(user_data);
  const Sphere& sphere = spheres[index];
  SphereBounds(sphere, &output_bounds);
}

// A function adapter from SphereIntersections() to RTCIntersectFunc in order
// to be passed to rtcSetIntersectFunction().
// The signature of RTCIntersectFunc does not comply with Google's C++ style,

static void EmbreeSphereIntersectFunction(void* user_data,
                                          RTCRay& ray,
                                          size_t index) {
  Sphere* const spheres = static_cast<Sphere*>(user_data);
  const Sphere& sphere = spheres[index];
  SphereIntersection(sphere, &ray);
}

}  // namespace

SceneManager::SceneManager() {
  // Use a single RTCDevice for all scenes.
  device_ = rtcNewDevice(nullptr);
  CHECK_NOTNULL(device_);
  scene_ = rtcDeviceNewScene(device_, RTC_SCENE_STATIC | RTC_SCENE_HIGH_QUALITY,
                             RTC_INTERSECT1);
  listener_scene_ = rtcDeviceNewScene(
      device_, RTC_SCENE_STATIC | RTC_SCENE_HIGH_QUALITY, RTC_INTERSECT1);
}

SceneManager::~SceneManager() {
  rtcDeleteScene(scene_);
  rtcDeleteScene(listener_scene_);
  rtcDeleteDevice(device_);
}

void SceneManager::BuildScene(const std::vector<Vertex>& vertex_buffer,
                              const std::vector<Triangle>& triangle_buffer) {
  num_vertices_ = vertex_buffer.size();
  num_triangles_ = triangle_buffer.size();
  unsigned int mesh_id = rtcNewTriangleMesh(scene_, RTC_GEOMETRY_STATIC,
                                            num_triangles_, num_vertices_);
  struct EmbreeVertex {
    // Embree uses 4 floats for each vertex for alignment. The last value
    // is for padding only.
    float x, y, z, a;
  };
  EmbreeVertex* const embree_vertex_array = static_cast<EmbreeVertex*>(
      rtcMapBuffer(scene_, mesh_id, RTC_VERTEX_BUFFER));
  for (size_t i = 0; i < num_vertices_; ++i) {
    embree_vertex_array[i].x = vertex_buffer[i].x;
    embree_vertex_array[i].y = vertex_buffer[i].y;
    embree_vertex_array[i].z = vertex_buffer[i].z;
  }
  rtcUnmapBuffer(scene_, mesh_id, RTC_VERTEX_BUFFER);

  // Triangles. Somehow Embree is a left-handed system, so we re-order all
  // triangle indices here, i.e. {v0, v1, v2} -> {v0, v2, v1}.
  int* const embree_index_array =
      static_cast<int*>(rtcMapBuffer(scene_, mesh_id, RTC_INDEX_BUFFER));
  for (size_t i = 0; i < num_triangles_; ++i) {
    embree_index_array[3 * i + 0] = triangle_buffer[i].v0;
    embree_index_array[3 * i + 1] = triangle_buffer[i].v2;
    embree_index_array[3 * i + 2] = triangle_buffer[i].v1;
  }
  rtcUnmapBuffer(scene_, mesh_id, RTC_INDEX_BUFFER);
  rtcCommit(scene_);
  is_scene_committed_ = true;
}

bool SceneManager::AssociateReflectionKernelToTriangles(
    const ReflectionKernel& reflection,
    const std::unordered_set<unsigned int>& triangle_indices) {
  const size_t reflection_index = reflections_.size();
  reflections_.push_back(reflection);
  for (const unsigned int triangle_index : triangle_indices) {
    if (triangle_index >= num_triangles_) {
      return false;
    }
    triangle_to_reflection_map_[triangle_index] = reflection_index;
  }
  return true;
}

void SceneManager::BuildListenerScene(
    const std::vector<AcousticListener>& listeners,
    float listener_sphere_radius) {
  rtcDeleteScene(listener_scene_);
  listener_scene_ = rtcDeviceNewScene(
      device_, RTC_SCENE_STATIC | RTC_SCENE_HIGH_QUALITY, RTC_INTERSECT1);

  for (size_t listener_index = 0; listener_index < listeners.size();
       ++listener_index) {
    // Create a sphere per listener and add to |listener_scene_|.
    const AcousticListener& listener = listeners.at(listener_index);
    const unsigned int sphere_id = rtcNewUserGeometry(listener_scene_, 1);
    Sphere* const sphere =
        AllignedMalloc<Sphere, size_t, Sphere*>(sizeof(Sphere),
                                                /*alignment=*/64);
    sphere->center[0] = listener.position[0];
    sphere->center[1] = listener.position[1];
    sphere->center[2] = listener.position[2];
    sphere->radius = listener_sphere_radius;
    sphere->geometry_id = sphere_id;

    // rtcSetUserData() takes ownership of |sphere|.
    rtcSetUserData(listener_scene_, sphere_id, sphere);
    rtcSetBoundsFunction(listener_scene_, sphere_id,
                         &EmbreeSphereBoundsFunction);
    rtcSetIntersectFunction(listener_scene_, sphere_id,
                            &EmbreeSphereIntersectFunction);

    // Associate the listener to |sphere_id| through its index in the vector
    // of listeners.
    sphere_to_listener_map_[sphere_id] = listener_index;
  }

  rtcCommit(listener_scene_);
  is_listener_scene_committed_ = true;
}

}  // namespace vraudio
