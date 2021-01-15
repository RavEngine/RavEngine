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

#ifndef RESONANCE_AUDIO_GEOMETRICAL_ACOUSTICS_PATH_TRACER_H_
#define RESONANCE_AUDIO_GEOMETRICAL_ACOUSTICS_PATH_TRACER_H_

#include <functional>
#include <vector>

#include "geometrical_acoustics/acoustic_source.h"
#include "geometrical_acoustics/path.h"
#include "geometrical_acoustics/scene_manager.h"

namespace vraudio {

// A wrapper around Embree that finds sound propagation paths from a source.
class PathTracer {
 public:
  // Constructor.
  //
  // @param scene_manager Scene manager.
  explicit PathTracer(const SceneManager& scene_manager)
      : scene_manager_(scene_manager) {}
  ~PathTracer() {}

  // Traces sound propagation paths from a source.
  //
  // @param source Source from which paths are traced.
  // @param min_num_rays Minimum number of rays to be traced.
  // @param max_depth Maximum depth of tracing performed along a path. The
  //     tracing stops when reading |max_depth| interactions with the scene
  //     geometries.
  // @param energy_threshold Energy threshold below which the tracing stops.
  // @return Vector of at least |min_num_rays| traced paths.
  std::vector<Path> TracePaths(const AcousticSource& source,
                               size_t min_num_rays, size_t max_depth,
                               float energy_threshold);

 private:
  // Scene manager.
  const SceneManager& scene_manager_;
};

}  // namespace vraudio

#endif  // RESONANCE_AUDIO_GEOMETRICAL_ACOUSTICS_PATH_TRACER_H_
