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

#ifndef RESONANCE_AUDIO_GRAPH_GRAPH_MANAGER_CONFIG_H_
#define RESONANCE_AUDIO_GRAPH_GRAPH_MANAGER_CONFIG_H_

#include <string>
#include <utility>
#include <vector>

namespace vraudio {

// Configuration of the GraphManager and the nodes it is instantiating.
struct GraphManagerConfig {
  // Configuration name.
  std::string configuration_name;

  // Maximum ambisonic order allowed.
  int max_ambisonic_order = 1;

  // HRIR filenames (second element) per ambisonic order (first element).
  std::vector<std::pair<int, std::string>> sh_hrir_filenames = {};
};

}  // namespace vraudio

#endif  // RESONANCE_AUDIO_GRAPH_GRAPH_MANAGER_CONFIG_H_
