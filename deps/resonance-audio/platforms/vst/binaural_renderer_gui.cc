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

#include "binaural_renderer_gui.h"

BinauralRendererGui::BinauralRendererGui(void* ptr) : AEffGUIEditor(ptr) {
  frame = 0;
}

AEffEditor* BinauralRendererGui::createEditor(AudioEffectX* effect) {
  return new BinauralRendererGui(effect);
}

bool BinauralRendererGui::open(void* ptr) {
  CBitmap* background = new CBitmap("resonance_audio.png");

  // Initialize the size of the plugin (required by some DAWs like Ableton).
  rect.left = 0;
  rect.top = 0;
  rect.right = static_cast<VstInt16>(background->getWidth());
  rect.bottom = static_cast<VstInt16>(background->getHeight());

  CRect size(0, 0, background->getWidth(), background->getHeight());
  CFrame* new_frame = new CFrame(size, this);
  new_frame->open(ptr);
  new_frame->setBackground(background);
  frame = new_frame;

  background->forget();
  return true;
}

void BinauralRendererGui::close() {
  if (frame != nullptr) {
    frame->forget();
    frame = nullptr;
  }
}
