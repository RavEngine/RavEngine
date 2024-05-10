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

#include "platforms/wwise/wwise_win_app.h"

#include "AK/SoundEngine/Common/IAkPlugin.h"
#include "platforms/wwise/wwise_common.h"
#include "platforms/wwise/wwise_fx_factory.h"
#include "platforms/wwise/wwise_renderer_plugin.h"
#include "platforms/wwise/wwise_room_effects_attachment_plugin.h"
#include "platforms/wwise/wwise_room_effects_plugin.h"

// WwiseWinApp.
BEGIN_MESSAGE_MAP(WwiseWinApp, CWinApp)
END_MESSAGE_MAP()

// The one and only WwiseWinApp instance.
WwiseWinApp wwise_win_app;

BOOL WwiseWinApp::InitInstance() {
  CWinApp::InitInstance();
  AK::Wwise::RegisterWwisePlugin();
  return TRUE;
}

AK::Wwise::IPluginBase* __stdcall AkCreatePlugin(
    unsigned short company_id,
    unsigned short plugin_id) {
  if (company_id == vraudio::wwise::kCompanyId) {
    if (plugin_id == vraudio::wwise::kRendererPluginId) {
      // Binaural renderer plugin.
      return new vraudio::wwise::WwiseRendererPlugin;
    } else if (plugin_id == vraudio::wwise::kRoomEffectsPluginId) {
      // Room effects plugin.
      return new vraudio::wwise::WwiseRoomEffectsPlugin;
    } else if (plugin_id == vraudio::wwise::kRoomEffectsAttachmentPluginId) {
      // Room effects attachment plugin.
      return new vraudio::wwise::WwiseRoomEffectsAttachmentPlugin;
    }
  }
  return NULL;
}

/// Dummy assert hook for Wwise plug-ins using AKASSERT (cassert used by
/// default).
DEFINEDUMMYASSERTHOOK;
DEFINE_PLUGIN_REGISTER_HOOK;
