// SPDX-License-Identifier: BSD-2-Clause

// This code is part of the sfizz library and is licensed under a BSD 2-clause
// license. You should have receive a LICENSE.md file along with the code.
// If not, contact the sfizz maintainers at https://github.com/sfztools/sfizz

/**
  @file
  @brief sfizz public C++ API.
*/

#pragma once
#include "sfizz_message.h"
#include <string>
#include <utility>
#include <vector>
#include <memory>

#if defined SFIZZ_EXPORT_SYMBOLS
  #if defined _WIN32
    #define SFIZZ_EXPORTED_API __declspec(dllexport)
  #else
    #define SFIZZ_EXPORTED_API __attribute__ ((visibility ("default")))
  #endif
#else
  #define SFIZZ_EXPORTED_API
#endif

//! @cond Doxygen_Suppress
#if defined _WIN32
  #define SFIZZ_DEPRECATED_API __declspec(deprecated)
#else
  #define SFIZZ_DEPRECATED_API __attribute__ ((deprecated))
#endif
//! @endcond

struct sfizz_synth_t;

namespace sfz
{
class Client;
/**
 * @brief Synthesizer for SFZ instruments
*
* The synthesizer must be operated under indicated constraints in order to
* guarantee thread-safety.
*
* At any given time, no more than 2 tasks must interact in parallel with this
* library:
* - a processing tasks @b RT for audio and MIDI, which can be real-time
* - a Control tasks @b CT
*
* The tasks RT and CT can be assumed by different threads over the lifetime, as
* long as the switch is adequately synchronized. If real-time processing is not
* required, it's acceptable for the 2 tasks can be assumed by a single thread.
*
* Where one or more following items are indicated on a function, the constraints apply.
* - @b RT: the function must be invoked from the Real-time thread
* - @b CT: the function must be invoked from the Control thread
* - @b OFF: the function cannot be invoked while a thread is calling @b RT functions
*/
class SFIZZ_EXPORTED_API Sfizz
{
public:
    /**
     * @brief Construct a new Sfizz object.
     *
     * The synth by default is set at 48 kHz and a block size of 1024.
     * You should change these values if they are not suited to your application.
     */
    Sfizz();
    ~Sfizz();

    /**
     * @brief Move constructor.
     */
    Sfizz(Sfizz&& other) noexcept;

    /**
     * @brief Move assignment operator.
     */
    Sfizz& operator=(Sfizz&& other) noexcept;

    Sfizz(const Sfizz& other) = delete;
    Sfizz& operator=(const Sfizz& other) = delete;

    /**
     * @brief Reference an existing synth handle.
     */
    explicit Sfizz(sfizz_synth_t* synth);

    /**
     * @brief Get the synth handle.
     */
    sfizz_synth_t* handle() const noexcept { return synth; }

    /**
     * @brief Processing mode.
     * @since 0.4.0
     */
    enum ProcessMode {
        ProcessLive,
        ProcessFreewheeling,
    };

    /**
     * @brief Empties the current regions and load a new SFZ file into the synth.
     *
     * @since 0.2.0
     *
     * @param path The path to the file to load, as string.
     *
     * @return @false if the file was not found or no regions were loaded,
     *         @true otherwise.
     *
     * @par Thread-safety constraints
     * - @b CT: the function must be invoked from the Control thread
     * - @b OFF: the function cannot be invoked while a thread is calling @b RT functions
     */
    bool loadSfzFile(const std::string& path);

    /**
     * @brief Empties the current regions and load a new SFZ document from memory.
     *
     * This is similar to loadSfzFile() in functionality.
     * This accepts a virtual path name for the imaginary sfz file, which is not
     * required to exist on disk. The purpose of the virtual path is to locate
     * samples with relative paths.
     *
     * @since 0.4.0
     *
     * @param path The virtual path of the SFZ file, as string.
     * @param text The contents of the virtual SFZ file.
     *
     * @return @false if no regions were loaded,
     *         @true otherwise.
     *
     * @par Thread-safety constraints
     * - @b CT: the function must be invoked from the Control thread
     * - @b OFF: the function cannot be invoked while a thread is calling @b RT functions
     */
    bool loadSfzString(const std::string& path, const std::string& text);

