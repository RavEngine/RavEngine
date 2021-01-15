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

#include "platforms/fmod/fmod.h"

#ifdef WIN32
#define _CRT_SECURE_NO_WARNINGS
#endif

#include <algorithm>
#include <cmath>
#include <memory>
#include <unordered_map>
#include <utility>

#include "Eigen/Dense"
#include "api/resonance_audio_api.h"
#include "base/constants_and_types.h"
#include "base/misc_math.h"
#include "platforms/common/room_effects_utils.h"
#include "platforms/common/room_properties.h"
#include "platforms/common/utils.h"

// To be able to load modules built with Crosstool/Blaze, we need to export
// __google_auxv.

void* __google_auxv = nullptr;

namespace vraudio {
namespace fmod {

namespace {

// Minimum gain in dB.
const float kMinGainDecibels = -80.0f;

// Names of the different distance rolloff methods.
const char* kDistanceRolloffNames[5] = {
    "Linear", "Logarithmic", "Custom", "Linear Squared", "Logarithmic Tapered"};

// Maximum number of FMOD::System instances.
const int kMaxNumSystemInstances = 8;

// Stores the necessary components for the ResonanceAudio system.
struct ResonanceAudioSystem {
  ResonanceAudioSystem(int sample_rate, size_t num_channels,
                       size_t frames_per_buffer)
      : api(CreateResonanceAudioApi(num_channels, frames_per_buffer,
                                    sample_rate)) {}

  // VrAudio API instance to communicate with the internal system.
  std::unique_ptr<ResonanceAudioApi> api;

  // Current room properties of the environment.

  RoomProperties room_properties;
};

// Pointers to the |ResonanceAudioSystem|, one per FMOD::System instance,
// accessed by the soundfield, source and listener plugins.
static ResonanceAudioSystem* resonance_audio_systems[kMaxNumSystemInstances] = {
    nullptr};

// Piecewise linear mapping for minimum and maximum distance values.
// Note these arrays are static for FMOD_DSP_INIT_PARAMDESC_FLOAT_WITH_MAPPING.
static float distance_mapping_values[] = {0.0f,   1.0f,    5.0f,    10.0f,
                                          100.0f, 1000.0f, 10000.0f};
static float distance_mapping_scale[] = {0.0f,  2.0f,  5.0f, 10.0f,
                                         15.0f, 20.0f, 27.0f};

// Listener plugin parameters.
enum ListenerParams {
  kGlobalGain = 0,
  kRoomProperties,
  kNumListenerParameters
};

// Soundfield Plugin Parameters.
enum SoundfieldParams {
  kSoundfieldGain = 0,
  kSoundfieldAttributes3D,
  kSoundfieldOverallGain,
  kNumSoundfieldParameters
};

// Source Plugin Parameters.
enum SourceParams {
  kGain = 0,
  kSpread,
  kDistanceMin,
  kDistanceMax,
  kDistanceModel,
  kOcclusionIntensity,
  kDirectivityAlpha,
  kDirectivityOrder,
  kAttributes3D,
  kBypassRoom,
  kEnableNearField,
  kNearFieldGain,
  kSourceOverallGain,
  kNumSourceParameters
};

static FMOD_DSP_PARAMETER_DESC listener_gain_param;
static FMOD_DSP_PARAMETER_DESC listener_room_properties_param;

static FMOD_DSP_PARAMETER_DESC soundfield_gain_param;
static FMOD_DSP_PARAMETER_DESC soundfield_3d_attributes_param;
static FMOD_DSP_PARAMETER_DESC soundfield_overall_gain;

static FMOD_DSP_PARAMETER_DESC source_gain_param;
static FMOD_DSP_PARAMETER_DESC source_spread_param;
static FMOD_DSP_PARAMETER_DESC source_distance_min_param;
static FMOD_DSP_PARAMETER_DESC source_distance_max_param;
static FMOD_DSP_PARAMETER_DESC source_distance_model_param;
static FMOD_DSP_PARAMETER_DESC source_occlusion_intensity_param;
static FMOD_DSP_PARAMETER_DESC source_directivity_alpha_param;
static FMOD_DSP_PARAMETER_DESC source_directivity_order_param;
static FMOD_DSP_PARAMETER_DESC source_3d_attributes_param;
static FMOD_DSP_PARAMETER_DESC source_bypass_room_param;
static FMOD_DSP_PARAMETER_DESC source_enable_near_field_param;
static FMOD_DSP_PARAMETER_DESC source_near_field_gain_param;
static FMOD_DSP_PARAMETER_DESC source_overall_gain;

FMOD_DSP_PARAMETER_DESC
*listener_dsp_params[ListenerParams::kNumListenerParameters] = {
    &listener_gain_param,
    &listener_room_properties_param,
};

FMOD_DSP_PARAMETER_DESC
*soundfield_dsp_params[SoundfieldParams::kNumSoundfieldParameters] = {
    &soundfield_gain_param,
    &soundfield_3d_attributes_param,
    &soundfield_overall_gain,
};

FMOD_DSP_PARAMETER_DESC
*source_dsp_params[SourceParams::kNumSourceParameters] = {
    &source_gain_param,
    &source_spread_param,
    &source_distance_min_param,
    &source_distance_max_param,
    &source_distance_model_param,
    &source_occlusion_intensity_param,
    &source_directivity_alpha_param,
    &source_directivity_order_param,
    &source_3d_attributes_param,
    &source_bypass_room_param,
    &source_enable_near_field_param,
    &source_near_field_gain_param,
    &source_overall_gain,
};

FMOD_DSP_DESCRIPTION ListenerDesc = {
    FMOD_PLUGIN_SDK_VERSION,
    "Resonance Audio Listener",  // Name.
    0x00010000,                  // Plug-in version.
    1,                           // Number of input buffers to process.
    1,                           // Number of output buffers to process.
    ListenerCreateCallback,      // DSP::createDSP callback.
    ListenerReleaseCallback,     // DSP::release callback.
    ListenerResetCallback,       // DSP::reset callback.
    nullptr,                     // Called on each DSP update.
    ListenerProcessCallback,     // DSP::process callback.
    nullptr,                     // Channel::setPosition callback.
    ListenerParams::kNumListenerParameters,  // Number of parameters.
    listener_dsp_params,                     // Number of parameters structure.
    ListenerSetParamFloatCallback,           // DSP::setParameterFloat callback.
    nullptr,                                 // DSP::setParameterInt  callback.
    nullptr,                                 // DSP::setParameterBool callback.
    ListenerSetParamDataCallback,            // DSP::setParameterData callback.
    ListenerGetParamFloatCallback,           // DSP::getParameterFloat callback.
    nullptr,                                 // DSP::getParameterInt callback.
    nullptr,                                 // DSP::getParameterBool callback.
    nullptr,                                 // DSP::getParameterData callback.
    nullptr,                                 // DSP::shouldIProcess callback.
    nullptr,                                 // Userdata.
    ListenerSysRegisterCallback,             // Called by System::loadPlugin.
    ListenerSysDeregisterCallback,           // Called by System::release.
    nullptr,  // Called when mixer starts/finishes.
};

FMOD_DSP_DESCRIPTION SoundfieldDesc = {
    FMOD_PLUGIN_SDK_VERSION,
    "Resonance Audio Soundfield",  // Name.
    0x00010000,                    // Plug-in version.
    1,                             // Number of input buffers to process.
    1,                             // Number of output buffers to process.
    SoundfieldCreateCallback,      // DSP::createDSP callback.
    SoundfieldReleaseCallback,     // DSP::release callback.
    SoundfieldResetCallback,       // DSP::reset callback.
    nullptr,                       // Called on each DSP update.
    SoundfieldProcessCallback,     // DSP::process callback.
    nullptr,                       // Channel::setPosition callback.
    SoundfieldParams::kNumSoundfieldParameters,  // Number of parameters.
    soundfield_dsp_params,            // Number of parameters structure.
    SoundfieldSetParamFloatCallback,  // DSP::setParameterFloat callback.
    nullptr,                          // DSP::setParameterInt callback.
    nullptr,                          // DSP::setParameterBool callback.
    SoundfieldSetParamDataCallback,   // DSP::setParameterData callback.
    SoundfieldGetParamFloatCallback,  // DSP::getParameterFloat callback.
    nullptr,                          // DSP::getParameterInt callback.
    nullptr,                          // DSP::getParameterBool callback.
    SoundfieldGetParamDataCallback,   // DSP::getParameterData callback.
    nullptr,                          // DSP::shouldIProcess callback.
    nullptr,                          // Userdata.
    nullptr,                          // Called by System::loadPlugin.
    nullptr,                          // Called by System::release.
    nullptr,                          // Called when mixer starts/finishes.
};

FMOD_DSP_DESCRIPTION SourceDesc = {
    FMOD_PLUGIN_SDK_VERSION,
    "Resonance Audio Source",            // Name.
    0x00010000,                          // Plug-in version.
    1,                                   // Number of input buffers to process.
    1,                                   // Number of output buffers to process.
    SourceCreateCallback,                // DSP::createDSP callback.
    SourceReleaseCallback,               // DSP::release callback.
    SourceResetCallback,                 // DSP::reset callback.
    nullptr,                             // Called on each DSP update.
    SourceProcessCallback,               // DSP::process callback.
    nullptr,                             // Channel::setPosition callback.
    SourceParams::kNumSourceParameters,  // Number of parameters.
    source_dsp_params,                   // Number of parameters structure.
    SourceSetParamFloatCallback,         // DSP::setParameterFloat callback.
    SourceSetParamIntCallback,           // DSP::setParameterInt callback.
    SourceSetParamBoolCallback,          // DSP::setParameterBool callback.
    SourceSetParamDataCallback,          // DSP::setParameterData callback.
    SourceGetParamFloatCallback,         // DSP::getParameterFloat callback.
    SourceGetParamIntCallback,           // DSP::getParameterInt callback.
    SourceGetParamBoolCallback,          // DSP::getParameterBool callback.
    SourceGetParamDataCallback,          // DSP::getParameterData callback.
    nullptr,                             // DSP::shouldIProcess callback.
    nullptr,                             // Userdata.
    nullptr,                             // Called by System::loadPlugin.
    nullptr,                             // Called by System::release.
    nullptr,                             // Called when mixer starts/finishes.
};

static FMOD_PLUGINLIST plugin_list[] = {{FMOD_PLUGINTYPE_DSP, &ListenerDesc},
                                        {FMOD_PLUGINTYPE_DSP, &SoundfieldDesc},
                                        {FMOD_PLUGINTYPE_DSP, &SourceDesc},
                                        {FMOD_PLUGINTYPE_MAX, nullptr}};

// Represents the current listener state, including room effects state.
struct ListenerState {
  // Gain applied to all sources (stored here in dB).
  float gain;

