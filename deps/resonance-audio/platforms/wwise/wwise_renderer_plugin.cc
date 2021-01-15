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

#include "platforms/wwise/wwise_renderer_plugin.h"

#include "AK/Tools/Common/AkAssert.h"
#include "platforms/wwise/wwise_common.h"

namespace vraudio {
namespace wwise {

using AK::Wwise::IAudioPlugin;
using AK::Wwise::IPluginPropertySet;
using AK::Wwise::IWriteData;
using AK::Wwise::PopulateTableItem;

WwiseRendererPlugin::WwiseRendererPlugin()
    : property_set_(nullptr), property_view_(nullptr) {}

void WwiseRendererPlugin::Destroy() {
  // This is the recommended way of releasing the plugin resources when the
  // user deletes an instance of the plugin in the Wwise Authoring app. See
  // "Destroying the plug-in instance" in the Wwise Documentation for more info:
  // https://www.audiokinetic.com/library/edge/?source=SDK&id=plugin__dll.html
  delete this;
}

void WwiseRendererPlugin::SetPluginPropertySet(
    IPluginPropertySet* property_set) {
  property_set_ = property_set;
}

HINSTANCE WwiseRendererPlugin::GetResourceHandle() const {
  AFX_MANAGE_STATE(AfxGetStaticModuleState());
  return AfxGetStaticModuleState()->m_hCurrentResourceHandle;
}

bool WwiseRendererPlugin::GetDialog(
    IAudioPlugin::eDialog dialog, UINT& dialog_id,
    PopulateTableItem*& populate_table_item) const {
  AKASSERT(dialog == SettingsDialog);
  dialog_id = kDefaultPluginDialogId;
  populate_table_item = nullptr;

  // Don't create a UI window for this plugin in the Wwise Authoring app.
  return true;
}

bool WwiseRendererPlugin::WindowProc(IAudioPlugin::eDialog dialog,
                                     HWND property_view, UINT message,
                                     WPARAM wparam, LPARAM lparam,
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

bool WwiseRendererPlugin::GetBankParameters(const GUID& platform_guid,
                                            IWriteData* data_writer) const {
  return true;
}

}  // namespace wwise
}  // namespace vraudio
