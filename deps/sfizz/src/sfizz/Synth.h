// SPDX-License-Identifier: BSD-2-Clause

// This code is part of the sfizz library and is licensed under a BSD 2-clause
// license. You should have receive a LICENSE.md file along with the code.
// If not, contact the sfizz maintainers at https://github.com/sfztools/sfizz

#pragma once
#include "AudioSpan.h"
#include "Resources.h"
#include "Messaging.h"
#include "utility/NumericId.h"
#include "utility/LeakDetector.h"
#include <ghc/fs_std.hpp>
#include <absl/strings/string_view.h>
#include <memory>
#include <bitset>
#include <string>
#include <vector>
template <size_t> class BitArray;

namespace sfz {

// Forward declarations for the introspection methods
class Parser;
class RegionSet;
class PolyphonyGroup;
class EffectBus;
struct Region;
struct Layer;
class Voice;

using CCNamePair = std::pair<uint16_t, std::string>;
using NoteNamePair = std::pair<uint8_t, std::string>;

/**
 * @brief This class is the core of the sfizz library. In C++ it is the main point
 * of entry and in C the interface basically maps the functions of the class into
 * C bindings.
 *
 * The JACK client provides an example of how you can use this class as an entry
 * point for your own projects. Just include this header and compile against the
 * static library. If you wish to use the shared library you should rather use the
 * C bindings.
 *
 * This class derives from the Parser and provides a specific set of callbacks; see
 * the Parser documentation for more precisions.
 *
 * The Synth object contains:
 * - A set of SFZ Regions that get filled up upon parsing
 * - A set of Voices that play the sounds of the regions when triggered.
 * - Some singleton resources, particularly the midiState which contains the current
 *      midi status (note is on or off, last note velocity, current CC values, ...)
 *      as well as a FilePool that preloads and give access to files.
 *
 * The synth is callback based, in the sense that it renders audio block by block
 * using the renderBlock() function. Between each call to renderBlock() you have to
 * send the relevent events for the block in the form of MIDI events: noteOn(),
 * noteOff(), cc(). You can also send pitchBend(), aftertouch() and bpm()
 * events -- but as of 2019 they are not handled.
 *
 * All events have a delay information, which must be less than the size of the
 * next call to renderBlock() in units of frames or samples. For example, if you
 * will call to render a block of 256 samples, all the events you send to the
 * synth should have a delay parameter strictly lower than 256. Events beyond 256
 * may be completely ignored by the synth as the incoming event buffer is cleared
 * during the renderBlock() call. You SHOULD also feed the midi events in the correct
 * order.
 *
 * The jack_client.cpp file contains examples of the most classical usage of the
 * synth and can be used as a reference.
 */
class Synth final {
public:
    /**
     * @brief Construct a new Synth object with a default number of voices.
     *
     */
    Synth();
    /**
     * @brief Destructor
     */
    ~Synth();
    Synth(const Synth& other) = delete;
    Synth& operator=(const Synth& other) = delete;
    Synth(Synth&& other) = delete;
    Synth& operator=(Synth&& other) = delete;
    /**
     * @brief Processing mode
     */
    enum ProcessMode {
        ProcessLive,
        ProcessFreewheeling,
    };

