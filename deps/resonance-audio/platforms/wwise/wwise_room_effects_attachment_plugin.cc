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

#include "platforms/wwise/wwise_room_effects_attachment_plugin.h"

#include "AK/Tools/Common/AkAssert.h"
#include "platforms/wwise/wwise_common.h"

namespace vraudio {
namespace wwise {

using AK::Wwise::IAudioPlugin;
using AK::Wwise::IPluginPropertySet;
using AK::Wwise::IWriteData;
using AK::Wwise::PopulateTableItem;

namespace {

// Parameter names which correspond to the |ResonanceAudio.xml| file.
static const LPCWSTR kBypassParamName = L"Bypass";

}  // namespace

WwiseRoomEffectsAttachmentPlugin::WwiseRoomEffectsAttachmentPlugin()
    : property_set_(nullptr), property_view_(nullptr) {}

void WwiseRoomEffectsAttachmentPlugin::Destroy() {
  // This is the recommended way of releasing the plugin resources when the
  // user deletes an instance of the plugin in the Wwise Authoring app. See
  // "Destroying the plug-in instance" in the Wwise Documentation for more info:
  // https://www.audiokinetic.com/library/edge/?source=SDK&id=plugin__dll.html
  delete this;
}

void WwiseRoomEffectsAttachmentPlugin::SetPluginPropertySet(
    IPluginPropertySet* property_set) {
  property_set_ = property_set;
}

HINSTANCE WwiseRoomEffectsAttachmentPlugin::GetResourceHandle() const {
  AFX_MANAGE_STATE(AfxGetStaticModuleState());
  return AfxGetStaticModuleState()->m_hCurrentResourceHandle;
}

bool WwiseRoomEffectsAttachmentPlugin::GetDialog(
    IAudioPlugin::eDialog dialog, UINT& dialog_id,
    PopulateTableItem*& populate_table_item) const {
  AKASSERT(dialog == SettingsDialog);
  dialog_id = kDefaultPluginDialogId;
  populate_table_item = nullptr;

  // Returning |false| allows the Wwise Authoring app to create a new dialog
  // dynamically based on the properties defined for the attachment plugin in
  // the |ResonanceAudio.xml| file.
  return false;
}

bool WwiseRoomEffectsAttachmentPlugin::WindowProc(IAudioPlugin::eDialog dialog,
                                                  HWND property_view,
                                                  UINT message, WPARAM wparam,
                                                  LPARAM lparam,
                                                  LRESULT& result) {
  switch (message) {
    case WM_INITDIALOG:
      property_view_ = property_view;
      break;
    case WM_DESTROY:
      property_view_ = nullptr;
      break;
  }

  result = 0;
  return false;
}

bool WwiseRoomEffectsAttachmentPlugin::GetBankParameters(
    const GUID& platform_guid, IWriteData* data_writer) const {
  // Pack parameters in bank. Note that the read/write order should match
  // with the SoundEngine side implementation.
  CComVariant var_properties;

  // Bypass room effects parameter.
  property_set_->GetValue(platform_guid, kBypassParamName, var_properties);
  data_writer->WriteBool(var_properties.boolVal != 0);

  return true;
}

}  // namespace wwise
}  // namespace vraudio