  // Default room properties, which effectively disable the room effects.
  const RoomProperties null_room;
};

// Represents the current state of a soundfield source.
struct SoundfieldState {
  // Gain applied to a soundfield source (stored here in dB).
  float gain;

  // Id number for a soundfield source.
  int soundfield_id;

  // Overall gain value used for fmod
  FMOD_DSP_PARAMETER_OVERALLGAIN overall_gain;
};

// Represents the current state of a sound object source.
struct SourceState {
  // Id number for a sound object source.
  int source_id;

  // Source position.
  WorldPosition position;

  // Gain applied to a source (stored here in dB).
  float gain;

  // Spread of a source in degrees.
  float spread;

  // Intensity of the occlusion effect.
  float occlusion;

  // Roll off model.
  FMOD_DSP_PAN_3D_ROLLOFF_TYPE model;

  // Current distance between source and listener.
  float distance;

  // Minimum distance for the distance rolloff effects.
  float min_distance;

  // Maximum distance for the distance rolloff effects.
  float max_distance;

  // Order of the source directivity pattern, conrols sharpness.
  float directivity_order;

  // Alpha value for the directivity pattern equation.
  float directivity_alpha;

  // Toggles room effects for the source.
  bool bypass_room;

  // Enables near field effects at distances less than 1m for the source.
  bool enable_near_field;

  // Near field effects gain.
  float near_field_gain;

