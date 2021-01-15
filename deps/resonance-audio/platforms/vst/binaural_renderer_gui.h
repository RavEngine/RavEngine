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

#ifndef RESONANCE_AUDIO_PLATFORM_VST_BINAURAL_RENDERER_GUI_H_
#define RESONANCE_AUDIO_PLATFORM_VST_BINAURAL_RENDERER_GUI_H_

#include "plugin-bindings/aeffguieditor.h"

// Implements the VST GUI interface for |BinauralRendererVst|.
class BinauralRendererGui : public AEffGUIEditor {
 public:
  BinauralRendererGui(void* ptr);
  ~BinauralRendererGui() override {}

  static AEffEditor* createEditor(AudioEffectX* effect);

  // Implements AEffGUIEditor interface.
  bool open(void* ptr) override;
  void close() override;
};

#endif  // RESONANCE_AUDIO_PLATFORM_VST_BINAURAL_RENDERER_GUI_H_
