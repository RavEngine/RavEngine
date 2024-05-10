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

#ifndef RESONANCE_AUDIO_GEOMETRICAL_ACOUSTICS_IMPULSE_RESPONSE_COMPUTER_H_
#define RESONANCE_AUDIO_GEOMETRICAL_ACOUSTICS_IMPULSE_RESPONSE_COMPUTER_H_

#include <memory>
#include <vector>

#include "geometrical_acoustics/acoustic_listener.h"
#include "geometrical_acoustics/collection_kernel.h"
#include "geometrical_acoustics/path.h"
#include "geometrical_acoustics/scene_manager.h"

namespace vraudio {

// A class that computes impulse responses from traced sound propagation paths
// and modifies the listener's data (namely listener.energy_impulse_response)
// in place.
// Each listener is associated to a sphere with finite volume, and rays that
// intersect with the sphere contributes energy to the listener.
//
// This class is to be used in conjunction with a PathTracer, which produces
// batches of sound propagation paths that this class consumes.
// Example use (which follows a create-collect-finalize pattern):
//
//   // Create.
//   ImpulseResponseComputer impulse_response_computer(0.1f, 44100.0f,
//                                                     std::move(listeners));
//   // Collect.
//   for (const auto& source : sources) {
//     // Divide |total_num_paths| into |num_batch| batches, each of size
//     // |num_rays_per_batch|.
//     for (size_t i = 0; i < num_batch; ++i) {
//       paths_batch =
//           path_tracer.TracePaths(source, num_rays_per_batch, 10, 1e-6);
//       impulse_response_computer.CollectContributions(paths_batch,
//                                                      total_num_paths);
//     }
//   }
//
//   // Finalize and use the impulse responses in listeners e.g. write them to
//   // a file or pass them to an audio render.
//   for (const auto& listener:
//        impulse_response_computer. GetFinalizedListeners()) {
//     const auto& responses = listener.energy_impulse_responses;
//     // Do something with |responses|.
//   }
class ImpulseResponseComputer {
 public:
  // Constructor.
  //
  // @param listener_sphere_radius Radius of listener spheres (m).
  // @param sampling_rate Sampling rate (Hz).
  // @param listeners Vector of AcousticListener's whose impulse responses are
  //     to be computed.
  // @param scene_manager SceneManager.
  ImpulseResponseComputer(
      float listener_sphere_radius, float sampling_rate,
      std::unique_ptr<std::vector<AcousticListener>> listeners,
      SceneManager* scene_manager);
  virtual ~ImpulseResponseComputer();

  // Collects contributions from a batch of paths to all listeners if
  // the collection is not finalized yet.
  //
  // @param paths_batch All sound propagation paths in a batch.
  void CollectContributions(const std::vector<Path>& paths_batch);

  // Finalizes the listeners and returns them. After calling this, further
  // calls to CollectContributions() have no effect.
  //
  // @return Vector of finalized listeners.
  const std::vector<AcousticListener>& GetFinalizedListeners();

 private:
  // Vector of listeners.
  const std::unique_ptr<std::vector<AcousticListener>> listeners_;

  // Collection kernel used to collect contributions from rays to listeners.
  CollectionKernel collection_kernel_;

  // Scene manager. Used to keep records of listener spheres for ray-sphere
  // intersection tests. Also used in the diffuse rain algorithm to to test
  // whether there is an un-obstructed path from a reflection point to a
  // listener.
  SceneManager* scene_manager_;

  // Is the collection finalized.
  bool finalized_;

  // Total number of paths (contributing or not) that energies are collected
  // from before the collection is finalized. This will be used to average
  // energy contributions.
  size_t num_total_paths_;
};

}  // namespace vraudio
#endif  // RESONANCE_AUDIO_GEOMETRICAL_ACOUSTICS_IMPULSE_RESPONSE_COMPUTER_H_