    /**
     * @brief Sets the tuning from a Scala file loaded from the file system.
     *
     * @since 0.4.0
     *
     * @param path The path to the file in Scala format.
     *
     * @return @true when tuning scale loaded OK,
     *         @false if some error occurred.
     *
     * @par Thread-safety constraints
     * - @b CT: the function must be invoked from the Control thread
     * - @b OFF: the function cannot be invoked while a thread is calling @b RT functions
     */
    bool loadScalaFile(const std::string& path);

    /**
     * @brief Sets the tuning from a Scala file loaded from memory.
     *
     * @since 0.4.0
     *
     * @param text The contents of the file in Scala format.
     *
     * @return @true when tuning scale loaded OK,
     *         @false if some error occurred.
     *
     * @par Thread-safety constraints
     * - @b CT: the function must be invoked from the Control thread
     * - @b OFF: the function cannot be invoked while a thread is calling @b RT functions
     */
    bool loadScalaString(const std::string& text);

    /**
     * @brief Sets the scala root key.
     *
     * @since 0.4.0
     *
     * @param rootKey The MIDI number of the Scala root key (default 60 for C4).
     *
     * @par Thread-safety constraints
     * - @b RT: the function must be invoked from the Real-time thread
     */
    void setScalaRootKey(int rootKey);

    /**
     * @brief Gets the scala root key.
     * @since 0.4.0
     *
     * @return The MIDI number of the Scala root key (default 60 for C4).
     */
    int getScalaRootKey() const;

    /**
     * @brief Sets the reference tuning frequency.
     *
     * @since 0.4.0
     *
     * @param frequency The frequency which indicates where standard tuning A4 is (default 440 Hz).
     *
     * @par Thread-safety constraints
     * - @b RT: the function must be invoked from the Real-time thread
     */
    void setTuningFrequency(float frequency);

    /**
     * @brief Gets the reference tuning frequency.
     * @since 0.4.0
     *
     * @return The frequency which indicates where standard tuning A4 is (default 440 Hz).
     */
    float getTuningFrequency() const;

    /**
     * @brief Configure stretch tuning using a predefined parametric Railsback curve.
     *
     * A ratio 1/2 is supposed to match the average piano; 0 disables (the default).
     *
     * @since 0.4.0
     *
     * @param ratio The parameter in domain 0-1.
     *
     * @par Thread-safety constraints
     * - @b RT: the function must be invoked from the Real-time thread
     */
    void loadStretchTuningByRatio(float ratio);

    /**
     * @brief Return the current number of regions loaded.
     * @since 0.2.0
     */
    int getNumRegions() const noexcept;

    /**
     * @brief Return the current number of groups loaded.
     * @since 0.2.0
     */
    int getNumGroups() const noexcept;

    /**
     * @brief Return the current number of masters loaded.
     * @since 0.2.0
     */
    int getNumMasters() const noexcept;

    /**
     * @brief Return the current number of curves loaded.
     * @since 0.2.0
     */
    int getNumCurves() const noexcept;

    /**
     * @brief Return a list of unsupported opcodes, if any.
     * @since 0.2.0
     */
    const std::vector<std::string>& getUnknownOpcodes() const noexcept;

    /**
     * @brief Return the number of preloaded samples in the synth.
     * @since 0.2.0
     */
    size_t getNumPreloadedSamples() const noexcept;

    /**
     * @brief Set the maximum size of the blocks for the callback.
     *
     * The actual size can be lower in each callback but should not be larger
     * than this value.
     *
     * @since 0.2.0
     *
     * @param samplesPerBlock The number of samples per block.
     *
     * @par Thread-safety constraints
     * - @b CT: the function must be invoked from the Control thread
     * - @b OFF: the function cannot be invoked while a thread is calling @b RT functions
     */
    void setSamplesPerBlock(int samplesPerBlock) noexcept;

    /**
     * @brief Set the sample rate.
     *
     * If you do not call it it is initialized to `sfz::config::defaultSampleRate`.
     *
     * @since 0.2.0
     *
     * @param sampleRate The sample rate.
     *
     * @par Thread-safety constraints
     * - @b CT: the function must be invoked from the Control thread
     * - @b OFF: the function cannot be invoked while a thread is calling @b RT functions
     */
    void setSampleRate(float sampleRate) noexcept;

