// SPDX-License-Identifier: BSD-2-Clause

// This code is part of the sfizz library and is licensed under a BSD 2-clause
// license. You should have receive a LICENSE.md file along with the code.
// If not, contact the sfizz maintainers at https://github.com/sfztools/sfizz

#pragma once
#ifdef _WIN32
// There's a spurious min/max function in MSVC that makes everything go badly...
#ifndef NOMINMAX
#define NOMINMAX 1
#endif
#define _SILENCE_CXX17_OLD_ALLOCATOR_MEMBERS_DEPRECATION_WARNING
#endif
#include "absl/strings/string_view.h"
#include <cstddef>
#include <cstdint>

namespace sfz {

enum ExtendedCCs {
    pitchBend = 128,
    channelAftertouch = 129,
    polyphonicAftertouch = 130,
    noteOnVelocity = 131,
    noteOffVelocity = 132,
    keyboardNoteNumber = 133,
    keyboardNoteGate = 134,
    unipolarRandom = 135,
    bipolarRandom = 136,
    alternate = 137,
    keydelta = 140,
    absoluteKeydelta = 141,
};

namespace config {
    constexpr float defaultSampleRate { 48000 };
    constexpr float maxSampleRate { 192000 };
    constexpr int defaultSamplesPerBlock { 1024 };
    constexpr int maxBlockSize { 8192 };
    constexpr int bufferPoolSize { 6 };
    constexpr int stereoBufferPoolSize { 4 };
    constexpr int indexBufferPoolSize { 4 };
    constexpr int preloadSize { 8192 };
    constexpr bool loadInRam { false };
    constexpr int loggerQueueSize { 256 };
    constexpr int voiceLoggerQueueSize { 256 };
    constexpr bool loggingEnabled { false };
    constexpr size_t maxChannels { 32 };
    constexpr int numBackgroundThreads { 4 };
    constexpr unsigned fileClearingPeriod { 5 }; // in seconds
    constexpr int numVoices { 64 };
    constexpr unsigned maxVoices { 256 };
    constexpr unsigned smoothingSteps { 512 };
    constexpr uint16_t xfadeSmoothing { 5 };
    constexpr uint16_t gainSmoothing { 0 };
    constexpr unsigned powerTableSizeExponent { 11 };
    constexpr int maxFilePromises { maxVoices };
    constexpr int allSoundOffCC { 120 };
    constexpr int resetCC { 121 };
    constexpr int allNotesOffCC { 123 };
    constexpr int omniOffCC { 124 };
    constexpr int omniOnCC { 125 };
    constexpr int centPerSemitone { 100 };
    constexpr float virtuallyZero { 0.001f };
    constexpr float fastReleaseDuration { 0.01f };
    constexpr char defineCharacter { '$' };
    constexpr float A440 { 440.0 };
    constexpr size_t powerHistoryLength { 16 };
    constexpr size_t powerFollowerStep { 512 };
    constexpr float powerFollowerAttackTime { 5e-3f };
    constexpr float powerFollowerReleaseTime { 200e-3f };
    constexpr uint16_t numCCs { 512 };
    constexpr int maxCurves { 256 };
    constexpr int fileChunkSize { 1024 };
    constexpr int processChunkSize { 16 };
    constexpr unsigned int defaultAlignment { 16 };
    constexpr int filtersInPool { maxVoices * 2 };
    constexpr int excessFileFrames { 64 };
    constexpr int maxLFOSubs { 8 };
    constexpr int maxLFOSteps { 128 };
    /**
     * @brief The threshold for age stealing.
     *        In percentage of the voice's max age.
     */
    constexpr float stealingAgeCoeff { 0.5f };
    /**
     * @brief The threshold for power stealing.
     *        In percentage of the sum of all powers.
     */
    constexpr float stealingPowerCoeff { 0.5f };
    constexpr int filtersPerVoice { 2 };
    constexpr int eqsPerVoice { 3 };
    constexpr int oscillatorsPerVoice { 9 };
    constexpr float uniformNoiseBounds { 1.0f };
    constexpr float noiseVariance { 0.25f };
    /**
       Minimum interval in frames between recomputations of coefficients of the
       modulated filter. The lower, the more CPU resources are consumed.
    */
    constexpr int filterControlInterval { 16 };
    /**
       Amplitude below which an exponential releasing envelope is considered as
       finished.
     */
    constexpr float egReleaseThreshold = 1e-4;
    /**
       Duration of a linear transition user to smooth cases of otherwise
       immediate level transitions. (eg. decay->sustain or release->off)
     */
    constexpr float egTransitionTime = 50e-3;
    /**
       Default metadata for MIDIName documents
     */
    const absl::string_view midnamManufacturer { "The Sfizz authors" };
    const absl::string_view midnamModel { "Sfizz" };
    /**
       Limit of how many "fxN" buses are accepted (in SFZv2, maximum is 4)
     */
    constexpr int maxEffectBuses { 256 };
    // Wavetable constants; amplitude values are matched to reference
    static constexpr unsigned tableSize = 1024;
    static constexpr double tableRefSampleRate = 44100.0 * 1.1; // +10% aliasing permissivity
    /**
       Default wave amplitudes, adjusted for consistent RMS among all waves.
       (except square curiously, but it's to match ARIA)
     */
    static constexpr double amplitudeSine = 1.0;
    static constexpr double amplitudeTriangle = 1.0;
    static constexpr double amplitudeSaw = 0.8164965809277261; // sqrt(2)/sqrt(3)
    static constexpr double amplitudeSquare = 0.8164965809277261; // should have been sqrt(2)?
    /**
       Frame count high limit, for automatically loading a sound file as wavetable.
       Set to 3000 according to Cakewalk.
     */
    static constexpr unsigned wavetableMaxFrames = 3000;
    /**
       Background file loading
     */
    static constexpr int backgroundLoaderPthreadPriority = 50; // expressed in %
    /**
       @brief Ratio to target under which smoothing is considered as completed
     */
    static constexpr float smoothingShortcutThreshold = 5e-3;
    // loop crossfade settings
    static constexpr int loopXfadeCurve = 2;    // 0: linear
                                                // 1: use curves 5 & 6
                                                // 2: use S-shaped curve
    /**
     * @brief Overflow voices in the engine, relative to the required voices.
     * These are additional voices that more or less hold the "dying" voices
     * due to engine polyphony being reached.
     */
    static constexpr float overflowVoiceMultiplier { 1.5f };
    static_assert(overflowVoiceMultiplier >= 1.0f, "This needs to add voices");

    /**
     * @brief Calculate the effective voice number for the polyphony setting,
     * accounting for the overflow factor.
     */
    inline constexpr int calculateActualVoices(int polyphony)
    {
        return
            (int(polyphony * config::overflowVoiceMultiplier) < int(config::maxVoices)) ?
            int(polyphony * config::overflowVoiceMultiplier) : int(config::maxVoices);
    }
    /**
     * @brief The smoothing time constant per "smooth" steps
     */
    constexpr float smoothTauPerStep { 3e-3 };
    /**
     * @brief If a value below this threshold is given to `ampeg_sustain`, the envelope will free-run
     * and the voice will release itself at the end of the decay stage.
     */
    constexpr float sustainFreeRunningThreshold { 0.0032f };
    /**
     * @brief Number of frames offset between the end of a block and the beginning of the next
     * detected as a shift in the playhead position
     *
     */
    constexpr int playheadMovedFrames { 16 };
    /**
     * @brief Max number of voices to start on release pedal up
     *
     */
    constexpr unsigned delayedReleaseVoices { 16 };
} // namespace config

} // namespace sfz
