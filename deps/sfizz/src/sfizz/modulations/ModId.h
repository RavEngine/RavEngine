// SPDX-License-Identifier: BSD-2-Clause

// This code is part of the sfizz library and is licensed under a BSD 2-clause
// license. You should have receive a LICENSE.md file along with the code.
// If not, contact the sfizz maintainers at https://github.com/sfztools/sfizz

#pragma once

namespace sfz {

/**
 * @brief Generic identifier of a kind of modulation source or target,
 *        not necessarily unique per SFZ instrument
 */
enum class ModId : int {
    Undefined,

    //--------------------------------------------------------------------------
    // Sources
    //--------------------------------------------------------------------------
    _SourcesStart,

    Controller = _SourcesStart,
    Envelope,
    LFO,
    AmpLFO,
    PitchLFO,
    FilLFO,
    AmpEG,
    PitchEG,
    FilEG,
    ChannelAftertouch,
    PolyAftertouch,
    PerVoiceController,
    _SourcesEnd,

    //--------------------------------------------------------------------------
    // Targets
    //--------------------------------------------------------------------------
    _TargetsStart = _SourcesEnd,

    MasterAmplitude = _TargetsStart,
    Amplitude,
    Pan,
    Width,
    Position,
    Pitch,
    Volume,
    FilGain,
    FilCutoff,
    FilResonance,
    EqGain,
    EqFrequency,
    EqBandwidth,
    OscillatorDetune,
    OscillatorModDepth,
    PitchEGDepth,
    FilEGDepth,
    AmpLFODepth,
    AmpLFOFrequency,
    PitchLFODepth,
    PitchLFOFrequency,
    FilLFODepth,
    FilLFOFrequency,
    LFOFrequency,
    LFOBeats,
    LFOPhase,
    LFOAmplitudeDepth,
    LFOPanDepth,
    LFOWidthDepth,
    LFOPositionDepth,
    LFOPitchDepth,
    LFOVolumeDepth,
    LFOFilCutoffDepth,
    LFOFilResonanceDepth,
    LFOFilGainDepth,
    LFOEqGainDepth,
    LFOEqFrequencyDepth,
    LFOEqBandwidthDepth,
    EGAmplitudeDepth,
    EGPanDepth,
    EGWidthDepth,
    EGPositionDepth,
    EGPitchDepth,
    EGVolumeDepth,
    EGFilCutoffDepth,
    EGFilResonanceDepth,
    EGFilGainDepth,
    EGEqGainDepth,
    EGEqFrequencyDepth,
    EGEqBandwidthDepth,

    _TargetsEnd,
    // [/targets] --------------------------------------------------------------
};

/**
 * @brief Modulation bit flags (S=source, T=target, ST=either)
 */
enum ModFlags : int {
    //! This modulation is invalid. (ST)
    kModFlagsInvalid = -1,

    //! This modulation is global (the default). (ST)
    kModIsPerCycle = 1 << 1,
    //! This modulation is updated separately for every region of every voice (ST)
    kModIsPerVoice = 1 << 2,

    //! This target is additive. (T)
    kModIsAdditive = 1 << 3,
    //! This target is multiplicative (T)
    kModIsMultiplicative = 1 << 4,
};

namespace ModIds {

bool isSource(ModId id) noexcept;
bool isTarget(ModId id) noexcept;
int flags(ModId id) noexcept;

template <class F> inline void forEachSourceId(F&& f)
{
    for (int i = static_cast<int>(ModId::_SourcesStart);
         i < static_cast<int>(ModId::_SourcesEnd); ++i)
        f(static_cast<ModId>(i));
}

template <class F> inline void forEachTargetId(F&& f)
{
    for (int i = static_cast<int>(ModId::_TargetsStart);
         i < static_cast<int>(ModId::_TargetsEnd); ++i)
        f(static_cast<ModId>(i));
}

} // namespace ModIds

} // namespace sfz