    /**
     * @brief Get the default resampling quality.
     *
     * This is the quality setting which the engine uses when the instrument
     * does not use the opcode `sample_quality`. The engine uses distinct
     * default quality settings for live mode and freewheeling mode,
     * which both can be accessed by the means of this function.
     * @since 0.4.0
     *
     * @param[in] mode  The processing mode.
     *
     * @return The sample quality for the given mode, in the range 0 to 10.
     */
    int getSampleQuality(ProcessMode mode);

    /**
     * @brief Set the default resampling quality.
     *
     * This is the quality setting which the engine uses when the instrument
     * does not use the opcode `sample_quality`. The engine uses distinct
     * default quality settings for live mode and freewheeling mode,
     * which both can be accessed by the means of this function.
     *
     * @since 0.4.0
     *
     * @param[in] mode    The processing mode.
     * @param[in] quality The desired sample quality, in the range 0 to 10.
     *
     * @par Thread-safety constraints
     * - @b RT: the function must be invoked from the Real-time thread
     */
    void setSampleQuality(ProcessMode mode, int quality);

    /**
     * @brief Get the default oscillator quality.
     *
     * This is the quality setting which the engine uses when the instrument
     * does not use the opcode `oscillator_quality`. The engine uses distinct
     * default quality settings for live mode and freewheeling mode,
     * which both can be accessed by the means of this function.
     * @since 1.0.0
     *
     * @param[in] mode  The processing mode.
     *
     * @return The oscillator quality for the given mode, in the range 0 to 3.
     */
    int getOscillatorQuality(ProcessMode mode);

    /**
     * @brief Set the default oscillator quality.
     *
     * This is the quality setting which the engine uses when the instrument
     * does not use the opcode `oscillator_quality`. The engine uses distinct
     * default quality settings for live mode and freewheeling mode,
     * which both can be accessed by the means of this function.
     *
     * @since 1.0.0
     *
     * @param[in] mode    The processing mode.
     * @param[in] quality The desired oscillator quality, in the range 0 to 3.
     *
     * @par Thread-safety constraints
     * - @b RT: the function must be invoked from the Real-time thread
     */
    void setOscillatorQuality(ProcessMode mode, int quality);

    /**
     * @brief Set whether pressing the sustain pedal cancels the release stage
     *
     * @since 1.2.0
     *
     * @param value
     */
    void setSustainCancelsRelease(bool value);

    /**
     * @brief Return the current value for the volume, in dB.
     * @since 0.2.0
     */
    float getVolume() const noexcept;

     /**
     * @brief Set the value for the volume.
     *
     * This value will be clamped within `sfz::default::volumeRange`.
     *
     * @since 0.2.0
     *
     * @param volume The new volume.
     *
     * @par Thread-safety constraints
     * - @b RT: the function must be invoked from the Real-time thread
     */
    void setVolume(float volume) noexcept;

    /**
     * @brief Send a note on event to the synth.
     * @since 0.2.0
     *
     * This command should be delay-ordered with all other midi-type events
     * (notes, CCs, aftertouch and pitch-wheel), otherwise the behavior of the
     * synth is undefined.
     *
     * @param delay the delay at which the event occurs; this should be lower
     *              than the size of the block in the next call to renderBlock().
     * @param noteNumber the midi note number, in domain 0 to 127.
     * @param velocity the midi note velocity, in domain 0 to 127.
     *
     * @par Thread-safety constraints
     * - @b RT: the function must be invoked from the Real-time thread
     */
    void noteOn(int delay, int noteNumber, int velocity) noexcept;

    /**
     * @brief Send a high-precision note on event to the synth.
     * @since 1.0.0
     *
     * This command should be delay-ordered with all other midi-type events
     * (notes, CCs, aftertouch and pitch-wheel), otherwise the behavior of the
     * synth is undefined.
     *
     * @param delay the delay at which the event occurs; this should be lower
     *              than the size of the block in the next call to renderBlock().
     * @param noteNumber the midi note number, in domain 0 to 127.
     * @param velocity the normalized midi note velocity, in domain 0 to 1.
     *
     * @par Thread-safety constraints
     * - @b RT: the function must be invoked from the Real-time thread
     */
    void hdNoteOn(int delay, int noteNumber, float velocity) noexcept;