  // Overall gain value with applied falloff model used for fmod
  FMOD_DSP_PARAMETER_OVERALLGAIN overall_gain;
};

// Destroys the |ResonanceAudioSystem| instcorresponding to the given
// |FMOD_DSP_STATE|.
void DestroySystem(FMOD_DSP_STATE* dsp_state) {
  const int system_index = dsp_state->systemobject;
  if (system_index >= kMaxNumSystemInstances) {
    return;
  }
  if (resonance_audio_systems[system_index] != nullptr) {
    delete resonance_audio_systems[system_index];
    resonance_audio_systems[system_index] = nullptr;
  }
}

// Creates an instance of a |ResonanceAudioSystem| and assigns it to
// |resonance_audio_systems.at(dsp_state->systemobject)|.
void CreateSystem(FMOD_DSP_STATE* dsp_state) {
  const int system_index = dsp_state->systemobject;
  if (system_index >= kMaxNumSystemInstances) {
    return;
  }
  int sample_rate = -1;
  FMOD_DSP_GETSAMPLERATE(dsp_state, &sample_rate);
  unsigned int frames_per_buffer = 0;
  FMOD_DSP_GETBLOCKSIZE(dsp_state, &frames_per_buffer);
  if (resonance_audio_systems[system_index] != nullptr) {
    DestroySystem(dsp_state);
  }
  resonance_audio_systems[system_index] = new ResonanceAudioSystem(
      sample_rate, kNumStereoChannels, frames_per_buffer);
}

// Returns a pointer to an instance of the |ResonanceAudioSystem| for the given
// |FMOD_DSP_STATE| or creates one if one does not already exist.
ResonanceAudioSystem* GetSystem(FMOD_DSP_STATE* dsp_state) {
  const int system_index = dsp_state->systemobject;
  if (system_index >= kMaxNumSystemInstances) {
    return nullptr;
  }
  if (resonance_audio_systems[system_index] == nullptr) {
    CreateSystem(dsp_state);
  }
  return resonance_audio_systems[system_index];
}

// Returns the transformation matrix from the given FMOD vectors.
//
// @param position FMOD position vector.
// @param forward FMOD forward vector.
// @param up FMOD up vector.
// @return A 4x4 transformation matrix.
Eigen::Matrix4f GetTransformMatrixFromFmod(const FMOD_VECTOR& fmod_position,
                                           const FMOD_VECTOR& fmod_forward,
                                           const FMOD_VECTOR& fmod_up) {
  const Eigen::Vector3f position(fmod_position.x, fmod_position.y,
                                 fmod_position.z);
  const Eigen::Vector3f forward(fmod_forward.x, fmod_forward.y, fmod_forward.z);
  const Eigen::Vector3f up(fmod_up.x, fmod_up.y, fmod_up.z);
  return GetTransformMatrix(position, forward, up);
}

// Converts a gain value in linear to dB (Minimum value -80.0 dB).
//
// @param linear Gain value in linear.
// @return Gain value in dB.
float DecibelsFromLinear(float linear) {
  return std::max(kMinGainDecibels, 20.0f * std::log10(linear));
}

// Converts a gain value in dB to linear (Minimum value -80.0 dB).
//
// @param decibels Gain value in dB.
// @return Gain value in linear.
float LinearFromDecibels(float decibels) {
  return (decibels <= kMinGainDecibels) ? 0.0f
                                        : std::pow(10.0f, decibels / 20.0f);
}

// Mixes |input| into |input_output|.
//
// @param input Buffer of input data.
// @param input_output Buffer of input data, will contain mixed data on return.
// @param size Length of the two buffers in samples.
void MixBuffers(const float* input, float* input_output, size_t length) {
  for (size_t sample = 0; sample < length; ++sample) {
    input_output[sample] += input[sample];
  }
}

// Initalizes the FMOD plugin parameters for the listener plugin.
void InitializeListenerPluginParameters() {
  FMOD_DSP_INIT_PARAMDESC_FLOAT(listener_gain_param, "Gain", "dB",
                                "[-80.0 to 0.0] Default = 0.0",
                                kMinGainDecibels, 0.0f, -0.00001f);
  FMOD_DSP_INIT_PARAMDESC_DATA(listener_room_properties_param,
                               "Room Properties", "", "",
                               FMOD_DSP_PARAMETER_DATA_TYPE_USER);
}

// Initalizes the FMOD plugin parameters for the soundfield plugin.
void InitializeSoundfieldPluginParameters() {
  FMOD_DSP_INIT_PARAMDESC_FLOAT(soundfield_gain_param, "Gain", "dB",
                                "[-80.0 to 24.0f] Default = 0.0",
                                kMinGainDecibels, 24.0f, 0.0f);
  FMOD_DSP_INIT_PARAMDESC_DATA(soundfield_3d_attributes_param, "3D Attributes",
                               "", "",
                               FMOD_DSP_PARAMETER_DATA_TYPE_3DATTRIBUTES);
  FMOD_DSP_INIT_PARAMDESC_DATA(soundfield_overall_gain, "Overall Gain",
                               "", "Overall Gain",
                               FMOD_DSP_PARAMETER_DATA_TYPE_OVERALLGAIN);
}

// Initalizes the FMOD plugin parameters for the source plugin.
void InitializeSourcePluginParameters() {
  FMOD_DSP_INIT_PARAMDESC_FLOAT(source_gain_param, "Gain", "dB",
                                "[-80.0 to 24.0f] Default = 0.0",
                                kMinGainDecibels, 24.0f, 0.0f);
  FMOD_DSP_INIT_PARAMDESC_FLOAT(source_spread_param, "Spread", "Deg",
                                "Spread in degrees. Default 0.0", 0.0f, 360.0f,
                                0.00001f);
  FMOD_DSP_INIT_PARAMDESC_FLOAT_WITH_MAPPING(
      source_distance_min_param, "Min Distance", "m",
      "[0.0 to 10000.0] Default = 1.0", 1.0f, distance_mapping_values,
      distance_mapping_scale);
  FMOD_DSP_INIT_PARAMDESC_FLOAT_WITH_MAPPING(
      source_distance_max_param, "Max Distance", "m",
      "[0.0 to 10000.0] Default = 500.0", 500.0f, distance_mapping_values,
      distance_mapping_scale);
  FMOD_DSP_INIT_PARAMDESC_INT(
      source_distance_model_param, "Dist Rolloff", "",
      "LINEAR, LOGARITHMIC, NONE, LINEAR SQUARED, LOGARITHMIC TAPERED. Default "
      "= LOGARITHMIC",
      static_cast<int>(FMOD_DSP_PAN_3D_ROLLOFF_LINEARSQUARED),
      static_cast<int>(FMOD_DSP_PAN_3D_ROLLOFF_CUSTOM),
      static_cast<int>(FMOD_DSP_PAN_3D_ROLLOFF_INVERSE), false,
      kDistanceRolloffNames);
  FMOD_DSP_INIT_PARAMDESC_FLOAT(source_occlusion_intensity_param, "Occlusion",
                                "", "[0.0 to 10.0] Default = 0.0", 0.0f, 10.0f,
                                0.000001f);
  FMOD_DSP_INIT_PARAMDESC_FLOAT(source_directivity_alpha_param, "Directivity",
                                "", "[0.0 to 1.0] Default = 0.0", 0.0f, 1.0f,
                                0.00001f);
  FMOD_DSP_INIT_PARAMDESC_FLOAT(source_directivity_order_param, "Dir Sharpness",
                                "", "[1.0 to 10.0)] Default = 1.0", 1.0f, 10.0f,
                                1.00001f);
  FMOD_DSP_INIT_PARAMDESC_DATA(source_3d_attributes_param, "3D Attributes", "",
                               "", FMOD_DSP_PARAMETER_DATA_TYPE_3DATTRIBUTES);
  FMOD_DSP_INIT_PARAMDESC_BOOL(source_bypass_room_param, "Bypass Room", "",
                               "Bypass room effects. Default = false", false,
                               nullptr);
  FMOD_DSP_INIT_PARAMDESC_BOOL(source_enable_near_field_param, "Near-Field FX",
                               "", "Enable Near-Field Effects. Default = false",
                               false, nullptr);
  FMOD_DSP_INIT_PARAMDESC_FLOAT(source_near_field_gain_param, "Near-Field Gain",
                               "", "[0.0 to 9.0)] Default = 1.0", 0.0f, 9.0f,
                               1.00001f);
  FMOD_DSP_INIT_PARAMDESC_DATA(source_overall_gain,
      "Overall Gain", "", "Overall Gain",
      FMOD_DSP_PARAMETER_DATA_TYPE_OVERALLGAIN);
}

}  // namespace

FMOD_RESULT F_CALLBACK ListenerCreateCallback(FMOD_DSP_STATE* dsp_state) {
  // Call GetSystem for its side affect of creating a new |ResonanceAudioSystem|
  // if needed.

  GetSystem(dsp_state);
  void* plugin_data = FMOD_DSP_ALLOC(dsp_state, sizeof(ListenerState));
  dsp_state->plugindata = reinterpret_cast<ListenerState*>(plugin_data);
  if (dsp_state->plugindata == nullptr) {
    return FMOD_ERR_MEMORY;
  }
  return FMOD_OK;
}

FMOD_RESULT F_CALLBACK ListenerReleaseCallback(FMOD_DSP_STATE* dsp_state) {
  ListenerState* state =
      reinterpret_cast<ListenerState*>(dsp_state->plugindata);
  if (state != nullptr) {
    FMOD_DSP_FREE(dsp_state, state);
  }
  return FMOD_OK;
}

FMOD_RESULT F_CALLBACK ListenerResetCallback(FMOD_DSP_STATE* dsp_state) {
  return FMOD_OK;
}

FMOD_RESULT F_CALLBACK ListenerProcessCallback(
    FMOD_DSP_STATE* dsp_state, unsigned int length,
    const FMOD_DSP_BUFFER_ARRAY* in_buffer_array,
    FMOD_DSP_BUFFER_ARRAY* out_buffer_array, FMOD_BOOL inputs_idle,
    FMOD_DSP_PROCESS_OPERATION operation) {
  auto* resonance_audio = GetSystem(dsp_state);
  // This callback will be called twice per mix as it has a dual purpose. Once
  // will be with operation = FMOD_DSP_PROCESS_QUERY,
  // and then depending on the return value of the query, if it is FMOD_OK it
  // will call it again with FMOD_DSP_PROCESS_PERFORM.
  if (operation == FMOD_DSP_PROCESS_QUERY) {
    if (out_buffer_array) {
      out_buffer_array->bufferchannelmask[0] = 0;
      out_buffer_array->buffernumchannels[0] = kNumStereoChannels;
      out_buffer_array->speakermode = FMOD_SPEAKERMODE_STEREO;
    }
    return FMOD_OK;
  }

  // Update room properties.
  resonance_audio->api->SetReflectionProperties(
      ComputeReflectionProperties(resonance_audio->room_properties));
  resonance_audio->api->SetReverbProperties(
      ComputeReverbProperties(resonance_audio->room_properties));

  const size_t buffer_size = kNumStereoChannels * length;
  if (!resonance_audio->api->FillInterleavedOutputBuffer(
          kNumStereoChannels, length, out_buffer_array->buffers[0])) {
    // No valid output was rendered, fill the output buffer with zeros.
    std::fill_n(out_buffer_array->buffers[0], buffer_size, 0.0f);
  }
  if (in_buffer_array &&
      (in_buffer_array->buffernumchannels[0] == kNumStereoChannels)) {
    MixBuffers(in_buffer_array->buffers[0], out_buffer_array->buffers[0],
               buffer_size);
  }
  return FMOD_OK;
}

FMOD_RESULT F_CALLBACK ListenerSetParamFloatCallback(FMOD_DSP_STATE* dsp_state,
                                                     int index, float value) {
  auto* resonance_audio = GetSystem(dsp_state);
  ListenerState* state =
      reinterpret_cast<ListenerState*>(dsp_state->plugindata);
  switch (index) {
    case ListenerParams::kGlobalGain:
      state->gain = value;
      resonance_audio->api->SetMasterVolume(LinearFromDecibels(value));
      return FMOD_OK;
  }
  return FMOD_ERR_INVALID_PARAM;
}

FMOD_RESULT F_CALLBACK ListenerSetParamDataCallback(FMOD_DSP_STATE* dsp_state,
                                                    int index, void* data,
                                                    unsigned int length) {
  auto* resonance_audio = GetSystem(dsp_state);
  ListenerState* state =
      reinterpret_cast<ListenerState*>(dsp_state->plugindata);

  auto room = &state->null_room;
  switch (index) {
    case ListenerParams::kRoomProperties:
      if (data != nullptr && length == sizeof(RoomProperties)) {
        room = reinterpret_cast<RoomProperties*>(data);
      }
      resonance_audio->room_properties = *room;
      break;
    default:
      return FMOD_ERR_INVALID_PARAM;
  }
  return FMOD_OK;
}

FMOD_RESULT F_CALLBACK ListenerGetParamFloatCallback(FMOD_DSP_STATE* dsp_state,
                                                     int index, float* value,
                                                     char* value_string) {
  ListenerState* state =
      reinterpret_cast<ListenerState*>(dsp_state->plugindata);
  switch (index) {
    case ListenerParams::kGlobalGain:
      *value = state->gain;
      return FMOD_OK;
  }
  return FMOD_ERR_INVALID_PARAM;
}

FMOD_RESULT F_CALLBACK ListenerSysRegisterCallback(FMOD_DSP_STATE* dsp_state) {
  return FMOD_OK;
}

FMOD_RESULT F_CALLBACK
ListenerSysDeregisterCallback(FMOD_DSP_STATE* dsp_state) {
  DestroySystem(dsp_state);
  return FMOD_OK;
}

FMOD_RESULT F_CALLBACK SoundfieldCreateCallback(FMOD_DSP_STATE* dsp_state) {
  auto* resonance_audio = GetSystem(dsp_state);
  dsp_state->plugindata = FMOD_DSP_ALLOC(dsp_state, sizeof(SoundfieldState));
  if (dsp_state->plugindata == nullptr) {
    return FMOD_ERR_MEMORY;
  }
  SoundfieldState* state =
      reinterpret_cast<SoundfieldState*>(dsp_state->plugindata);
  state->soundfield_id = resonance_audio->api->CreateAmbisonicSource(
      kNumFirstOrderAmbisonicChannels);
  return FMOD_OK;
}

FMOD_RESULT F_CALLBACK SoundfieldReleaseCallback(FMOD_DSP_STATE* dsp_state) {
  auto* resonance_audio = GetSystem(dsp_state);
  SoundfieldState* state =
      reinterpret_cast<SoundfieldState*>(dsp_state->plugindata);
  if (state != nullptr) {
    if (resonance_audio != nullptr) {
      resonance_audio->api->DestroySource(state->soundfield_id);
    }
    FMOD_DSP_FREE(dsp_state, state);
  }
  return FMOD_OK;
}

FMOD_RESULT F_CALLBACK SoundfieldResetCallback(FMOD_DSP_STATE* dsp_state) {
  SoundfieldState* state =
      reinterpret_cast<SoundfieldState*>(dsp_state->plugindata);
  if (state == nullptr) {
    dsp_state->plugindata = FMOD_DSP_ALLOC(dsp_state, sizeof(SoundfieldState));
    if (dsp_state->plugindata == nullptr) {
      return FMOD_ERR_MEMORY;
    }
    state = reinterpret_cast<SoundfieldState*>(dsp_state->plugindata);
  }
  return FMOD_OK;
}

FMOD_RESULT F_CALLBACK SoundfieldProcessCallback(
    FMOD_DSP_STATE* dsp_state, unsigned int length,
    const FMOD_DSP_BUFFER_ARRAY* in_buffer_array,
    FMOD_DSP_BUFFER_ARRAY* out_buffer_array, FMOD_BOOL inputs_idle,
    FMOD_DSP_PROCESS_OPERATION operation) {
  auto* resonance_audio = GetSystem(dsp_state);
  // This callback will be called twice per mix as it has a dual purpose. Once
  // will be with operation = FMOD_DSP_PROCESS_QUERY, and then depending on the
  // return value of the query, if it is FMOD_OK it will call it again with
  // FMOD_DSP_PROCESS_PERFORM.

  if (operation == FMOD_DSP_PROCESS_QUERY) {
    if (out_buffer_array) {
      out_buffer_array->bufferchannelmask[0] = 0;
      out_buffer_array->buffernumchannels[0] = 1;
      out_buffer_array->speakermode = FMOD_SPEAKERMODE_MONO;
    }
    if (static_cast<bool>(inputs_idle)) {
      return FMOD_ERR_DSP_DONTPROCESS;
    }
  }

  if (operation == FMOD_DSP_PROCESS_PERFORM) {
    SoundfieldState* state =
        reinterpret_cast<SoundfieldState*>(dsp_state->plugindata);

    const size_t num_input_channels =
        static_cast<size_t>(in_buffer_array->buffernumchannels[0]);
    if (num_input_channels == kNumFirstOrderAmbisonicChannels) {
      if (length > 0 && in_buffer_array->numbuffers > 0) {
        resonance_audio->api->SetInterleavedBuffer(
            state->soundfield_id, in_buffer_array->buffers[0],
            kNumFirstOrderAmbisonicChannels, length);
      }
    }

    if (out_buffer_array) {
      const size_t size =
          static_cast<size_t>(length * out_buffer_array->buffernumchannels[0]);
      std::fill_n(out_buffer_array->buffers[0], size, 0.0f);
    }
  }
  return FMOD_OK;
}

FMOD_RESULT F_CALLBACK SoundfieldSetParamFloatCallback(
    FMOD_DSP_STATE* dsp_state, int index, float value) {
  auto* resonance_audio = GetSystem(dsp_state);
  SoundfieldState* state =
      reinterpret_cast<SoundfieldState*>(dsp_state->plugindata);
  switch (index) {
    case SoundfieldParams::kSoundfieldGain:
      state->gain = value;
      resonance_audio->api->SetSourceVolume(state->soundfield_id,
                                            LinearFromDecibels(value));
      return FMOD_OK;
  }
  return FMOD_ERR_INVALID_PARAM;
}

// Callback to be called on update to the source plugin's transform data.
FMOD_RESULT F_CALLBACK SoundfieldSetParamDataCallback(FMOD_DSP_STATE* dsp_state,
                                                      int index, void* data,
                                                      unsigned int length) {
  auto* resonance_audio = GetSystem(dsp_state);
  SoundfieldState* state =
      reinterpret_cast<SoundfieldState*>(dsp_state->plugindata);

  switch (index) {
    case SoundfieldParams::kSoundfieldAttributes3D:
      FMOD_DSP_PARAMETER_3DATTRIBUTES* param =
          reinterpret_cast<FMOD_DSP_PARAMETER_3DATTRIBUTES*>(data);

      // Calculate the source transform matrix.
      Eigen::Matrix4f soundfield_transform = GetTransformMatrixFromFmod(
          param->absolute.position, param->absolute.forward,
          param->absolute.up);
      // Calculate the relative transform matrix from source to listener.
      Eigen::Matrix4f relative_transform = GetTransformMatrixFromFmod(
          param->relative.position, param->relative.forward,
          param->relative.up);
      // Switch to right-handed coordinate system (FMOD is left-handed).
      FlipZAxis(&soundfield_transform);
      FlipZAxis(&relative_transform);
      // Calculate the listener transform matrix.
      const auto listener_transform =
          soundfield_transform * relative_transform.inverse();
      // Set listener transform.
      const Eigen::Vector3f listener_position = GetPosition(listener_transform);
      const Eigen::Quaternionf listener_rotation =
          GetQuaternion(listener_transform);
      resonance_audio->api->SetHeadPosition(
          listener_position.x(), listener_position.y(), listener_position.z());
      resonance_audio->api->SetHeadRotation(
          listener_rotation.x(), listener_rotation.y(), listener_rotation.z(),
          listener_rotation.w());
      // Set soundfield transform.
      const Eigen::Quaternionf soundfield_rotation =
          GetQuaternion(soundfield_transform);
      resonance_audio->api->SetSourceRotation(
          state->soundfield_id, soundfield_rotation.x(),
          soundfield_rotation.y(), soundfield_rotation.z(),
          soundfield_rotation.w());
      return FMOD_OK;
  }
  return FMOD_ERR_INVALID_PARAM;
}

// Callback to be called on request for the source plugin's float parameters.
FMOD_RESULT F_CALLBACK SoundfieldGetParamFloatCallback(
    FMOD_DSP_STATE* dsp_state, int index, float* value, char* value_string) {
  SoundfieldState* state =
      reinterpret_cast<SoundfieldState*>(dsp_state->plugindata);
  switch (index) {
    case SoundfieldParams::kSoundfieldGain:
      *value = state->gain;
      return FMOD_OK;
  }
  return FMOD_ERR_INVALID_PARAM;
}

FMOD_RESULT F_CALLBACK SoundfieldGetParamDataCallback(FMOD_DSP_STATE* dsp_state,
    int index, void **value, unsigned int *length,
    char* value_string) {
    SoundfieldState* state =
        reinterpret_cast<SoundfieldState*>(dsp_state->plugindata);
    switch (index) {
    case SoundfieldParams::kSoundfieldOverallGain:

      state->overall_gain.linear_gain = 0.0f;
      state->overall_gain.linear_gain_additive = LinearFromDecibels(state->gain);

      *value = &state->overall_gain;
      *length = sizeof(state->overall_gain);
      return FMOD_OK;
    }
    return FMOD_ERR_INVALID_PARAM;
}

FMOD_RESULT F_CALLBACK SourceCreateCallback(FMOD_DSP_STATE* dsp_state) {
  auto* resonance_audio = GetSystem(dsp_state);
  dsp_state->plugindata = FMOD_DSP_ALLOC(dsp_state, sizeof(SourceState));
  if (dsp_state->plugindata == nullptr) {
    return FMOD_ERR_MEMORY;
  }
  SourceState* state = reinterpret_cast<SourceState*>(dsp_state->plugindata);
  state->source_id = resonance_audio->api->CreateSoundObjectSource(
      RenderingMode::kBinauralHighQuality);
  // Updates distance model to ensure near field effects are only applied when
  // the minimum distance is below 1m. The +1.0f here ensures that max distance
  // is greater than min distance.
  resonance_audio->api->SetSourceDistanceModel(
      state->source_id, DistanceRolloffModel::kNone, kNearFieldThreshold,
      kNearFieldThreshold + 1.0f);
  return FMOD_OK;
}

FMOD_RESULT F_CALLBACK SourceReleaseCallback(FMOD_DSP_STATE* dsp_state) {
  auto* resonance_audio = GetSystem(dsp_state);
  SourceState* state = reinterpret_cast<SourceState*>(dsp_state->plugindata);
  if (state != nullptr) {
    if (resonance_audio != nullptr) {
      resonance_audio->api->DestroySource(state->source_id);
    }
    FMOD_DSP_FREE(dsp_state, state);
  }
  return FMOD_OK;
}

FMOD_RESULT F_CALLBACK SourceResetCallback(FMOD_DSP_STATE* dsp_state) {
  SourceState* state = reinterpret_cast<SourceState*>(dsp_state->plugindata);
  if (state == nullptr) {
    dsp_state->plugindata = FMOD_DSP_ALLOC(dsp_state, sizeof(SourceState));
    if (dsp_state->plugindata == nullptr) {
      return FMOD_ERR_MEMORY;
    }
    state = reinterpret_cast<SourceState*>(dsp_state->plugindata);
  }
  return FMOD_OK;
}

FMOD_RESULT F_CALLBACK SourceProcessCallback(
    FMOD_DSP_STATE* dsp_state, unsigned int length,
    const FMOD_DSP_BUFFER_ARRAY* in_buffer_array,
    FMOD_DSP_BUFFER_ARRAY* out_buffer_array, FMOD_BOOL inputs_idle,
    FMOD_DSP_PROCESS_OPERATION operation) {
  auto* resonance_audio = GetSystem(dsp_state);
  // This callback will be called twice per mix as it has a dual purpose. Once
  // will be with operation = FMOD_DSP_PROCESS_QUERY,
  // and then depending on the return value of the query, if it is FMOD_OK it
  // will call it again with FMOD_DSP_PROCESS_PERFORM.
  if (operation == FMOD_DSP_PROCESS_PERFORM) {
    SourceState* state = reinterpret_cast<SourceState*>(dsp_state->plugindata);
    // Update room effects gain.
    float room_effects_gain = 0.0f;
    if (!state->bypass_room) {
      room_effects_gain = ComputeRoomEffectsGain(
          state->position,
          WorldPosition(resonance_audio->room_properties.position),
          WorldRotation(resonance_audio->room_properties.rotation),
          WorldPosition(resonance_audio->room_properties.dimensions));
    }
    resonance_audio->api->SetSourceRoomEffectsGain(state->source_id,
                                                   room_effects_gain);
    // Process the next buffer.
    resonance_audio->api->SetInterleavedBuffer(
        state->source_id, in_buffer_array[0].buffers[0],
        in_buffer_array[0].buffernumchannels[0], length);
    if (out_buffer_array) {
      const size_t size = length * out_buffer_array->buffernumchannels[0];
      std::fill_n(out_buffer_array->buffers[0], size, 0.0f);
    }
  } else if (operation == FMOD_DSP_PROCESS_QUERY &&
             static_cast<bool>(inputs_idle)) {
    return FMOD_ERR_DSP_DONTPROCESS;
  }
  return FMOD_OK;
}

FMOD_RESULT F_CALLBACK SourceSetParamFloatCallback(FMOD_DSP_STATE* dsp_state,
                                                   int index, float value) {
  float distance_attenuation;
  auto* resonance_audio = GetSystem(dsp_state);
  SourceState* state = reinterpret_cast<SourceState*>(dsp_state->plugindata);
  switch (index) {
    case SourceParams::kGain:
      state->gain = value;
      resonance_audio->api->SetSourceVolume(state->source_id,
                                            LinearFromDecibels(value));
      return FMOD_OK;
    case SourceParams::kSpread:
      state->spread = value;
      resonance_audio->api->SetSoundObjectSpread(state->source_id, value);
      return FMOD_OK;
    case SourceParams::kDistanceMin:
      state->min_distance = value;
      FMOD_DSP_PAN_GETROLLOFFGAIN(dsp_state, state->model, state->distance,
                                  state->min_distance, state->max_distance,
                                  &distance_attenuation);
      resonance_audio->api->SetSourceDistanceAttenuation(
          state->source_id, distance_attenuation);
      // Updates distance model to ensure near field effects are only applied
      // when the minimum distance is below 1m. The +1.0f here ensures that max
      // distance is greater than min distance. For a min distance of zero,
      // near field effect will be fully on when the source distance is 0.1.
      resonance_audio->api->SetSourceDistanceModel(
          state->source_id, DistanceRolloffModel::kNone, state->min_distance,
          state->min_distance + 1.0f);
      return FMOD_OK;
    case SourceParams::kDistanceMax:
      state->max_distance = value;
      FMOD_DSP_PAN_GETROLLOFFGAIN(dsp_state, state->model, state->distance,
                                  state->min_distance, state->max_distance,
                                  &distance_attenuation);
      resonance_audio->api->SetSourceDistanceAttenuation(
          state->source_id, distance_attenuation);
      return FMOD_OK;
    case SourceParams::kOcclusionIntensity:
      state->occlusion = value;
      resonance_audio->api->SetSoundObjectOcclusionIntensity(
          state->source_id, value);
      return FMOD_OK;
    case SourceParams::kDirectivityAlpha:
      state->directivity_alpha = value;
      resonance_audio->api->SetSoundObjectDirectivity(
          state->source_id, state->directivity_alpha, state->directivity_order);
      return FMOD_OK;
    case SourceParams::kDirectivityOrder:
      state->directivity_order = value;
      resonance_audio->api->SetSoundObjectDirectivity(
          state->source_id, state->directivity_alpha, state->directivity_order);
      return FMOD_OK;
    case SourceParams::kNearFieldGain:
      state->near_field_gain = value;
      resonance_audio->api->SetSoundObjectNearFieldEffectGain(
          state->source_id, state->enable_near_field ? value : 0.0f);
      return FMOD_OK;
  }
  return FMOD_ERR_INVALID_PARAM;
}

FMOD_RESULT F_CALLBACK SourceSetParamIntCallback(FMOD_DSP_STATE* dsp_state,
                                                 int index, int value) {
  float distance_attenuation;
  auto* resonance_audio = GetSystem(dsp_state);
  SourceState* state = reinterpret_cast<SourceState*>(dsp_state->plugindata);
  switch (index) {
    case SourceParams::kDistanceModel:
      switch (value) {
        case 0:
          state->model = FMOD_DSP_PAN_3D_ROLLOFF_LINEAR;
          break;
        case 1:
          state->model = FMOD_DSP_PAN_3D_ROLLOFF_INVERSE;
          break;
        case 2:
          state->model = FMOD_DSP_PAN_3D_ROLLOFF_CUSTOM;
          break;
        case 3:
          state->model = FMOD_DSP_PAN_3D_ROLLOFF_LINEARSQUARED;
          break;
        case 4:
          state->model = FMOD_DSP_PAN_3D_ROLLOFF_INVERSETAPERED;
          break;
        default:
          state->model = FMOD_DSP_PAN_3D_ROLLOFF_INVERSE;
      }
      FMOD_DSP_PAN_GETROLLOFFGAIN(dsp_state, state->model, state->distance,
                                  state->min_distance, state->max_distance,
                                  &distance_attenuation);
      resonance_audio->api->SetSourceDistanceAttenuation(
          state->source_id, distance_attenuation);
      return FMOD_OK;
  }
  return FMOD_ERR_INVALID_PARAM;
}

FMOD_RESULT F_CALLBACK SourceSetParamBoolCallback(FMOD_DSP_STATE* dsp_state,
                                                  int index, FMOD_BOOL value) {
  auto* resonance_audio = GetSystem(dsp_state);
  SourceState* state = reinterpret_cast<SourceState*>(dsp_state->plugindata);
  switch (index) {
    case SourceParams::kBypassRoom:
      // Room effects gain is updated in the process callback.
      state->bypass_room = static_cast<bool>(value);
      return FMOD_OK;
    case SourceParams::kEnableNearField:
      state->enable_near_field = static_cast<bool>(value);
      resonance_audio->api->SetSoundObjectNearFieldEffectGain(
          state->source_id, value ? state->near_field_gain : 0.0f);
      return FMOD_OK;
  }
  return FMOD_ERR_INVALID_PARAM;
}

FMOD_RESULT F_CALLBACK SourceSetParamDataCallback(FMOD_DSP_STATE* dsp_state,
                                                  int index, void* data,
                                                  unsigned int length) {
  auto* resonance_audio = GetSystem(dsp_state);
  SourceState* state = reinterpret_cast<SourceState*>(dsp_state->plugindata);

  switch (index) {
    case SourceParams::kAttributes3D:
      FMOD_DSP_PARAMETER_3DATTRIBUTES* param =
          reinterpret_cast<FMOD_DSP_PARAMETER_3DATTRIBUTES*>(data);

      // Calculate the source transform matrix.
      Eigen::Matrix4f source_transform = GetTransformMatrixFromFmod(
          param->absolute.position, param->absolute.forward,
          param->absolute.up);
      // Calculate the relative transform matrix from source to listener.
      Eigen::Matrix4f relative_transform = GetTransformMatrixFromFmod(
          param->relative.position, param->relative.forward,
          param->relative.up);
      // Switch to right-handed coordinate system (FMOD is left-handed).
      FlipZAxis(&source_transform);
      FlipZAxis(&relative_transform);
      // Calculate the listener transform matrix.
      const auto listener_transform =
          source_transform * relative_transform.inverse();
      // Set listener transform.
      const Eigen::Vector3f listener_position = GetPosition(listener_transform);
      const Eigen::Quaternionf listener_rotation =
          GetQuaternion(listener_transform);
      resonance_audio->api->SetHeadPosition(
          listener_position.x(), listener_position.y(), listener_position.z());
      resonance_audio->api->SetHeadRotation(
          listener_rotation.x(), listener_rotation.y(), listener_rotation.z(),
          listener_rotation.w());
      // Set source transform.
      state->position = GetPosition(source_transform);
      const Eigen::Quaternionf source_rotation =
          GetQuaternion(source_transform);
      resonance_audio->api->SetSourcePosition(
          state->source_id, state->position.x(), state->position.y(),
          state->position.z());
      resonance_audio->api->SetSourceRotation(
          state->source_id, source_rotation.x(), source_rotation.y(),
          source_rotation.z(), source_rotation.w());

      // Update the distance attenuation with respect to the distance between
      // the source and the listener.
      float distance_attenuation;
      state->distance =
          1.0f / FastReciprocalSqrt(
                     param->relative.position.x * param->relative.position.x +
                     param->relative.position.y * param->relative.position.y +
                     param->relative.position.z * param->relative.position.z);
      FMOD_DSP_PAN_GETROLLOFFGAIN(dsp_state, state->model, state->distance,
                                  state->min_distance, state->max_distance,
                                  &distance_attenuation);
      resonance_audio->api->SetSourceDistanceAttenuation(state->source_id,
                                                         distance_attenuation);

      return FMOD_OK;
  }
  return FMOD_ERR_INVALID_PARAM;
}

FMOD_RESULT F_CALLBACK SourceGetParamFloatCallback(FMOD_DSP_STATE* dsp_state,
                                                   int index, float* value,
                                                   char* value_string) {
  SourceState* state = reinterpret_cast<SourceState*>(dsp_state->plugindata);
  switch (index) {
    case SourceParams::kGain:
      *value = state->gain;
      return FMOD_OK;
    case SourceParams::kSpread:
      *value = state->spread;
      return FMOD_OK;
    case SourceParams::kDistanceMin:
      *value = state->min_distance;
      return FMOD_OK;
    case SourceParams::kDistanceMax:
      *value = state->max_distance;
      return FMOD_OK;
    case SourceParams::kOcclusionIntensity:
      *value = state->occlusion;
      return FMOD_OK;
    case SourceParams::kDirectivityAlpha:
      *value = state->directivity_alpha;
      return FMOD_OK;
    case SourceParams::kDirectivityOrder:
      *value = state->directivity_order;
      return FMOD_OK;
    case SourceParams::kNearFieldGain:
      *value = state->near_field_gain;
      return FMOD_OK;
  }
  return FMOD_ERR_INVALID_PARAM;
}

FMOD_RESULT F_CALLBACK SourceGetParamIntCallback(FMOD_DSP_STATE* dsp_state,
                                                 int index, int* value,
                                                 char* value_string) {
  SourceState* state = reinterpret_cast<SourceState*>(dsp_state->plugindata);
  switch (index) {
    case SourceParams::kDistanceModel:
      switch (state->model) {
        case FMOD_DSP_PAN_3D_ROLLOFF_LINEAR:
          *value = 0;
          return FMOD_OK;
        case FMOD_DSP_PAN_3D_ROLLOFF_INVERSE:
          *value = 1;
          return FMOD_OK;
        case FMOD_DSP_PAN_3D_ROLLOFF_CUSTOM:
          *value = 2;
          return FMOD_OK;
        case FMOD_DSP_PAN_3D_ROLLOFF_LINEARSQUARED:
          *value = 3;
          return FMOD_OK;
        case FMOD_DSP_PAN_3D_ROLLOFF_INVERSETAPERED:
          *value = 4;
          return FMOD_OK;
      }
  }
  return FMOD_ERR_INVALID_PARAM;
}

FMOD_RESULT F_CALLBACK SourceGetParamBoolCallback(FMOD_DSP_STATE* dsp_state,
                                                  int index, FMOD_BOOL* value,
                                                  char* value_string) {
  SourceState* state = reinterpret_cast<SourceState*>(dsp_state->plugindata);
  switch (index) {
    case SourceParams::kBypassRoom:
      *value = static_cast<FMOD_BOOL>(state->bypass_room);
      return FMOD_OK;
    case SourceParams::kEnableNearField:
      *value = static_cast<FMOD_BOOL>(state->enable_near_field);
      return FMOD_OK;
  }
  return FMOD_ERR_INVALID_PARAM;
}

FMOD_RESULT F_CALLBACK SourceGetParamDataCallback(FMOD_DSP_STATE* dsp_state,
    int index, void **value, unsigned int *length,
    char* value_string) {
    float distance_attenuation;
    SourceState* state =
        reinterpret_cast<SourceState*>(dsp_state->plugindata);
    switch (index) {
    case SourceParams::kSourceOverallGain:

      FMOD_DSP_PAN_GETROLLOFFGAIN(dsp_state, state->model, state->distance,
          state->min_distance, state->max_distance, &distance_attenuation);

      state->overall_gain.linear_gain = 0.0f;
      state->overall_gain.linear_gain_additive = distance_attenuation * LinearFromDecibels(state->gain);

      *value = &state->overall_gain;
      *length = sizeof(state->overall_gain);
      return FMOD_OK;
    }
    return FMOD_ERR_INVALID_PARAM;
}

F_EXPORT FMOD_PLUGINLIST* F_CALL FMODGetPluginDescriptionList() {
  // Listener plugin parameters.
  InitializeListenerPluginParameters();
  // Soundfield plugin parameters.
  InitializeSoundfieldPluginParameters();
  // Source plugin parameters.
  InitializeSourcePluginParameters();
  return plugin_list;
}

}  // namespace fmod
}  // namespace vraudio

F_EXPORT FMOD_DSP_DESCRIPTION* F_CALL
FMOD_ResonanceAudioListener_GetDSPDescription() {
  // Listener plugin parameters.
  vraudio::fmod::InitializeListenerPluginParameters();
  return &vraudio::fmod::ListenerDesc;
}

F_EXPORT FMOD_DSP_DESCRIPTION* F_CALL
FMOD_ResonanceAudioSoundfield_GetDSPDescription() {
  // Soundfield plugin parameters.
  vraudio::fmod::InitializeSoundfieldPluginParameters();
  return &vraudio::fmod::SoundfieldDesc;
}

F_EXPORT FMOD_DSP_DESCRIPTION* F_CALL
FMOD_ResonanceAudioSource_GetDSPDescription() {
  // Source plugin parameters.
  vraudio::fmod::InitializeSourcePluginParameters();
  return &vraudio::fmod::SourceDesc;
}