    /**
     * @brief Empties the current regions and load a new SFZ file into the synth.
     *
     * This function will disable all callbacks so it is safe to call from a
     * UI thread for example, although it may generate a click. However it is
     * not reentrant, so you should not call it from concurrent threads.
     *
     * @param file
     * @return true
     * @return false if the file was not found or no regions were loaded.
     */
    bool loadSfzFile(const fs::path& file);
    /**
     * @brief Empties the current regions and load a new SFZ document from memory.
     *
     * This is similar to loadSfzFile() in functionality.
     * This accepts a virtual path name for the imaginary sfz file, which is not
     * required to exist on disk. The purpose of the virtual path is to locate
     * samples with relative paths.
     *
     * @param path The virtual path of the SFZ file, as string.
     * @param text The contents of the virtual SFZ file.
     *
     * @return @false if no regions were loaded,
     *         @true otherwise.
     */
    bool loadSfzString(const fs::path& path, absl::string_view text);
    /**
     * @brief Sets the tuning from a Scala file loaded from the file system.
     *
     * @param  path   The path to the file in Scala format.
     * @return @true when tuning scale loaded OK,
     *         @false if some error occurred.
     */
    bool loadScalaFile(const fs::path& path);
    /**
     * @brief Sets the tuning from a Scala file loaded from memory.
     *
     * @param  text   The contents of the file in Scala format.
     * @return @true when tuning scale loaded OK,
     *         @false if some error occurred.
     */
    bool loadScalaString(const std::string& text);
    /**
     * @brief Sets the scala root key.
     *
     * @param rootKey The MIDI number of the Scala root key (default 60 for C4).
     */
    void setScalaRootKey(int rootKey);
    /**
     * @brief Gets the scala root key.
     *
     * @return The MIDI number of the Scala root key (default 60 for C4).
     */
    int getScalaRootKey() const;
    /**
     * @brief Sets the reference tuning frequency.
     *
     * @param frequency The frequency which indicates where standard tuning A4 is (default 440 Hz).
     */
    void setTuningFrequency(float frequency);
    /**
     * @brief Gets the reference tuning frequency.
     *
     * @return The frequency which indicates where standard tuning A4 is (default 440 Hz).
     */
    float getTuningFrequency() const;
    /**
     * @brief Configure stretch tuning using a predefined parametric Railsback curve.
     *
     * @param ratio The parameter in domain 0-1.
     */
    void loadStretchTuningByRatio(float ratio);
    /**
     * @brief Get the current number of regions loaded
     *
     * @return int
     */
    int getNumRegions() const noexcept;
    /**
     * @brief Get the current number of groups loaded
     *
     * @return int
     */
    int getNumGroups() const noexcept;
    /**
     * @brief Get the current number of masters loaded
     *
     * @return int
     */
    int getNumMasters() const noexcept;
    /**
     * @brief Get the current number of curves loaded
     *
     * @return int
     */
    int getNumCurves() const noexcept;
    /**
     * @brief Export a MIDI Name document describing the loaded instrument
     */
    std::string exportMidnam(absl::string_view model = {}) const;
    /**
     * @brief Find the layer which is associated with the given identifier.
     *
     * @param id
     * @return Layer*
     */
    Layer* getLayerById(NumericId<Region> id) noexcept;
    /**
     * @brief Find the region which is associated with the given identifier.
     *
     * @param id
     * @return const Region*
     */
    const Region* getRegionById(NumericId<Region> id) const noexcept;
    /**
     * @brief Get a raw view into a specific layer. This is mostly used
     * for testing.
     *
     * @param idx
     * @return const Layer*
     */
    const Layer* getLayerView(int idx) const noexcept;
    /**
     * @brief Get a raw view into a specific region. This is mostly used
     * for testing.
     *
     * @param idx
     * @return const Region*
     */
    const Region* getRegionView(int idx) const noexcept;
    /**
     * @brief Get a raw view into a specific voice. This is mostly used
     * for testing.
     *
     * @param idx
     * @return const Voice*
     */
    const Voice* getVoiceView(int idx) const noexcept;
    /**
     * @brief Get a raw view into a specific effect bus. This is mostly used
     * for testing.
     * You'll need to include "Effects.h" to resolve the forward declaration.
     *
     * @param idx
     * @param output
     * @return const EffectBus*
     */
    const EffectBus* getEffectBusView(int idx, int output = 0) const noexcept;
    /**
     * @brief Get a raw view into a specific set of regions. This is mostly used
     * for testing.
     * You'll need to include "RegionSet.h" to resolve the forward declaration.
     *
     * @param idx
     * @return const RegionSet*
     */
    const RegionSet* getRegionSetView(int idx) const noexcept;
    /**
     * @brief Get a raw view into a specific polyphony group. This is mostly used
     * for testing.
     * You'll need to include "PolyphonyGroup.h" to resolve the forward declaration.
     *
     * @param idx
     * @return const PolyphonyGroup*
     */
    const PolyphonyGroup* getPolyphonyGroupView(int idx) const noexcept;
    /**
     * @brief Get the number of polyphony groups
     *
     * @return unsigned
     */
    unsigned getNumPolyphonyGroups() const noexcept;
    /**
     * @brief Get a list of unknown opcodes. The lifetime of the
     * string views in the code are linked to the currently loaded
     * sfz file.
     *
     * TODO: change this to strings we don't really care about performance
     * here and this hurts the C interface.
     *
     * @return std::set<absl::string_view>
     */
    const std::vector<std::string>& getUnknownOpcodes() const noexcept;
    /**
     * @brief Get the number of preloaded samples in the synth
     *
     * @return size_t
     */
    size_t getNumPreloadedSamples() const noexcept;