    /**
     * @brief Send a note off event to the synth.
     * @since 0.2.0
     *
     * This command should be delay-ordered with all other midi-type events
     * (notes, CCs, aftertouch and pitch-wheel), otherwise the behavior of the
     * synth is undefined.
     *
     * @param delay the delay at which the event occurs; this should be lower
     *              than the size of the block in the next call to renderBlock().
     * @param noteNumber the midi note number, in domain 0 to 127.
     * @param velocity the midi note velocity, in domain 0 to 127.
     *
     * @par Thread-safety constraints
     * - @b RT: the function must be invoked from the Real-time thread
     */
    void noteOff(int delay, int noteNumber, int velocity) noexcept;

    /**
     * @brief Send a note off event to the synth.
     * @since 1.0.0
     *
     * This command should be delay-ordered with all other midi-type events
     * (notes, CCs, aftertouch and pitch-wheel), otherwise the behavior of the
     * synth is undefined.
     *
     * @param delay the delay at which the event occurs; this should be lower
     *              than the size of the block in the next call to renderBlock().
     * @param noteNumber the midi note number, in domain 0 to 127.
     * @param velocity the normalized midi note velocity, in domain 0 to 1.
     *
     * @par Thread-safety constraints
     * - @b RT: the function must be invoked from the Real-time thread
     */
    void hdNoteOff(int delay, int noteNumber, float velocity) noexcept;

    /**
     * @brief Send a CC event to the synth
     * @since 0.2.0
     *
     * This command should be delay-ordered with all other midi-type events
     * (notes, CCs, aftertouch and pitch-wheel), otherwise the behavior of the
     * synth is undefined.
     *
     * @param delay the delay at which the event occurs; this should be lower
     *              than the size of the block in the next call to renderBlock().
     * @param ccNumber the cc number, in domain 0 to 127.
     * @param ccValue the cc value, in domain 0 to 127.
     *
     * @par Thread-safety constraints
     * - @b RT: the function must be invoked from the Real-time thread
     */
    void cc(int delay, int ccNumber, int ccValue) noexcept;

    /**
     * @brief Send a high precision CC event to the synth
     * @since 0.4.0
     *
     * This command should be delay-ordered with all other midi-type events
     * (notes, CCs, aftertouch and pitch-wheel), otherwise the behavior of the
     * synth is undefined.
     *
     * @param delay the delay at which the event occurs; this should be lower
     *              than the size of the block in the next call to renderBlock().
     * @param ccNumber the cc number, in domain 0 to 127.
     * @param normValue the normalized cc value, in domain 0 to 1.
     *
     * @par Thread-safety constraints
     * - @b RT: the function must be invoked from the Real-time thread
     */
    void hdcc(int delay, int ccNumber, float normValue) noexcept;

    /**
     * @brief Send a program change event to the synth
     * @since 1.2.0
     *
     * This command should be delay-ordered with all other midi-type events
     * (notes, CCs, aftertouch and pitch-wheel), otherwise the behavior of the
     * synth is undefined.
     *
     * @param delay the delay at which the event occurs; this should be lower
     *              than the size of the block in the next call to renderBlock().
     * @param program the cc number, in domain 0 to 127.
     *
     * @par Thread-safety constraints
     * - @b RT: the function must be invoked from the Real-time thread
     */
    void programChange(int delay, int program) noexcept;

    /**
     * @brief Send a high precision CC automation to the synth
     *
     * This updates the CC value known to the synth, but without performing
     * additional MIDI-specific interpretations. (eg. the CC 120 and up)
     *
     * This command should be delay-ordered with all other midi-type events
     * (notes, CCs, aftertouch and pitch-wheel), otherwise the behavior of the
     * synth is undefined.
     *
     * @since 1.0.0
     *
     * @param delay the delay at which the event occurs; this should be lower
     *              than the size of the block in the next call to renderBlock().
     * @param ccNumber the cc number, in domain 0 to 127.
     * @param normValue the normalized cc value, in domain 0 to 1.
     *
     * @par Thread-safety constraints
     * - @b RT: the function must be invoked from the Real-time thread
     */
    void automateHdcc(int delay, int ccNumber, float normValue) noexcept;

    /**
     * @brief Send a pitch bend event to the synth
     *
     * This command should be delay-ordered with all other midi-type events
     * (notes, CCs, aftertouch and pitch-wheel), otherwise the behavior of the
     * synth is undefined.
     *
     * @since 0.2.0
     *
     * @param delay the delay at which the event occurs; this should be lower
     *              than the size of the block in the next call to renderBlock().
     * @param pitch the pitch value centered between -8192 and 8192.
     *
     * @par Thread-safety constraints
     * - @b RT: the function must be invoked from the Real-time thread
     */
    void pitchWheel(int delay, int pitch) noexcept;

