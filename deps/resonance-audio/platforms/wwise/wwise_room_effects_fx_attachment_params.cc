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

#include "platforms/wwise/wwise_room_effects_fx_attachment_params.h"

#include "AK/Tools/Common/AkAssert.h"
#include "AK/Tools/Common/AkBankReadHelpers.h"

namespace vraudio {
namespace wwise {

WwiseRoomEffectsFxAttachmentParams::WwiseRoomEffectsFxAttachmentParams(
    const WwiseRoomEffectsFxAttachmentParams&
        wwise_room_effects_fx_attachment_params_copy) {
  bypass_ = wwise_room_effects_fx_attachment_params_copy.bypass();
}

AK::IAkPluginParam* WwiseRoomEffectsFxAttachmentParams::Clone(
    AK::IAkPluginMemAlloc* memory_allocator) {
  AKASSERT(memory_allocator != nullptr);
  return AK_PLUGIN_NEW(memory_allocator,
                       WwiseRoomEffectsFxAttachmentParams(*this));
}

AKRESULT WwiseRoomEffectsFxAttachmentParams::Init(
    AK::IAkPluginMemAlloc* memory_allocator, const void* params_block,
    AkUInt32 block_size) {
  if (block_size == 0) {
    // Init default parameters.
    bypass_ = false;
    return AK_Success;
  }
  return SetParamsBlock(params_block, block_size);
}

AKRESULT WwiseRoomEffectsFxAttachmentParams::Term(
    AK::IAkPluginMemAlloc* memory_allocator) {
  AKASSERT(memory_allocator != nullptr);
  AK_PLUGIN_DELETE(memory_allocator, this);
  return AK_Success;
}

AKRESULT WwiseRoomEffectsFxAttachmentParams::SetParamsBlock(
    const void* params_block, AkUInt32 block_size) {
  AKRESULT result = AK_Success;

  AkUInt8* bank_params_block =
      const_cast<AkUInt8*>(reinterpret_cast<const AkUInt8*>(params_block));
  bypass_ = READBANKDATA(bool, bank_params_block, block_size);
  CHECKBANKDATASIZE(block_size, result);

  return result;
}

AKRESULT WwiseRoomEffectsFxAttachmentParams::SetParam(AkPluginParamID param_id,
                                                      const void* param_value,
                                                      AkUInt32 param_size) {
  AKASSERT(param_value != nullptr);
  if (param_value == nullptr) {
    return AK_InvalidParameter;
  }

  switch (param_id) {
    case kBypass:
      // Wwise always handles RTPC-able values as |AkReal32|.
      bypass_ = (*reinterpret_cast<const AkReal32*>(param_value) != 0);
      break;
    default:
      AKASSERT(!"Invalid parameter.");
      return AK_InvalidParameter;
  }

  return AK_Success;
}

}  // namespace wwise
}  // namespace vraudio