    /**
     * @brief Set the maximum size of the blocks for the callback. The actual
     * size can be lower in each callback but should not be larger
     * than this value.
     *
     * @param samplesPerBlock
     */
    void setSamplesPerBlock(int samplesPerBlock) noexcept;
    /**
     * @brief Get the maximum size of the blocks for the callback. The actual
     * size can be lower in each callback but should not be larger
     * than this value.
     */
    int getSamplesPerBlock() const noexcept;
    /**
     * @brief Set the sample rate. If you do not call it it is initialized
     * to sfz::config::defaultSampleRate.
     *
     * @param sampleRate
     */
    void setSampleRate(float sampleRate) noexcept;
    /**
     * @brief Get the default resampling quality for the given mode.
     *
     * @param mode the processing mode
     *
     * @return the quality setting
     */
    int getSampleQuality(ProcessMode mode);
    /**
     * @brief Set the default resampling quality for the given mode.
     *
     * @param mode the processing mode
     * @param quality the quality setting
     */
    void setSampleQuality(ProcessMode mode, int quality);
    /**
     * @brief Get the default oscillator quality for the given mode.
     *
     * @param mode the processing mode
     *
     * @return the quality setting
     */
    int getOscillatorQuality(ProcessMode mode);
    /**
     * @brief Set the default oscillator quality for the given mode.
     *
     * @param mode the processing mode
     * @param quality the quality setting
     */
    void setOscillatorQuality(ProcessMode mode, int quality);
    /**
     * @brief Set whether pressing the sustain pedal cancels the releases
     *
     * @param value
     */
    void setSustainCancelsRelease(bool value);
    /**
     * @brief Get the current value for the volume, in dB.
     *
     * @return float
     */
    float getVolume() const noexcept;
    /**
     * @brief Set the value for the volume. This value will be
     * clamped within sfz::default::volumeRange.
     *
     * @param volume
     */
    void setVolume(float volume) noexcept;

