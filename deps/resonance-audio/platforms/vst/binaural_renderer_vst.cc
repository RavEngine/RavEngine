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

#include "binaural_renderer_vst.h"

#include <cstdlib>

#include <unordered_map>

#include "base/constants_and_types.h"

namespace {

// Plugin configuration.
const char* kVendorName = "Google";
const char* kEffectName = "ResonanceAudioMonitor";
const VstInt32 kVendorVersion = 1;
const int kNumPluginParameters = 0;
const int kNumPluginPrograms = 1;

}  // namespace

using namespace vraudio;

AudioEffect* createEffectInstance(audioMasterCallback audioMaster) {
  return new BinauralRendererVst(audioMaster);
}

BinauralRendererVst::BinauralRendererVst(audioMasterCallback audioMaster)
    : AudioEffectX(audioMaster, kNumPluginPrograms, kNumPluginParameters),
      framesPerBuffer_(0),
      numInputChannels_(0),
      sampleRateHz_(0),
      binauralSurroundRenderer_(nullptr) {
  setNumInputs(
      kNumThirdOrderAmbisonicChannels);  // Maximum number of input channels.
  setNumOutputs(kNumStereoChannels);     // Stereo output.
  setUniqueID('GVBR');                   // Identifier.
  canProcessReplacing();                 // Supports replacing output.
  vst_strncpy(programName, "Default",
              kVstMaxProgNameLen);  // default program name
  setEditor(BinauralRendererGui::createEditor(this));
}

BinauralRendererVst::~BinauralRendererVst() {}

void BinauralRendererVst::setProgramName(char* name) {
  vst_strncpy(programName, name, kVstMaxProgNameLen);
}

void BinauralRendererVst::getProgramName(char* name) {
  vst_strncpy(name, programName, kVstMaxProgNameLen);
}

bool BinauralRendererVst::getEffectName(char* name) {
  vst_strncpy(name, kEffectName, kVstMaxEffectNameLen);
  return true;
}

bool BinauralRendererVst::getProductString(char* text) {
  vst_strncpy(text, kEffectName, kVstMaxProductStrLen);
  return true;
}

bool BinauralRendererVst::getVendorString(char* text) {
  vst_strncpy(text, kVendorName, kVstMaxVendorStrLen);
  return true;
}

VstInt32 BinauralRendererVst::getVendorVersion() { return kVendorVersion; }

void BinauralRendererVst::processReplacing(float** inputs, float** outputs,
                                           VstInt32 sampleFrames) {
  const VstInt32 numInputChannels = cEffect.numInputs;
  const VstInt32 numOutputChannels = cEffect.numOutputs;
  VstInt32 numOutputChannelsProcessed = 0;

  // Ignore zero input / output channel configurations.
  if (numOutputChannels == 0 || numInputChannels == 0) {
    return;
  }

  // Ignore mono output channel configuration.
  if (numOutputChannels == static_cast<VstInt32>(kNumMonoChannels)) {
    std::fill_n(outputs[0], sampleFrames, 0.0f);
    return;
  }

  if (framesPerBuffer_ != sampleFrames ||
      numInputChannels_ != numInputChannels ||
      sampleRateHz_ != static_cast<int>(getSampleRate())) {
    if (!initBinauralSurroundRenderer(sampleFrames, numInputChannels,
                                      static_cast<int>(getSampleRate()))) {
      binauralSurroundRenderer_.reset();
    }
  }

  if (binauralSurroundRenderer_ != nullptr) {
    binauralSurroundRenderer_->AddPlanarInput(inputs, numInputChannels,
                                              sampleFrames);
    binauralSurroundRenderer_->GetPlanarStereoOutput(outputs, sampleFrames);
    numOutputChannelsProcessed += static_cast<VstInt32>(kNumStereoChannels);
  }

  // Clear remaining output buffers.
  for (VstInt32 channel = numOutputChannelsProcessed;
       channel < numOutputChannels; ++channel) {
    std::fill_n(outputs[channel], sampleFrames, 0.0f);
  }
}

bool BinauralRendererVst::initBinauralSurroundRenderer(
    VstInt32 framesPerBuffer, VstInt32 numInputChannels, int sampleRateHz) {
  BinauralSurroundRenderer::SurroundFormat surround_format =
      BinauralSurroundRenderer::SurroundFormat::kInvalid;
  switch (numInputChannels) {
    case kNumFirstOrderAmbisonicChannels:
      surround_format =
          BinauralSurroundRenderer::SurroundFormat::kFirstOrderAmbisonics;
      break;
    case kNumFirstOrderAmbisonicWithNonDiegeticStereoChannels:
      surround_format = BinauralSurroundRenderer::SurroundFormat::
          kFirstOrderAmbisonicsWithNonDiegeticStereo;
      break;
    case kNumSecondOrderAmbisonicChannels:
      surround_format =
          BinauralSurroundRenderer::SurroundFormat::kSecondOrderAmbisonics;
      break;
    case kNumSecondOrderAmbisonicWithNonDiegeticStereoChannels:
      surround_format = BinauralSurroundRenderer::SurroundFormat::
          kSecondOrderAmbisonicsWithNonDiegeticStereo;
      break;
    case kNumThirdOrderAmbisonicChannels:
      surround_format =
          BinauralSurroundRenderer::SurroundFormat::kThirdOrderAmbisonics;
      break;
    case kNumThirdOrderAmbisonicWithNonDiegeticStereoChannels:
      surround_format = BinauralSurroundRenderer::SurroundFormat::
          kThirdOrderAmbisonicsWithNonDiegeticStereo;
      break;
    default:
      // Unsupported number of input channels.
      return false;
      break;
  }

  binauralSurroundRenderer_.reset(BinauralSurroundRenderer::Create(
      framesPerBuffer, sampleRateHz, surround_format));
  if (binauralSurroundRenderer_ == nullptr) {
    return false;
  }

  framesPerBuffer_ = framesPerBuffer;
  numInputChannels_ = numInputChannels;
  sampleRateHz_ = sampleRateHz;
  return true;
}