    /**
     * @brief Send a high-precision pitch bend event to the synth
     *
     * This command should be delay-ordered with all other midi-type events
     * (notes, CCs, aftertouch and pitch-wheel), otherwise the behavior of the
     * synth is undefined.
     *
     * @since 1.0.0
     *
     * @param delay the delay at which the event occurs; this should be lower
     *              than the size of the block in the next call to renderBlock().
     * @param pitch the normalized pitch, in domain -1 to 1.
     *
     * @par Thread-safety constraints
     * - @b RT: the function must be invoked from the Real-time thread
     */
    void hdPitchWheel(int delay, float pitch) noexcept;

    /**
     * @brief Send an aftertouch event to the synth.
     *
     * This command should be delay-ordered with all other midi-type events
     * (notes, CCs, aftertouch and pitch-wheel), otherwise the behavior of the
     * synth is undefined.
     *
     * @since 0.2.0
     *
     * @param delay the delay at which the event occurs; this should be lower
     *              than the size of the block in the next call to renderBlock().
     * @param aftertouch the aftertouch value, in domain 0 to 127.
     *
     * @par Thread-safety constraints
     * - @b RT: the function must be invoked from the Real-time thread
     */
    SFIZZ_DEPRECATED_API void aftertouch(int delay, int aftertouch) noexcept;

    /**
     * @brief Send a channel aftertouch (channel pressure) event to the synth.
     *
     * This command should be delay-ordered with all other midi-type events
     * (notes, CCs, aftertouch and pitch-wheel), otherwise the behavior of the
     * synth is undefined.
     *
     * @since 0.2.0
     *
     * @param delay the delay at which the event occurs; this should be lower
     *              than the size of the block in the next call to renderBlock().
     * @param aftertouch the aftertouch value, in domain 0 to 127.
     *
     * @par Thread-safety constraints
     * - @b RT: the function must be invoked from the Real-time thread
     */
    void channelAftertouch(int delay, int aftertouch) noexcept;

    /**
     * @brief Send a high-precision aftertouch event to the synth.
     *
     * This command should be delay-ordered with all other midi-type events
     * (notes, CCs, aftertouch and pitch-wheel), otherwise the behavior of the
     * synth is undefined.
     *
     * @since 1.0.0
     *
     * @param delay the delay at which the event occurs; this should be lower
     *              than the size of the block in the next call to renderBlock().
     * @param aftertouch the normalized aftertouch value, in domain 0 to 1.
     *
     * @par Thread-safety constraints
     * - @b RT: the function must be invoked from the Real-time thread
     */
    void hdChannelAftertouch(int delay, float aftertouch) noexcept;

    /**
     * @brief Send a polyphonic aftertouch event to the synth.
     *        This feature is experimental and needs more testing in the internal engine.
     *
     * This command should be delay-ordered with all other midi-type events
     * (notes, CCs, aftertouch and pitch-wheel), otherwise the behavior of the
     * synth is undefined.
     *
     * @since 1.0.0
     *
     * @param delay the delay at which the event occurs; this should be lower
     *              than the size of the block in the next call to renderBlock().
     * @param noteNumber the note number, in domain 0 to 127.
     * @param aftertouch the aftertouch value, in domain 0 to 127.
     *
     * @par Thread-safety constraints
     * - @b RT: the function must be invoked from the Real-time thread
     */
    void polyAftertouch(int delay, int noteNumber, int aftertouch) noexcept;

    /**
     * @brief Send a high-precision polyphonic aftertouch event to the synth.
     *        This feature is experimental and needs more testing in the internal engine.
     *
     * This command should be delay-ordered with all other midi-type events
     * (notes, CCs, aftertouch and pitch-wheel), otherwise the behavior of the
     * synth is undefined.
     *
     * @since 1.0.0
     *
     * @param delay the delay at which the event occurs; this should be lower
     *              than the size of the block in the next call to renderBlock().
     * @param noteNumber the note number, in domain 0 to 127.
     * @param aftertouch the normalized aftertouch value, in domain 0 to 1.
     *
     * @par Thread-safety constraints
     * - @b RT: the function must be invoked from the Real-time thread
     */
    void hdPolyAftertouch(int delay, int noteNumber, float aftertouch) noexcept;