    /**
     * @brief Send a note on event to the synth
     *
     * @param delay the delay at which the event occurs; this should be lower
     *              than the size of the block in the next call to renderBlock().
     * @param noteNumber the midi note number
     * @param velocity the midi note velocity
     */
    void noteOn(int delay, int noteNumber, int velocity) noexcept;
    /**
     * @brief Send a high-precision note on event to the synth
     *
     * @param delay the delay at which the event occurs; this should be lower
     *              than the size of the block in the next call to renderBlock().
     * @param noteNumber the midi note number
     * @param velocity the normalized midi note velocity, in domain 0 to 1
     */
    void hdNoteOn(int delay, int noteNumber, float velocity) noexcept;
    /**
     * @brief Send a note off event to the synth
     *
     * @param delay the delay at which the event occurs; this should be lower
     *              than the size of the block in the next call to renderBlock().
     * @param noteNumber the midi note number
     * @param velocity the midi note velocity
     */
    void noteOff(int delay, int noteNumber, int velocity) noexcept;
    /**
     * @brief Send a high-precision note off event to the synth
     *
     * @param delay the delay at which the event occurs; this should be lower
     *              than the size of the block in the next call to renderBlock().
     * @param noteNumber the midi note number
     * @param velocity the normalized midi note velocity, in domain 0 to 1
     */
    void hdNoteOff(int delay, int noteNumber, float velocity) noexcept;
    /**
     * @brief Send a CC event to the synth
     *
     * @param delay the delay at which the event occurs; this should be lower than the size of
     *              the block in the next call to renderBlock().
     * @param ccNumber the cc number
     * @param ccValue the cc value
     */
    void cc(int delay, int ccNumber, int ccValue) noexcept;
    /**
     * @brief Send a high precision CC event to the synth
     *
     * @param delay the delay at which the event occurs; this should be lower than the size of
     *              the block in the next call to renderBlock().
     * @param ccNumber the cc number
     * @param normValue the normalized cc value, in domain 0 to 1
     */
    void hdcc(int delay, int ccNumber, float normValue) noexcept;
    /**
     * @brief Send a program change event to the synth
     *
     * @param delay the delay at which the event occurs; this should be lower than the size of
     *              the block in the next call to renderBlock().
     * @param ccNumber the program number
     */
    void programChange(int delay, int program) noexcept;
    /**
     * @brief Send a high precision CC automation to the synth
     *
     * @param delay the delay at which the event occurs; this should be lower
     *              than the size of the block in the next call to renderBlock().
     * @param ccNumber the cc number.
     * @param normValue the normalized cc value, in domain 0 to 1.
     */
    void automateHdcc(int delay, int ccNumber, float normValue) noexcept;
    /**
     * @brief Get the current value of a controller under the current instrument
     *
     * @param ccNumber the cc number
     * @return the current value
     */
    float getHdcc(int ccNumber);
    /**
     * @brief Get the default value of a controller under the current instrument
     *
     * @param ccNumber the cc number
     * @return the default value
     */
    float getDefaultHdcc(int ccNumber);
   /**
     * @brief Send a pitch bend event to the synth
     *
     * @param delay the delay at which the event occurs; this should be lower
     *              than the size of the block in the next call to
     *              renderBlock().
     * @param pitch the pitch value centered between -8192 and 8192
     */
    void pitchWheel(int delay, int pitch) noexcept;
   /**
     * @brief Send a high-precision pitch bend event to the synth
     *
     * @param delay the delay at which the event occurs; this should be lower
     *              than the size of the block in the next call to
     *              renderBlock().
     * @param pitch the normalized pitch value centered between -1 and 1
     */
    void hdPitchWheel(int delay, float pitch) noexcept;
    /**
     * @brief Send a aftertouch event to the synth
     *
     * @param delay the delay at which the event occurs; this should be lower than the size of
     *              the block in the next call to renderBlock().
     * @param aftertouch the aftertouch value
     */
    void channelAftertouch(int delay, int aftertouch) noexcept;
    /**
     * @brief Send a high precision aftertouch event to the synth
     *
     * @param delay the delay at which the event occurs; this should be lower than the size of
     *              the block in the next call to renderBlock().
     * @param normAftertouch the normalized aftertouch value, in domain 0 to 1
     */
    void hdChannelAftertouch(int delay, float normAftertouch) noexcept;
    /**
     * @brief Send a tempo event to the synth
     *
     * @param delay the delay at which the event occurs; this should be lower than the size of
     *              the block in the next call to renderBlock(), and ordered with respect to
     *              calls to tempo(), timeSignature(), timePosition(), and playbackState().
     * @param secondsPerQuarter the new period of the quarter note
     */
    void tempo(int delay, float secondsPerQuarter) noexcept;
    /**
     * @brief Send a tempo event to the synth
     *
     * @param delay the delay at which the event occurs; this should be lower than the size of
     *              the block in the next call to renderBlock(), and ordered with respect to
     *              calls to tempo(), timeSignature(), timePosition(), and playbackState().
     * @param beatsPerMinute the new tempo, in beats per minute
     */
    void bpmTempo(int delay, float beatsPerMinute) noexcept;
    /**
     * @brief Send a polyphonic aftertouch event to the synth
     *
     * @param delay
     * @param noteNumber
     * @param normAftertouch
     */
    void polyAftertouch(int delay, int noteNumber, int aftertouch) noexcept;
    /**
     * @brief Send a polyphonic aftertouch event to the synth
     *
     * @param delay
     * @param noteNumber
     * @param normAftertouch
     */
    void hdPolyAftertouch(int delay, int noteNumber, float normAftertouch) noexcept;
    /**
     * @brief       Send the time signature.
     *
     * @param delay the delay at which the event occurs; this should be lower than the size of
     *              the block in the next call to renderBlock(), and ordered with respect to
     *              calls to tempo(), timeSignature(), timePosition(), and playbackState().
     * @param beats_per_bar The number of beats per bar, or time signature numerator.
     * @param beat_unit The note corresponding to one beat, or time signature denominator.
     */
    void timeSignature(int delay, int beatsPerBar, int beatUnit);
    /**
     * @brief Send the time position.
     *
     * @param delay the delay at which the event occurs; this should be lower than the size of
     *              the block in the next call to renderBlock(), and ordered with respect to
     *              calls to tempo(), timeSignature(), timePosition(), and playbackState().
     * @param bar The current bar.
     * @param bar_beat The fractional position of the current beat within the bar.
     */
    void timePosition(int delay, int bar, double barBeat);
    /**
     * @brief Send the playback state.
     *
     * @param delay the delay at which the event occurs; this should be lower than the size of
     *              the block in the next call to renderBlock(), and ordered with respect to
     *              calls to tempo(), timeSignature(), timePosition(), and playbackState().
     * @param playback_state The playback state, 1 if playing, 0 if stopped.
     */
    void playbackState(int delay, int playbackState);

