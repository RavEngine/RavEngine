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

#ifndef RESONANCE_AUDIO_PLATFORM_WWISE_WWISE_FX_NULL_PARAMS_H_
#define RESONANCE_AUDIO_PLATFORM_WWISE_WWISE_FX_NULL_PARAMS_H_

#include "AK/SoundEngine/Common/IAkPlugin.h"

namespace vraudio {
namespace wwise {

// Class that implements a *null* plugin parameter node for the Wwise
// integration. This is used by the plugins with *no* exposed parameters to
// avoid redundant code.
class WwiseFxNullParams : public AK::IAkPluginParam {
 public:
  WwiseFxNullParams() = default;

  WwiseFxNullParams(const WwiseFxNullParams& wwise_fx_null_params_copy) =
      default;

  ~WwiseFxNullParams() override = default;

  // Clones parameter node.
  IAkPluginParam* Clone(AK::IAkPluginMemAlloc* memory_allocator) override;

  // Initializes parameter node.
  AKRESULT Init(AK::IAkPluginMemAlloc* memory_allocator,
                const void* params_block, AkUInt32 block_size) override;

  // Terminates parameter node.
  AKRESULT Term(AK::IAkPluginMemAlloc* memory_allocator) override;

  // Sets parameter block.
  AKRESULT SetParamsBlock(const void* params_block,
                          AkUInt32 block_size) override;

  // Updates a single parameter.
  AKRESULT SetParam(AkPluginParamID param_id, const void* param_value,
                    AkUInt32 param_size) override;
};

}  // namespace wwise
}  // namespace vraudio

#endif  // RESONANCE_AUDIO_PLATFORM_WWISE_WWISE_FX_NULL_PARAMS_H_