    /**
     * @brief Send a tempo event to the synth.
     *
     * This command should be delay-ordered with all other time/signature commands, namely
     * tempo(), timeSignature(), timePosition(), and playbackState(), otherwise the behavior
     * of the synth is undefined.
     *
     * @since 0.2.0
     *
     * @param delay the delay at which the event occurs; this should be lower
     *              than the size of the block in the next call to renderBlock().
     * @param secondsPerBeat the new period of the beat.
     *
     * @par Thread-safety constraints
     * - @b RT: the function must be invoked from the Real-time thread
     */
    SFIZZ_DEPRECATED_API void tempo(int delay, float secondsPerBeat) noexcept;

    /**
     * @brief Send a tempo event to the synth.
     *
     * This command should be delay-ordered with all other time/signature commands, namely
     * tempo(), timeSignature(), timePosition(), and playbackState(), otherwise the behavior
     * of the synth is undefined.
     *
     * @since 1.0.0
     *
     * @param delay the delay at which the event occurs; this should be lower
     *              than the size of the block in the next call to renderBlock().
     * @param beatsPerMinute the new tempo, in beats per minute.
     *
     * @par Thread-safety constraints
     * - @b RT: the function must be invoked from the Real-time thread
     */
    void bpmTempo(int delay, float beatsPerMinute) noexcept;

    /**
     * @brief Send the time signature.
     *
     * This command should be delay-ordered with all other time/signature commands, namely
     * tempo(), timeSignature(), timePosition(), and playbackState(), otherwise the behavior
     * of the synth is undefined.
     *
     * @since 0.5.0
     *
     * @param delay       The delay.
     * @param beatsPerBar The number of beats per bar, or time signature numerator.
     * @param beatUnit    The note corresponding to one beat, or time signature denominator.
     *
     * @par Thread-safety constraints
     * - @b RT: the function must be invoked from the Real-time thread
     */
    void timeSignature(int delay, int beatsPerBar, int beatUnit);

    /**
     * @brief Send the time position.
     *
     * This command should be delay-ordered with all other time/signature commands, namely
     * tempo(), timeSignature(), timePosition(), and playbackState(), otherwise the behavior
     * of the synth is undefined.
     *
     * @since 0.5.0
     *
     * @param delay   The delay.
     * @param bar     The current bar.
     * @param barBeat The fractional position of the current beat within the bar.
     *
     * @par Thread-safety constraints
     * - @b RT: the function must be invoked from the Real-time thread
     */
    void timePosition(int delay, int bar, double barBeat);

    /**
     * @brief Send the playback state.
     *
     * This command should be delay-ordered with all other time/signature commands, namely
     * tempo(), timeSignature(), timePosition(), and playbackState(), otherwise the behavior
     * of the synth is undefined.
     *
     * @since 0.5.0
     *
     * @param delay         The delay.
     * @param playbackState The playback state, 1 if playing, 0 if stopped.
     *
     * @par Thread-safety constraints
     * - @b RT: the function must be invoked from the Real-time thread
     */
    void playbackState(int delay, int playbackState);

    /**
     * @brief Render an block of audio data in the buffer.
     *
     * This call will reset the synth in its waiting state for the next batch
     * of events. The buffers must be float[numSamples][numOutputs * 2].
     *
     * @since 0.2.0
     *
     * @param buffers the buffers to write the next block into.
     * @param numFrames the number of stereo frames in the block.
     * @param numOutputs the number of stereo outputs.
     *
     * @par Thread-safety constraints
     * - @b RT: the function must be invoked from the Real-time thread
     */
    void renderBlock(float** buffers, size_t numFrames, int numOutputs = 1) noexcept;

    /**
     * @brief Return the number of active voices.
     * @since 0.2.0
     */
    int getNumActiveVoices() const noexcept;

    /**
     * @brief Return the total number of voices in the synth (the polyphony).
     * @since 0.2.0
     */
    int getNumVoices() const noexcept;

    /**
     * @brief Change the number of voices (the polyphony).
     *
     * @since 0.2.0
     *
     * @param numVoices The number of voices.
     *
     * @par Thread-safety constraints
     * - @b CT: the function must be invoked from the Control thread
     * - @b OFF: the function cannot be invoked while a thread is calling @b RT functions
     */
    void setNumVoices(int numVoices) noexcept;