    /**
     * @brief Render an block of audio data in the buffer. This call will reset
     * the synth in its waiting state for the next batch of events. The size of
     * the block is integrated in the AudioSpan object. You can build an
     * AudioSpan implicitely from a large number of source objects; check the
     * AudioSpan reference for more precision.
     *
     * @param buffer the buffer to write the next block into; this should be a
     * stereo buffer.
     */
    void renderBlock(AudioSpan<float> buffer) noexcept;

    /**
     * @brief Get the number of active voices
     *
     * @return int
     */
    int getNumActiveVoices() const noexcept;
    /**
     * @brief Get the total number of voices in the synth (the polyphony)
     *
     * @return int
     */
    int getNumVoices() const noexcept;
    /**
     * @brief Change the number of voices (the polyphony).
     * This function takes a lock and disables the callback; prefer calling
     * it out of the RT thread. It can also take a long time to return.
     * If the new number of voices is the same as the current one, it will
     * release the lock immediately and exit.
     *
     * @param numVoices
     */
    void setNumVoices(int numVoices) noexcept;

    /**
     * @brief Set the preloaded file size.
     * This function takes a lock and disables the callback; prefer calling
     * it out of the RT thread. It can also take a long time to return.
     * If the new preload size is the same as the current one, it will
     * release the lock immediately and exit.
     *
     * @param factor
     */
    void setPreloadSize(uint32_t preloadSize) noexcept;

    /**
     * @brief get the current preloaded file size
     *
     * @return Oversampling
     */
    uint32_t getPreloadSize() const noexcept;

    /**
     * @brief Gets the number of allocated buffers.
     *
     * @return  The allocated buffers.
     */
    int getAllocatedBuffers() const noexcept { return Buffer<float>::counter().getNumBuffers(); }

