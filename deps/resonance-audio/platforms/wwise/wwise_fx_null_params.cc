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

#include "platforms/wwise/wwise_fx_null_params.h"

#include "AK/Tools/Common/AkAssert.h"

namespace vraudio {
namespace wwise {

AK::IAkPluginParam* WwiseFxNullParams::Clone(
    AK::IAkPluginMemAlloc* memory_allocator) {
  AKASSERT(memory_allocator != nullptr);
  return AK_PLUGIN_NEW(memory_allocator, WwiseFxNullParams(*this));
}

AKRESULT WwiseFxNullParams::Init(AK::IAkPluginMemAlloc* memory_allocator,
                                 const void* params_block,
                                 AkUInt32 block_size) {
  if (block_size == 0) {
    // Init default parameters.
    return AK_Success;
  }
  return SetParamsBlock(params_block, block_size);
}

AKRESULT WwiseFxNullParams::Term(AK::IAkPluginMemAlloc* memory_allocator) {
  AKASSERT(memory_allocator != nullptr);
  AK_PLUGIN_DELETE(memory_allocator, this);
  return AK_Success;
}

AKRESULT WwiseFxNullParams::SetParamsBlock(const void* params_block,
                                           AkUInt32 block_size) {
  return AK_Success;
}

AKRESULT WwiseFxNullParams::SetParam(AkPluginParamID param_id,
                                     const void* param_value,
                                     AkUInt32 param_size) {
  return AK_Success;
}

}  // namespace wwise
}  // namespace vraudio