    /**
     * @brief Set the oversampling factor to a new value.
     *
     * As of 1.0, This is an inactive stub for future work on oversampling in the engine.
     *
     *
     * @since 0.2.0
     *
     * @param factor The oversampling factor.
     *
     * @return @true if the factor did indeed change, @false otherwise.
     *
     * @par Thread-safety constraints
     * - @b CT: the function must be invoked from the Control thread
     * - @b OFF: the function cannot be invoked while a thread is calling @b RT functions
     */
    bool setOversamplingFactor(int factor) noexcept;

    /**
     * @brief Return the current oversampling factor.
     * @since 0.2.0
     *
     * As of 1.0, This is an inactive stub for future work on oversampling in the engine.
     *
     */
    int getOversamplingFactor() const noexcept;

    /**
     * @brief Set the preloaded file size.
     *
     * @since 0.2.0
     *
     * @param preloadSize  The preload size.
     *
     * @par Thread-safety constraints
     * - @b CT: the function must be invoked from the Control thread
     * - @b OFF: the function cannot be invoked while a thread is calling @b RT functions
     */
    void setPreloadSize(uint32_t preloadSize) noexcept;

    /**
     * @brief Return the current preloaded file size.
     * @since 0.2.0
     */
    uint32_t getPreloadSize() const noexcept;

    /**
     * @brief Return the number of allocated buffers.
     * @since 0.2.0
     */
    int getAllocatedBuffers() const noexcept;

    /**
     * @brief Return the number of bytes allocated through the buffers.
     * @since 0.2.0
     */
    int getAllocatedBytes() const noexcept;

    /**
     * @brief Enable freewheeling on the synth.
     *
     * This will wait for background loaded files to finish loading
     * before each render callback to ensure that there will be no dropouts.
     *
     * @since 0.2.0
     *
     * @par Thread-safety constraints
     * - @b RT: the function must be invoked from the Real-time thread
     */
    void enableFreeWheeling() noexcept;

    /**
     * @brief Disable freewheeling on the synth.
     *
     * You should disable freewheeling before live use of the plugin
     * otherwise the audio thread will lock.
     *
     * @since 0.2.0
     *
     * @par Thread-safety constraints
     * - @b RT: the function must be invoked from the Real-time thread
     */
    void disableFreeWheeling() noexcept;

    /**
     * @brief Check if the SFZ should be reloaded.
     *
     * Depending on the platform this can create file descriptors.
     *
     * @since 0.2.0
     *
     * @return @true if any included files (including the root file) have
     *         been modified since the sfz file was loaded, @false otherwise.
     *
     * @par Thread-safety constraints
     * - @b CT: the function must be invoked from the Control thread
     */
    bool shouldReloadFile();

    /**
     * @brief Check if the tuning (scala) file should be reloaded.
     *
     * Depending on the platform this can create file descriptors.
     *
     * @since 0.4.0
     *
     * @return @true if a scala file has been loaded and has changed, @false otherwise.
     *
     * @par Thread-safety constraints
     * - @b CT: the function must be invoked from the Control thread
     */
    bool shouldReloadScala();

    /**
     * @brief Enable logging of timings to sidecar CSV files.
     * @since 0.3.0
     *
     * @note This can produce many outputs so use with caution.
     *
     * @param prefix the file prefix to use for logging.
     *
     * @par Thread-safety constraints
     * - TBD ?
     */
    SFIZZ_DEPRECATED_API void enableLogging() noexcept;

    /**
     * @brief Enable logging of timings to sidecar CSV files.
     *
     * @since 0.3.2
     *
     * @note This can produce many outputs so use with caution.
     * Deprecated since 1.2.0
     *
     * @param prefix the file prefix to use for logging.
     *
     * @par Thread-safety constraints
     * - TBD ?
     */
    SFIZZ_DEPRECATED_API void enableLogging(const std::string& prefix) noexcept;

    /**
     * @brief Set the logging prefix.
     *
     * @since 0.3.2
     * @note Deprecated since 1.2.0
     *
     * @param prefix
     *
     *
     * @par Thread-safety constraints
     * - TBD ?
     */
    SFIZZ_DEPRECATED_API void setLoggingPrefix(const std::string& prefix) noexcept;

    /**
     * @brief Disable logging of timings to sidecar CSV files.
     *
     * @since 0.3.0
     * @note Deprecated since 1.2.0
     *
     * @par Thread-safety constraints
     * - TBD ?
     */
    SFIZZ_DEPRECATED_API void disableLogging() noexcept;

