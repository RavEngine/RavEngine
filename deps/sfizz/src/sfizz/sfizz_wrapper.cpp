// SPDX-License-Identifier: BSD-2-Clause

// This code is part of the sfizz library and is licensed under a BSD 2-clause
// license. You should have receive a LICENSE.md file along with the code.
// If not, contact the sfizz maintainers at https://github.com/sfztools/sfizz

#include "Config.h"
#include "Synth.h"
#include "Messaging.h"
#include "utility/Macros.h"
#include "sfizz.h"
#include "sfizz_private.hpp"
#include <limits>

#ifdef __cplusplus
extern "C" {
#endif

sfizz_synth_t* sfizz_create_synth()
{
    return new sfizz_synth_t;
}

bool sfizz_load_file(sfizz_synth_t* synth, const char* path)
{
    return synth->synth.loadSfzFile(path);
}

bool sfizz_load_string(sfizz_synth_t* synth, const char* path, const char* text)
{
    return synth->synth.loadSfzString(path, text);
}

bool sfizz_load_scala_file(sfizz_synth_t* synth, const char* path)
{
    return synth->synth.loadScalaFile(path);
}

bool sfizz_load_scala_string(sfizz_synth_t* synth, const char* text)
{
    return synth->synth.loadScalaString(text);
}

void sfizz_set_scala_root_key(sfizz_synth_t* synth, int root_key)
{
    synth->synth.setScalaRootKey(root_key);
}

int sfizz_get_scala_root_key(sfizz_synth_t* synth)
{
    return synth->synth.getScalaRootKey();
}

void sfizz_set_tuning_frequency(sfizz_synth_t* synth, float frequency)
{
    synth->synth.setTuningFrequency(frequency);
}

float sfizz_get_tuning_frequency(sfizz_synth_t* synth)
{
    return synth->synth.getTuningFrequency();
}

void sfizz_load_stretch_tuning_by_ratio(sfizz_synth_t* synth, float ratio)
{
    synth->synth.loadStretchTuningByRatio(ratio);
}

void sfizz_add_ref(sfizz_synth_t* synth)
{
    synth->remember();
}

void sfizz_free(sfizz_synth_t* synth)
{
    synth->forget();
}

int sfizz_get_num_regions(sfizz_synth_t* synth)
{
    return synth->synth.getNumRegions();
}
int sfizz_get_num_groups(sfizz_synth_t* synth)
{
    return synth->synth.getNumGroups();
}
int sfizz_get_num_masters(sfizz_synth_t* synth)
{
    return synth->synth.getNumMasters();
}
int sfizz_get_num_curves(sfizz_synth_t* synth)
{
    return synth->synth.getNumCurves();
}
char* sfizz_export_midnam(sfizz_synth_t* synth, const char* model)
{
    return strdup(synth->synth.exportMidnam(model ? model : "").c_str());
}
size_t sfizz_get_num_preloaded_samples(sfizz_synth_t* synth)
{
    return synth->synth.getNumPreloadedSamples();
}
int sfizz_get_num_active_voices(sfizz_synth_t* synth)
{
    return synth->synth.getNumActiveVoices();
}

void sfizz_set_samples_per_block(sfizz_synth_t* synth, int samples_per_block)
{
    synth->synth.setSamplesPerBlock(samples_per_block);
}
void sfizz_set_sample_rate(sfizz_synth_t* synth, float sample_rate)
{
    synth->synth.setSampleRate(sample_rate);
}

void sfizz_send_note_on(sfizz_synth_t* synth, int delay, int note_number, int velocity)
{
    synth->synth.noteOn(delay, note_number, velocity);
}
void sfizz_send_hd_note_on(sfizz_synth_t* synth, int delay, int note_number, float velocity)
{
    synth->synth.hdNoteOn(delay, note_number, velocity);
}
void sfizz_send_note_off(sfizz_synth_t* synth, int delay, int note_number, int velocity)
{
    synth->synth.noteOff(delay, note_number, velocity);
}
void sfizz_send_hd_note_off(sfizz_synth_t* synth, int delay, int note_number, float velocity)
{
    synth->synth.hdNoteOff(delay, note_number, velocity);
}
void sfizz_send_cc(sfizz_synth_t* synth, int delay, int cc_number, int cc_value)
{
    synth->synth.cc(delay, cc_number, cc_value);
}
void sfizz_send_hdcc(sfizz_synth_t* synth, int delay, int cc_number, float norm_value)
{
    synth->synth.hdcc(delay, cc_number, norm_value);
}
void sfizz_automate_hdcc(sfizz_synth_t* synth, int delay, int cc_number, float norm_value)
{
    synth->synth.automateHdcc(delay, cc_number, norm_value);
}
void sfizz_send_pitch_wheel(sfizz_synth_t* synth, int delay, int pitch)
{
    synth->synth.pitchWheel(delay, pitch);
}
void sfizz_send_hd_pitch_wheel(sfizz_synth_t* synth, int delay, float pitch)
{
    synth->synth.hdPitchWheel(delay, pitch);
}
void sfizz_send_program_change(sfizz_synth_t* synth, int delay, int program)
{
    synth->synth.programChange(delay, program);
}
void sfizz_send_aftertouch(sfizz_synth_t* synth, int delay, int aftertouch)
{
    synth->synth.channelAftertouch(delay, aftertouch);
}
void sfizz_send_channel_aftertouch(sfizz_synth_t* synth, int delay, int aftertouch)
{
    synth->synth.channelAftertouch(delay, aftertouch);
}
void sfizz_send_hd_channel_aftertouch(sfizz_synth_t* synth, int delay, float aftertouch)
{
    synth->synth.hdChannelAftertouch(delay, aftertouch);
}
void sfizz_send_poly_aftertouch(sfizz_synth_t* synth, int delay, int note_number, int aftertouch)
{
    synth->synth.polyAftertouch(delay, note_number, aftertouch);
}
void sfizz_send_hd_poly_aftertouch(sfizz_synth_t* synth, int delay, int note_number, float aftertouch)
{
    synth->synth.hdPolyAftertouch(delay, note_number, aftertouch);
}
void sfizz_send_tempo(sfizz_synth_t* synth, int delay, float seconds_per_quarter)
{
    synth->synth.tempo(delay, seconds_per_quarter);
}
void sfizz_send_bpm_tempo(sfizz_synth_t* synth, int delay, float beats_per_minute)
{
    synth->synth.bpmTempo(delay, beats_per_minute);
}
void sfizz_send_time_signature(sfizz_synth_t* synth, int delay, int beats_per_bar, int beat_unit)
{
    synth->synth.timeSignature(delay, beats_per_bar, beat_unit);
}
void sfizz_send_time_position(sfizz_synth_t* synth, int delay, int bar, double bar_beat)
{
    synth->synth.timePosition(delay, bar, bar_beat);
}
void sfizz_send_playback_state(sfizz_synth_t* synth, int delay, int playback_state)
{
    synth->synth.playbackState(delay, playback_state);
}

void sfizz_render_block(sfizz_synth_t* synth, float** channels, int num_channels, int num_frames)
{
    sfz::AudioSpan<float> channelSpan { channels, static_cast<size_t>(num_channels), 0, static_cast<size_t>(num_frames) };
    synth->synth.renderBlock(channelSpan);
}

unsigned int sfizz_get_preload_size(sfizz_synth_t* synth)
{
    return synth->synth.getPreloadSize();
}
void sfizz_set_preload_size(sfizz_synth_t* synth, unsigned int preload_size)
{
    synth->synth.setPreloadSize(preload_size);
}

sfizz_oversampling_factor_t sfizz_get_oversampling_factor(sfizz_synth_t*)
{
    return SFIZZ_OVERSAMPLING_X1;
}

bool sfizz_set_oversampling_factor(sfizz_synth_t*, sfizz_oversampling_factor_t)
{
    return true;
}

int sfizz_get_sample_quality(sfizz_synth_t* synth, sfizz_process_mode_t mode)
{
    return synth->synth.getSampleQuality(static_cast<sfz::Synth::ProcessMode>(mode));
}

void sfizz_set_sample_quality(sfizz_synth_t* synth, sfizz_process_mode_t mode, int quality)
{
    return synth->synth.setSampleQuality(static_cast<sfz::Synth::ProcessMode>(mode), quality);
}

int sfizz_get_oscillator_quality(sfizz_synth_t* synth, sfizz_process_mode_t mode)
{
    return synth->synth.getOscillatorQuality(static_cast<sfz::Synth::ProcessMode>(mode));
}

void sfizz_set_oscillator_quality(sfizz_synth_t* synth, sfizz_process_mode_t mode, int quality)
{
    return synth->synth.setOscillatorQuality(static_cast<sfz::Synth::ProcessMode>(mode), quality);
}

void sfizz_set_sustain_cancels_release(sfizz_synth_t* synth, bool value)
{
    return synth->synth.setSustainCancelsRelease(value);
}

void sfizz_set_volume(sfizz_synth_t* synth, float volume)
{
    synth->synth.setVolume(volume);
}

float sfizz_get_volume(sfizz_synth_t* synth)
{
    return synth->synth.getVolume();
}

void sfizz_set_num_voices(sfizz_synth_t* synth, int num_voices)
{
    synth->synth.setNumVoices(num_voices);
}

int sfizz_get_num_voices(sfizz_synth_t* synth)
{
    return synth->synth.getNumVoices();
}

int sfizz_get_num_buffers(sfizz_synth_t* synth)
{
    return synth->synth.getAllocatedBuffers();
}

int sfizz_get_num_bytes(sfizz_synth_t* synth)
{
    return synth->synth.getAllocatedBytes();
}

void sfizz_enable_freewheeling(sfizz_synth_t* synth)
{
    synth->synth.enableFreeWheeling();
}

void sfizz_disable_freewheeling(sfizz_synth_t* synth)
{
    synth->synth.disableFreeWheeling();
}

char* sfizz_get_unknown_opcodes(sfizz_synth_t* synth)
{
    const auto unknownOpcodes = synth->synth.getUnknownOpcodes();
    size_t totalLength = 0;
    for (auto& opcode: unknownOpcodes)
        totalLength += opcode.length() + 1;

    if (totalLength == 0)
        return nullptr;

    auto opcodeList = (char *)std::malloc(totalLength);

    auto listIterator = opcodeList;
    for (auto& opcode : unknownOpcodes) {
        std::copy(opcode.begin(), opcode.end(), listIterator);
        listIterator += opcode.length();
        *listIterator++ = (opcode == *unknownOpcodes.rbegin()) ? '\0' : ',';
    }
    return opcodeList;
}

bool sfizz_should_reload_file(sfizz_synth_t* synth)
{
    return synth->synth.shouldReloadFile();
}

bool sfizz_should_reload_scala(sfizz_synth_t* synth)
{
    return synth->synth.shouldReloadScala();
}

void sfizz_enable_logging(sfizz_synth_t* synth, const char* prefix)
{
    (void)synth;
    (void)prefix;
}

void sfizz_set_logging_prefix(sfizz_synth_t* synth, const char* prefix)
{
    (void)synth;
    (void)prefix;
}

void sfizz_disable_logging(sfizz_synth_t* synth)
{
    (void)synth;
}

void sfizz_get_callback_breakdown(sfizz_synth_t* synth, sfizz_callback_breakdown_t* breakdown)
{
    const auto& bd = synth->synth.getCallbackBreakdown();
    breakdown->amplitude = bd.amplitude;
    breakdown->panning = bd.panning;
    breakdown->renderMethod = bd.renderMethod;
    breakdown->data = bd.data;
    breakdown->dispatch = bd.dispatch;
    breakdown->filters = bd.filters;
    breakdown->effects = bd.effects;
}

void sfizz_all_sound_off(sfizz_synth_t* synth)
{
    return synth->synth.allSoundOff();
}

void sfizz_add_external_definitions(sfizz_synth_t* synth, const char* id, const char* value)
{
    synth->synth.addExternalDefinition(id, value);
}

void sfizz_clear_external_definitions(sfizz_synth_t* synth)
{
    synth->synth.clearExternalDefinitions();
}

unsigned int sfizz_get_num_key_labels(sfizz_synth_t* synth)
{
    return synth->synth.getKeyLabels().size();
}

int sfizz_get_key_label_number(sfizz_synth_t* synth, int label_index)
{
    const auto keyLabels = synth->synth.getKeyLabels();
    if (label_index < 0)
        return SFIZZ_OUT_OF_BOUNDS_LABEL_INDEX;

    if (static_cast<unsigned int>(label_index) >= keyLabels.size())
        return SFIZZ_OUT_OF_BOUNDS_LABEL_INDEX;

    // Sanity checks for the future or platforms
    static_assert(
        std::numeric_limits<sfz::NoteNamePair::first_type>::max() < std::numeric_limits<int>::max(),
        "The C API sends back an int but the note index in NoteNamePair can overflow it on this platform"
    );
    return static_cast<int>(keyLabels[label_index].first);
}

const char * sfizz_get_key_label_text(sfizz_synth_t* synth, int label_index)
{
    const auto keyLabels = synth->synth.getKeyLabels();
    if (label_index < 0)
        return NULL;

    if (static_cast<unsigned int>(label_index) >= keyLabels.size())
        return NULL;

    return keyLabels[label_index].second.c_str();
}

unsigned int sfizz_get_num_cc_labels(sfizz_synth_t* synth)
{
    return synth->synth.getCCLabels().size();
}

int sfizz_get_cc_label_number(sfizz_synth_t* synth, int label_index)
{
    const auto ccLabels = synth->synth.getCCLabels();
    if (label_index < 0)
        return SFIZZ_OUT_OF_BOUNDS_LABEL_INDEX;

    if (static_cast<unsigned int>(label_index) >= ccLabels.size())
        return SFIZZ_OUT_OF_BOUNDS_LABEL_INDEX;

    // Sanity checks for the future or platforms
    static_assert(
        std::numeric_limits<sfz::CCNamePair::first_type>::max() < std::numeric_limits<int>::max(),
        "The C API sends back an int but the cc index in CCNamePair can overflow it on this platform"
    );
    return static_cast<int>(ccLabels[label_index].first);
}

const char * sfizz_get_cc_label_text(sfizz_synth_t* synth, int label_index)
{
    const auto ccLabels = synth->synth.getCCLabels();
    if (label_index < 0)
        return NULL;

    if (static_cast<unsigned int>(label_index) >= ccLabels.size())
        return NULL;

    return ccLabels[label_index].second.c_str();
}

void sfizz_free_memory(void* ptr)
{
    free(ptr);
}

sfizz_client_t* sfizz_create_client(void* data)
{
    return reinterpret_cast<sfizz_client_t*>(new sfz::Client(data));
}

void sfizz_delete_client(sfizz_client_t* client)
{
    delete reinterpret_cast<sfz::Client*>(client);
}

void* sfizz_get_client_data(sfizz_client_t* client)
{
    return reinterpret_cast<sfz::Client*>(client)->getClientData();
}

void sfizz_set_receive_callback(sfizz_client_t* client, sfizz_receive_t* receive)
{
    reinterpret_cast<sfz::Client*>(client)->setReceiveCallback(receive);
}

void sfizz_send_message(sfizz_synth_t* synth, sfizz_client_t* client, int delay, const char* path, const char* sig, const sfizz_arg_t* args)
{
    synth->synth.dispatchMessage(*reinterpret_cast<sfz::Client*>(client), delay, path, sig, args);
}

void sfizz_set_broadcast_callback(sfizz_synth_t* synth, sfizz_receive_t* broadcast, void* data)
{
    synth->synth.setBroadcastCallback(broadcast, data);
}

#ifdef __cplusplus
}
#endif