    /**
     * @brief Gets the number of bytes allocated through the buffers
     *
     * @return  The allocated bytes.
     */
    int getAllocatedBytes() const noexcept { return Buffer<float>::counter().getTotalBytes(); }

    /**
     * @brief Enable freewheeling on the synth. This will wait for background
     * loaded files to finish loading before each render callback to ensure that
     * there will be no dropouts.
     *
     */
    void enableFreeWheeling() noexcept;
    /**
     * @brief Disable freewheeling on the synth. You should disable freewheeling
     * before live use of the plugin otherwise the audio thread will lock.
     *
     */
    void disableFreeWheeling() noexcept;

    Resources& getResources() noexcept;
    const Resources& getResources() const noexcept;

    /**
     * @brief Check if the SFZ should be reloaded.
     *
     * Depending on the platform this can create file descriptors.
     *
     * @return true if any included files (including the root file) have
     *              been modified since the sfz file was loaded.
     * @return false
     */
    bool shouldReloadFile();

    /**
     * @brief Check if the tuning (scala) file should be reloaded.
     *
     * Depending on the platform this can create file descriptors.
     *
     * @return true if a scala file has been loaded and has changed
     * @return false
     */
    bool shouldReloadScala();

    struct CallbackBreakdown
    {
        double dispatch { 0 };
        double renderMethod { 0 };
        double data { 0 };
        double amplitude { 0 };
        double filters { 0 };
        double panning { 0 };
        double effects { 0 };
    };
    /**
     * @brief View the callback breakdown for the last frame.
     * Call from the real-time thread after a renderBlock call.
     *
     * @return const CallbackBreakdown&
     */
    const CallbackBreakdown& getCallbackBreakdown() const noexcept;

    /**
     * @brief Shuts down the current processing, clear buffers and reset the voices.
     *
     */
    void allSoundOff() noexcept;

    /**
     * @brief Add external definitions prior to loading.
     *
     */
    void addExternalDefinition(const std::string& id, const std::string& value);

    /**
     * @brief Clears external definitions for the next file loading.
     *
     */
    void clearExternalDefinitions();

    /**
     * @brief Get the parser.
     *
     * @return    A reference to the parser.
     */
    Parser& getParser() noexcept;
    /**
     * @brief Get the parser.
     *
     * @return    A reference to the parser.
     */
    const Parser& getParser() const noexcept;

    /**
     * @brief Get the key labels, if any
     *
     * @return const std::vector<NoteNamePair>&
     */
    const std::vector<NoteNamePair>& getKeyLabels() const noexcept;
    /**
     * @brief Get the CC labels, if any
     *
     * @return const std::vector<NoteNamePair>&
     */
    const std::vector<CCNamePair>& getCCLabels() const noexcept;

    /**
     * @brief Get the used CCs
     *
     * @return const std::bitset<config::numCCs>&
     */
    const BitArray<config::numCCs>& getUsedCCs() const noexcept;

    /**
     * @brief Dispatch the incoming message to the synth engine
     * @since 1.0.0
     *
     * @param client       The client sending the message.
     * @param delay        The delay of the message in the block, in samples.
     * @param path         The OSC address pattern.
     * @param sig          The OSC type tag string.
     * @param args         The OSC arguments, whose number and format is determined the type tag string.
     */
    void dispatchMessage(Client& client, int delay, const char* path, const char* sig, const sfizz_arg_t* args);

    /**
     * @brief Set the function which receives broadcast messages from the synth engine.
     * @since 1.0.0
     *
     * @param broadcast    The pointer to the receiving function.
     * @param data         The opaque data pointer which is passed to the receiver.
     */
    void setBroadcastCallback(sfizz_receive_t* broadcast, void* data);

private:
    struct Impl;
    std::unique_ptr<Impl> impl_;

    LEAK_DETECTOR(Synth);
};

}