    struct CallbackBreakdown
    {
        double dispatch;
        double renderMethod;
        double data;
        double amplitude;
        double filters;
        double panning;
        double effects;
    };

    /**
     * @brief Get the last callback breakdown. Call after a render method.
     *
     * @since 1.2.0
     *
     * @par Thread-safety constraints
     * - @b RT: the function must be invoked from the Real-time thread
     *
     * @return CallbackBreakdown
     */
    CallbackBreakdown getCallbackBreakdown() noexcept;

    /**
     * @brief Shuts down the current processing, clear buffers and reset the voices.
     *
     * @since 0.3.2
     *
     * @par Thread-safety constraints
     * - @b RT: the function must be invoked from the Real-time thread
     */
    void allSoundOff() noexcept;

    /**
     * @brief Add external definitions prior to loading.
     *
     * @since 0.4.0
     *
     * @note These do not get reset by loading or resetting the synth.
     * You need to call clearExternalDefintions() to erase them.
     *
     * @param id    The definition variable name.
     * @param value The definition value.
     *
     * @par Thread-safety constraints
     * - @b CT: the function must be invoked from the Control thread
     */
    void addExternalDefinition(const std::string& id, const std::string& value);

    /**
     * @brief Clears external definitions for the next file loading.
     *
     * @since 0.4.0
     *
     * @par Thread-safety constraints
     * - @b CT: the function must be invoked from the Control thread
     */
    void clearExternalDefinitions();

    /**
     * @brief Get the key labels, if any.
     * @since 0.4.0
     */
    const std::vector<std::pair<uint8_t, std::string>>& getKeyLabels() const noexcept;

    /**
     * @brief Get the CC labels, if any.
     * @since 0.4.0
     */
    const std::vector<std::pair<uint16_t, std::string>>& getCCLabels() const noexcept;

    /**
     * @brief Export a MIDI Name document describing the currently loaded SFZ file.
     * @since 1.0.0
     *
     * @param model  The model name used if a non-empty string, otherwise generated.
     *
     * @return A XML string.
     */
    std::string exportMidnam(const std::string& model) const;

    /**
     * @addtogroup Messaging
     * @{
     */

private:
    struct ClientDeleter {
        void operator()(Client *client) const noexcept;
    };

public:
//! @cond Doxygen_Suppress
    using ClientPtr = std::unique_ptr<Client, ClientDeleter>;
//! @endcond

    /**
     * @brief Create a new messaging client
     * @since 1.0.0
     *
     * @param data         The opaque data pointer which is passed to the receiver.
     * @return             The new client.
     */
    static ClientPtr createClient(void* data);

    /**
     * @brief Get the client data
     * @since 1.0.0
     *
     * @param client       The client.
     * @return             The client data.
     */
    static void* getClientData(Client& client);

    /**
     * @brief Set the function which receives reply messages from the synth engine.
     * @since 1.0.0
     *
     * @param client       The client.
     * @param receive      The pointer to the receiving function.
     */
    static void setReceiveCallback(Client& client, sfizz_receive_t* receive);

    /**
     * @brief Send a message to the synth engine
     *
     * @since 1.0.0
     *
     * @param client       The client sending the message.
     * @param delay        The delay of the message in the block, in samples.
     * @param path         The OSC address pattern.
     * @param sig          The OSC type tag string.
     * @param args         The OSC arguments, whose number and format is determined the type tag string.
     *
     * @par Thread-safety constraints
     * - @b RT: the function must be invoked from the Real-time thread
     */
    void sendMessage(Client& client, int delay, const char* path, const char* sig, const sfizz_arg_t* args);

    /**
     * @brief Set the function which receives broadcast messages from the synth engine.
     *
     * @since 1.0.0
     *
     * @param broadcast    The pointer to the receiving function.
     * @param data         The opaque data pointer which is passed to the receiver.
     *
     * @par Thread-safety constraints
     * - @b RT: the function must be invoked from the Real-time thread
     */
    void setBroadcastCallback(sfizz_receive_t* broadcast, void* data);

    /**
     * @}
     */

private:
    sfizz_synth_t* synth {};
};
//! @cond Doxygen_Suppress
using ClientPtr = Sfizz::ClientPtr;
//! @endcond
}
