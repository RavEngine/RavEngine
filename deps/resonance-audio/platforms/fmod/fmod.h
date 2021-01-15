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

#ifndef RESONANCE_AUDIO_PLATFORM_FMOD_FMOD_H_
#define RESONANCE_AUDIO_PLATFORM_FMOD_FMOD_H_

#include "api/core/inc/fmod.hpp"

namespace vraudio {
namespace fmod {

// Listener Callbacks.

// Callback to be called on creation of a listener plugin instance.
FMOD_RESULT F_CALLBACK ListenerCreateCallback(FMOD_DSP_STATE* dsp_state);

// Callback to be called on release of a listener plugin instance.
FMOD_RESULT F_CALLBACK ListenerReleaseCallback(FMOD_DSP_STATE* dsp_state);

// Callback to be called when a listener plugin instance is reset.
FMOD_RESULT F_CALLBACK ListenerResetCallback(FMOD_DSP_STATE* dsp_state);

// Callback to be called to process the next output buffer.
FMOD_RESULT F_CALLBACK ListenerProcessCallback(
    FMOD_DSP_STATE* dsp_state, unsigned int length,
    const FMOD_DSP_BUFFER_ARRAY* in_buffer_array,
    FMOD_DSP_BUFFER_ARRAY* out_buffer_array, FMOD_BOOL inputs_idle,
    FMOD_DSP_PROCESS_OPERATION operation);

// Callback to be called on update of the listener plugin's float parameters.
FMOD_RESULT F_CALLBACK ListenerSetParamFloatCallback(FMOD_DSP_STATE* dsp_state,
                                                     int index, float value);

// Callback to be called on update to the listener plugin's room data.
FMOD_RESULT F_CALLBACK ListenerSetParamDataCallback(FMOD_DSP_STATE* dsp_state,
                                                    int index, void* data,
                                                    unsigned int length);

// Callback to be called on request for the listener plugin's float parameters.
FMOD_RESULT F_CALLBACK ListenerGetParamFloatCallback(FMOD_DSP_STATE* dsp_state,
                                                     int index, float* value,
                                                     char* value_string);

// Callback to be called on registration of the listener plugin.
FMOD_RESULT F_CALLBACK ListenerSysRegisterCallback(FMOD_DSP_STATE* dsp_state);

// Callback to be called on deregistration of the listener plugin.
FMOD_RESULT F_CALLBACK ListenerSysDeregisterCallback(FMOD_DSP_STATE* dsp_state);

// Soundfield Callbacks.

// Callback to be called on creation of a soundfield plugin instance.
FMOD_RESULT F_CALLBACK SoundfieldCreateCallback(FMOD_DSP_STATE* dsp_state);

// Callback to be called on release of a soundfield plugin instance.
FMOD_RESULT F_CALLBACK SoundfieldReleaseCallback(FMOD_DSP_STATE* dsp_state);

// Callback to be called when a soundfield plugin instance is reset.
FMOD_RESULT F_CALLBACK SoundfieldResetCallback(FMOD_DSP_STATE* dsp_state);

// Callback to be called on each buffer loop. Adds audio data to the Google VR
// audio graph.
FMOD_RESULT F_CALLBACK SoundfieldProcessCallback(
    FMOD_DSP_STATE* dsp_state, unsigned int length,
    const FMOD_DSP_BUFFER_ARRAY* in_buffer_array,
    FMOD_DSP_BUFFER_ARRAY* out_buffer_array, FMOD_BOOL inputs_idle,
    FMOD_DSP_PROCESS_OPERATION operation);

// Callback to be called on update to the soundfield plugin's float parameters.
FMOD_RESULT F_CALLBACK SoundfieldSetParamFloatCallback(
    FMOD_DSP_STATE* dsp_state, int index, float value);

// Callback to be called on update to the soundfield plugin's transform data.
FMOD_RESULT F_CALLBACK SoundfieldSetParamDataCallback(FMOD_DSP_STATE* dsp_state,
                                                      int index, void* data,
                                                      unsigned int length);

// Callback to be called on request for the soundfield plugin's float
// parameters.
FMOD_RESULT F_CALLBACK SoundfieldGetParamFloatCallback(
    FMOD_DSP_STATE* dsp_state, int index, float* value, char* value_string);

// Callback to be called on request for the soundfield plugin's data parameters.
FMOD_RESULT F_CALLBACK SoundfieldGetParamDataCallback(
    FMOD_DSP_STATE* dsp_state, int index, void **value,
    unsigned int *length, char* value_string);

// Source Callbacks.

// Callback to be called on creation of a source plugin instance.
FMOD_RESULT F_CALLBACK SourceCreateCallback(FMOD_DSP_STATE* dsp_state);

// Callback to be called on release of a source plugin instance.
FMOD_RESULT F_CALLBACK SourceReleaseCallback(FMOD_DSP_STATE* dsp_state);

// Callback to be called when a source plugin instance is reset.
FMOD_RESULT F_CALLBACK SourceResetCallback(FMOD_DSP_STATE* dsp_state);

// Callback to be called on each buffer loop. Adds audio data to the Google VR
// audio graph.
FMOD_RESULT F_CALLBACK SourceProcessCallback(
    FMOD_DSP_STATE* dsp_state, unsigned int length,
    const FMOD_DSP_BUFFER_ARRAY* in_buffer_array,
    FMOD_DSP_BUFFER_ARRAY* out_buffer_array, FMOD_BOOL inputs_idle,
    FMOD_DSP_PROCESS_OPERATION operation);

// Callback to be called on update to the source plugin's float parameters.
FMOD_RESULT F_CALLBACK SourceSetParamFloatCallback(FMOD_DSP_STATE* dsp_state,
                                                   int index, float value);

// Callback to be called on update to the source plugin's int parameters.
FMOD_RESULT F_CALLBACK SourceSetParamIntCallback(FMOD_DSP_STATE* dsp_state,
                                                 int index, int value);

// Callback to be called on update to the source plugin's bool parameters.
FMOD_RESULT F_CALLBACK SourceSetParamBoolCallback(FMOD_DSP_STATE* dsp_state,
                                                  int index, FMOD_BOOL value);

// Callback to be called on update to the source plugin's transform data.
FMOD_RESULT F_CALLBACK SourceSetParamDataCallback(FMOD_DSP_STATE* dsp_state,
                                                  int index, void* data,
                                                  unsigned int length);

// Callback to be called on request for the source plugin's float parameters.
FMOD_RESULT F_CALLBACK SourceGetParamFloatCallback(FMOD_DSP_STATE* dsp_state,
                                                   int index, float* value,
                                                   char* value_string);

// Callback to be called on request for the source plugin's int parameters.
FMOD_RESULT F_CALLBACK SourceGetParamIntCallback(FMOD_DSP_STATE* dsp_state,
                                                 int index, int* value,
                                                 char* value_string);

// Callback to be called on request for the source plugin's bool parameters.
FMOD_RESULT F_CALLBACK SourceGetParamBoolCallback(FMOD_DSP_STATE* dsp_state,
                                                  int index, FMOD_BOOL* value,
                                                  char* value_string);

// Callback to be called on request for the soundfield plugin's data parameters.
FMOD_RESULT F_CALLBACK SourceGetParamDataCallback(FMOD_DSP_STATE* dsp_state,
                                                  int index, void **value, unsigned int *length,
                                                  char* value_string);

extern "C" {

// Returns an FMOD_PLUGINLIST containing pointers to the DSP plugin descriptions
// for both the listener and source plugins.
F_EXPORT FMOD_PLUGINLIST* F_CALL FMODGetPluginDescriptionList();

}  // extern C

}  // namespace fmod
}  // namespace vraudio

#if defined(TARGET_OS_IPHONE) && defined(__APPLE__)
extern "C" {

// Returns plugin descriptions indivdually for each of the three Resonance Audio
// plugins This is necessary on iOS as FMOD do not support
// FMODGetPluginDescriptionList for static libraries.
F_EXPORT FMOD_DSP_DESCRIPTION* F_CALL
FMOD_ResonanceAudioListener_GetDSPDescription();

F_EXPORT FMOD_DSP_DESCRIPTION* F_CALL
FMOD_ResonanceAudioSoundfield_GetDSPDescription();

F_EXPORT FMOD_DSP_DESCRIPTION* F_CALL
FMOD_ResonanceAudioSource_GetDSPDescription();

}  // extern C
#endif  // defined(TARGET_OS_IPHONE) && defined(__APPLE__)

#endif  // RESONANCE_AUDIO_PLATFORM_FMOD_FMOD_H_
