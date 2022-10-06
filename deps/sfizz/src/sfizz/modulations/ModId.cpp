// SPDX-License-Identifier: BSD-2-Clause

// This code is part of the sfizz library and is licensed under a BSD 2-clause
// license. You should have receive a LICENSE.md file along with the code.
// If not, contact the sfizz maintainers at https://github.com/sfztools/sfizz

#include "ModId.h"

namespace sfz {

bool ModIds::isSource(ModId id) noexcept
{
    return static_cast<int>(id) >= static_cast<int>(ModId::_SourcesStart) &&
        static_cast<int>(id) < static_cast<int>(ModId::_SourcesEnd);
}

bool ModIds::isTarget(ModId id) noexcept
{
    return static_cast<int>(id) >= static_cast<int>(ModId::_TargetsStart) &&
        static_cast<int>(id) < static_cast<int>(ModId::_TargetsEnd);
}

int ModIds::flags(ModId id) noexcept
{
    switch (id) {
        // sources
    case ModId::Controller:
        return kModIsPerCycle;
    case ModId::Envelope:
        return kModIsPerVoice;
    case ModId::LFO:
        return kModIsPerVoice;
    case ModId::AmpLFO:
        return kModIsPerVoice;
    case ModId::PitchLFO:
        return kModIsPerVoice;
    case ModId::FilLFO:
        return kModIsPerVoice;
    case ModId::AmpEG:
        return kModIsPerVoice;
    case ModId::PitchEG:
        return kModIsPerVoice;
    case ModId::FilEG:
        return kModIsPerVoice;
    case ModId::ChannelAftertouch:
        return kModIsPerCycle;
    case ModId::PolyAftertouch:
        return kModIsPerVoice;
    case ModId::PerVoiceController:
        return kModIsPerVoice;

        // targets
    case ModId::MasterAmplitude:
        return kModIsPerVoice|kModIsMultiplicative;
    case ModId::Amplitude:
        return kModIsPerVoice|kModIsMultiplicative;
    case ModId::Pan:
        return kModIsPerVoice|kModIsAdditive;
    case ModId::Width:
        return kModIsPerVoice|kModIsAdditive;
    case ModId::Position:
        return kModIsPerVoice|kModIsAdditive;
    case ModId::Pitch:
        return kModIsPerVoice|kModIsAdditive;
    case ModId::Volume:
        return kModIsPerVoice|kModIsAdditive;
    case ModId::FilGain:
        return kModIsPerVoice|kModIsAdditive;
    case ModId::FilCutoff:
        return kModIsPerVoice|kModIsAdditive;
    case ModId::FilResonance:
        return kModIsPerVoice|kModIsAdditive;
    case ModId::EqGain:
        return kModIsPerVoice|kModIsAdditive;
    case ModId::EqFrequency:
        return kModIsPerVoice|kModIsAdditive;
    case ModId::EqBandwidth:
        return kModIsPerVoice|kModIsAdditive;
    case ModId::OscillatorDetune:
        return kModIsPerVoice|kModIsAdditive;
    case ModId::OscillatorModDepth:
        return kModIsPerVoice|kModIsMultiplicative;
    case ModId::PitchEGDepth:
        return kModIsPerVoice|kModIsAdditive;
    case ModId::FilEGDepth:
        return kModIsPerVoice|kModIsAdditive;
    case ModId::AmpLFODepth:
        return kModIsPerVoice|kModIsAdditive;
    case ModId::AmpLFOFrequency:
        return kModIsPerVoice|kModIsAdditive;
    case ModId::PitchLFODepth:
        return kModIsPerVoice|kModIsAdditive;
    case ModId::PitchLFOFrequency:
        return kModIsPerVoice|kModIsAdditive;
    case ModId::FilLFODepth:
        return kModIsPerVoice|kModIsAdditive;
    case ModId::FilLFOFrequency:
        return kModIsPerVoice|kModIsAdditive;
    case ModId::LFOFrequency:
        return kModIsPerVoice|kModIsAdditive;
    case ModId::LFOBeats:
        return kModIsPerVoice|kModIsAdditive;
    case ModId::LFOPhase:
        return kModIsPerVoice|kModIsAdditive;
    case ModId::LFOAmplitudeDepth:
        return kModIsPerVoice|kModIsMultiplicative;
    case ModId::LFOPanDepth:
        return kModIsPerVoice|kModIsAdditive;
    case ModId::LFOWidthDepth:
        return kModIsPerVoice|kModIsAdditive;
    case ModId::LFOPositionDepth:
        return kModIsPerVoice|kModIsAdditive;
    case ModId::LFOPitchDepth:
        return kModIsPerVoice|kModIsAdditive;
    case ModId::LFOVolumeDepth:
        return kModIsPerVoice|kModIsAdditive;
    case ModId::LFOFilCutoffDepth:
        return kModIsPerVoice|kModIsAdditive;
    case ModId::LFOFilResonanceDepth:
        return kModIsPerVoice|kModIsAdditive;
    case ModId::LFOFilGainDepth:
        return kModIsPerVoice|kModIsAdditive;
    case ModId::LFOEqGainDepth:
        return kModIsPerVoice|kModIsAdditive;
    case ModId::LFOEqFrequencyDepth:
        return kModIsPerVoice|kModIsAdditive;
    case ModId::LFOEqBandwidthDepth:
        return kModIsPerVoice|kModIsAdditive;
    case ModId::EGAmplitudeDepth:
        return kModIsPerVoice|kModIsMultiplicative;
    case ModId::EGPanDepth:
        return kModIsPerVoice|kModIsAdditive;
    case ModId::EGWidthDepth:
        return kModIsPerVoice|kModIsAdditive;
    case ModId::EGPositionDepth:
        return kModIsPerVoice|kModIsAdditive;
    case ModId::EGPitchDepth:
        return kModIsPerVoice|kModIsAdditive;
    case ModId::EGVolumeDepth:
        return kModIsPerVoice|kModIsAdditive;
    case ModId::EGFilCutoffDepth:
        return kModIsPerVoice|kModIsAdditive;
    case ModId::EGFilResonanceDepth:
        return kModIsPerVoice|kModIsAdditive;
    case ModId::EGFilGainDepth:
        return kModIsPerVoice|kModIsAdditive;
    case ModId::EGEqGainDepth:
        return kModIsPerVoice|kModIsAdditive;
    case ModId::EGEqFrequencyDepth:
        return kModIsPerVoice|kModIsAdditive;
    case ModId::EGEqBandwidthDepth:
        return kModIsPerVoice|kModIsAdditive;

        // unknown
    default:
        return kModFlagsInvalid;
    }
}

} // namespace sfz
