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

#ifndef RESONANCE_AUDIO_PLATFORM_VST_BINAURAL_RENDERER_VST_H_
#define RESONANCE_AUDIO_PLATFORM_VST_BINAURAL_RENDERER_VST_H_

#include <memory>

#include "api/binaural_surround_renderer.h"
#include "public.sdk/source/vst2.x/audioeffectx.h"
#include "binaural_renderer_gui.h"

class BinauralRendererVst : public AudioEffectX {
 public:
  explicit BinauralRendererVst(audioMasterCallback audioMaster);
  ~BinauralRendererVst() override;

  // Processing.
  void processReplacing(float** inputs, float** outputs,
                        VstInt32 sampleFrames) override;

  // Program.
  void setProgramName(char* name) override;
  void getProgramName(char* name) override;

  bool getEffectName(char* name) override;
  bool getVendorString(char* text) override;
  bool getProductString(char* text) override;
  VstInt32 getVendorVersion() override;

  VstPlugCategory getPlugCategory() override { return kPlugCategEffect; }

 protected:
  // Creates |BinauralSurroundRenderer| instances.
  bool initBinauralSurroundRenderer(VstInt32 framesPerBuffer,
                                    VstInt32 numInputChannels,
                                    int sampleRateHz);

  // VST program name.
  char programName[kVstMaxProgNameLen + 1];

  // Buffer size |binauralSurroundRenderer_| has been initialized with.
  VstInt32 framesPerBuffer_;

  // Buffer size |binauralSurroundRenderer_| has been initialized with.
  VstInt32 numInputChannels_;

  // Sample rate |binauralSurroundRenderer_| has been initialized with.
  int sampleRateHz_;

  // |BinauralSurroundRenderer| instance.
  std::unique_ptr<vraudio::BinauralSurroundRenderer> binauralSurroundRenderer_;
};

#endif  // RESONANCE_AUDIO_PLATFORM_VST_BINAURAL_RENDERER_VST_H_
