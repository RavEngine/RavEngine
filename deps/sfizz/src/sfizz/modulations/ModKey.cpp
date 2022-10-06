// SPDX-License-Identifier: BSD-2-Clause

// This code is part of the sfizz library and is licensed under a BSD 2-clause
// license. You should have receive a LICENSE.md file along with the code.
// If not, contact the sfizz maintainers at https://github.com/sfztools/sfizz

#include "ModKey.h"
#include "../Debug.h"
#include <absl/strings/str_cat.h>

namespace sfz {

ModKey::Parameters::Parameters() noexcept
{
    // zero-fill the structure
    //  1. this ensures that non-used values will be always 0
    //  2. this makes the object memcmp-comparable
    std::memset(
        static_cast<RawParameters*>(this),
        0, sizeof(RawParameters));
}

ModKey::Parameters::Parameters(const Parameters& other) noexcept
{
    std::memcpy(
        static_cast<RawParameters*>(this),
        static_cast<const RawParameters*>(&other),
        sizeof(RawParameters));
}

ModKey::Parameters& ModKey::Parameters::operator=(const Parameters& other) noexcept
{
    if (this != &other)
        std::memcpy(
            static_cast<RawParameters*>(this),
            static_cast<const RawParameters*>(&other),
            sizeof(RawParameters));
    return *this;
}

ModKey::Parameters::Parameters(Parameters&& other) noexcept
{
    std::memcpy(
        static_cast<RawParameters*>(this),
        static_cast<const RawParameters*>(&other),
        sizeof(RawParameters));
}

ModKey::Parameters& ModKey::Parameters::operator=(Parameters&& other) noexcept
{
    if (this != &other)
        std::memcpy(
            static_cast<RawParameters*>(this),
            static_cast<const RawParameters*>(&other),
            sizeof(RawParameters));
    return *this;
}

ModKey ModKey::createCC(uint16_t cc, uint8_t curve, uint16_t smooth, float step)
{
    ModKey::Parameters p;
    p.cc = cc;
    p.curve = curve;
    p.smooth = smooth;
    p.step = step;
    return ModKey(ModId::Controller, {}, p);
}

ModKey ModKey::createNXYZ(ModId id, NumericId<Region> region, uint8_t N, uint8_t X, uint8_t Y, uint8_t Z)
{
    ASSERT(id != ModId::Controller);
    ModKey::Parameters p;
    p.N = N;
    p.X = X;
    p.Y = Y;
    p.Z = Z;
    return ModKey(id, region, p);
}

bool ModKey::isSource() const noexcept
{
    return ModIds::isSource(id_);
}

bool ModKey::isTarget() const noexcept
{
    return ModIds::isTarget(id_);
}


std::string ModKey::toString() const
{
    switch (id_) {
    case ModId::Controller:
        return absl::StrCat("Controller ", params_.cc,
            " {curve=", params_.curve, ", smooth=", params_.smooth,
            ", step=", params_.step, "}");
    case ModId::Envelope:
        return absl::StrCat("EG ", 1 + params_.N, " {", region_.number(), "}");
    case ModId::LFO:
        return absl::StrCat("LFO ", 1 + params_.N, " {", region_.number(), "}");
    case ModId::AmpLFO:
        return absl::StrCat("AmplitudeLFO {", region_.number(), "}");
    case ModId::PitchLFO:
        return absl::StrCat("PitchLFO {", region_.number(), "}");
    case ModId::FilLFO:
        return absl::StrCat("FilterLFO {", region_.number(), "}");
    case ModId::AmpEG:
        return absl::StrCat("AmplitudeEG {", region_.number(), "}");
    case ModId::PitchEG:
        return absl::StrCat("PitchEG {", region_.number(), "}");
    case ModId::FilEG:
        return absl::StrCat("FilterEG {", region_.number(), "}");
    case ModId::ChannelAftertouch:
        return absl::StrCat("ChannelAftertouch");
    case ModId::PolyAftertouch:
        return absl::StrCat("PolyAftertouch");
    case ModId::PerVoiceController:
        return absl::StrCat("PerVoiceController ", params_.cc,
            " {curve=", params_.curve, ", smooth=", params_.smooth,
            ", step=", params_.step, ", region=", region_.number(), "}");

    case ModId::MasterAmplitude:
        return absl::StrCat("MasterAmplitude {", region_.number(), "}");
    case ModId::Amplitude:
        return absl::StrCat("Amplitude {", region_.number(), "}");
    case ModId::Pan:
        return absl::StrCat("Pan {", region_.number(), "}");
    case ModId::Width:
        return absl::StrCat("Width {", region_.number(), "}");
    case ModId::Position:
        return absl::StrCat("Position {", region_.number(), "}");
    case ModId::Pitch:
        return absl::StrCat("Pitch {", region_.number(), "}");
    case ModId::Volume:
        return absl::StrCat("Volume {", region_.number(), "}");
    case ModId::FilGain:
        return absl::StrCat("FilterGain {", region_.number(), ", N=", 1 + params_.N, "}");
    case ModId::FilCutoff:
        return absl::StrCat("FilterCutoff {", region_.number(), ", N=", 1 + params_.N, "}");
    case ModId::FilResonance:
        return absl::StrCat("FilterResonance {", region_.number(), ", N=", 1 + params_.N, "}");
    case ModId::EqGain:
        return absl::StrCat("EqGain {", region_.number(), ", N=", 1 + params_.N, "}");
    case ModId::EqFrequency:
        return absl::StrCat("EqFrequency {", region_.number(), ", N=", 1 + params_.N, "}");
    case ModId::EqBandwidth:
        return absl::StrCat("EqBandwidth {", region_.number(), ", N=", 1 + params_.N, "}");
    case ModId::OscillatorDetune:
        return absl::StrCat("OscillatorDetune {", region_.number(), ", N=", 1 + params_.N, "}");
   case ModId::OscillatorModDepth:
        return absl::StrCat("OscillatorModDepth {", region_.number(), ", N=", 1 + params_.N, "}");
    case ModId::PitchEGDepth:
        return absl::StrCat("PitchEGDepth {", region_.number(), "}");
    case ModId::FilEGDepth:
        return absl::StrCat("FilterEGDepth {", region_.number(), "}");
    case ModId::AmpLFODepth:
        return absl::StrCat("AmplitudeLFODepth {", region_.number(), "}");
    case ModId::AmpLFOFrequency:
        return absl::StrCat("AmplitudeLFOFrequency {", region_.number(), "}");
    case ModId::PitchLFODepth:
        return absl::StrCat("PitchLFODepth {", region_.number(), "}");
    case ModId::PitchLFOFrequency:
        return absl::StrCat("PitchLFOFrequency {", region_.number(), "}");
    case ModId::FilLFODepth:
        return absl::StrCat("FilterLFODepth {", region_.number(), "}");
    case ModId::FilLFOFrequency:
        return absl::StrCat("FilterLFOFrequency {", region_.number(), "}");
    case ModId::LFOFrequency:
        return absl::StrCat("LFOFrequency {", region_.number(), ", N=", 1 + params_.N, "}");
    case ModId::LFOBeats:
        return absl::StrCat("LFOBeats {", region_.number(), ", N=", 1 + params_.N, "}");
    case ModId::LFOPhase:
        return absl::StrCat("LFOPhase {", region_.number(), ", N=", 1 + params_.N, "}");
    case ModId::LFOAmplitudeDepth:
        return absl::StrCat("LFOAmplitudeDepth {", region_.number(), ", N=", 1 + params_.N, "}");
    case ModId::LFOPanDepth:
        return absl::StrCat("LFOPanDepth {", region_.number(), ", N=", 1 + params_.N, "}");
    case ModId::LFOWidthDepth:
        return absl::StrCat("LFOWidthDepth {", region_.number(), ", N=", 1 + params_.N, "}");
    case ModId::LFOPositionDepth:
        return absl::StrCat("LFOPositionDepth {", region_.number(), ", N=", 1 + params_.N, "}");
    case ModId::LFOPitchDepth:
        return absl::StrCat("LFOPitchDepth {", region_.number(), ", N=", 1 + params_.N, "}");
    case ModId::LFOVolumeDepth:
        return absl::StrCat("LFOVolumeDepth {", region_.number(), ", N=", 1 + params_.N, "}");
    case ModId::LFOFilCutoffDepth:
        return absl::StrCat("LFOFilCutoffDepth {", region_.number(), ", N=", 1 + params_.N, ", X=", 1 + params_.X, "}");
    case ModId::LFOFilResonanceDepth:
        return absl::StrCat("LFOFilResonanceDepth {", region_.number(), ", N=", 1 + params_.N, ", X=", 1 + params_.X, "}");
    case ModId::LFOFilGainDepth:
        return absl::StrCat("LFOFilGainDepth {", region_.number(), ", N=", 1 + params_.N, ", X=", 1 + params_.X, "}");
    case ModId::LFOEqGainDepth:
        return absl::StrCat("LFOEqGainDepth {", region_.number(), ", N=", 1 + params_.N, ", X=", 1 + params_.X, "}");
    case ModId::LFOEqFrequencyDepth:
        return absl::StrCat("LFOEqFrequencyDepth {", region_.number(), ", N=", 1 + params_.N, ", X=", 1 + params_.X, "}");
    case ModId::LFOEqBandwidthDepth:
        return absl::StrCat("LFOEqBandwidthDepth {", region_.number(), ", N=", 1 + params_.N, ", X=", 1 + params_.X, "}");
    case ModId::EGAmplitudeDepth:
        return absl::StrCat("EGAmplitudeDepth {", region_.number(), ", N=", 1 + params_.N, "}");
    case ModId::EGPanDepth:
        return absl::StrCat("EGPanDepth {", region_.number(), ", N=", 1 + params_.N, "}");
    case ModId::EGWidthDepth:
        return absl::StrCat("EGWidthDepth {", region_.number(), ", N=", 1 + params_.N, "}");
    case ModId::EGPositionDepth:
        return absl::StrCat("EGPositionDepth {", region_.number(), ", N=", 1 + params_.N, "}");
    case ModId::EGPitchDepth:
        return absl::StrCat("EGPitchDepth {", region_.number(), ", N=", 1 + params_.N, "}");
    case ModId::EGVolumeDepth:
        return absl::StrCat("EGVolumeDepth {", region_.number(), ", N=", 1 + params_.N, "}");
    case ModId::EGFilCutoffDepth:
        return absl::StrCat("EGFilCutoffDepth {", region_.number(), ", N=", 1 + params_.N, ", X=", 1 + params_.X, "}");
    case ModId::EGFilResonanceDepth:
        return absl::StrCat("EGFilResonanceDepth {", region_.number(), ", N=", 1 + params_.N, ", X=", 1 + params_.X, "}");
    case ModId::EGFilGainDepth:
        return absl::StrCat("EGFilGainDepth {", region_.number(), ", N=", 1 + params_.N, ", X=", 1 + params_.X, "}");
    case ModId::EGEqGainDepth:
        return absl::StrCat("EGEqGainDepth {", region_.number(), ", N=", 1 + params_.N, ", X=", 1 + params_.X, "}");
    case ModId::EGEqFrequencyDepth:
        return absl::StrCat("EGEqFrequencyDepth {", region_.number(), ", N=", 1 + params_.N, ", X=", 1 + params_.X, "}");
    case ModId::EGEqBandwidthDepth:
        return absl::StrCat("EGEqBandwidthDepth {", region_.number(), ", N=", 1 + params_.N, ", X=", 1 + params_.X, "}");

    default:
        return {};
    }
}

ModKey ModKey::getSourceDepthKey(ModKey source, ModKey target)
{
    const NumericId<Region> region = source.region();
    const ModKey::Parameters& tp = target.parameters();

    switch (source.id()) {
    case ModId::AmpLFO:
        switch (target.id()) {
        case ModId::Volume:
            return ModKey::createNXYZ(ModId::AmpLFODepth, region);
        default:
            break;
        }
        break;

    case ModId::PitchLFO:
        switch (target.id()) {
        case ModId::Pitch:
            return ModKey::createNXYZ(ModId::PitchLFODepth, region);
        default:
            break;
        }
        break;

    case ModId::FilLFO:
        switch (target.id()) {
        case ModId::FilCutoff:
            return ModKey::createNXYZ(ModId::FilLFODepth, region);
        default:
            break;
        }
        break;

    case ModId::PitchEG:
        switch (target.id()) {
        case ModId::Pitch:
            return ModKey::createNXYZ(ModId::PitchEGDepth, region);
        default:
            break;
        }
        break;

    case ModId::FilEG:
        switch (target.id()) {
        case ModId::FilCutoff:
            return ModKey::createNXYZ(ModId::FilEGDepth, region);
        default:
            break;
        }
        break;

    case ModId::LFO:
        switch (target.id()) {
        case ModId::Amplitude:
            return ModKey::createNXYZ(ModId::LFOAmplitudeDepth, region, tp.N);
        case ModId::Pan:
            return ModKey::createNXYZ(ModId::LFOPanDepth, region, tp.N);
        case ModId::Width:
            return ModKey::createNXYZ(ModId::LFOWidthDepth, region, tp.N);
        case ModId::Position:
            return ModKey::createNXYZ(ModId::LFOPositionDepth, region, tp.N);
        case ModId::Pitch:
            return ModKey::createNXYZ(ModId::LFOPitchDepth, region, tp.N);
        case ModId::Volume:
            return ModKey::createNXYZ(ModId::LFOVolumeDepth, region, tp.N);
        case ModId::FilCutoff:
            return ModKey::createNXYZ(ModId::LFOFilCutoffDepth, region, tp.N, tp.X);
        case ModId::FilResonance:
            return ModKey::createNXYZ(ModId::LFOFilResonanceDepth, region, tp.N, tp.X);
        case ModId::FilGain:
            return ModKey::createNXYZ(ModId::LFOFilGainDepth, region, tp.N, tp.X);
        case ModId::EqGain:
            return ModKey::createNXYZ(ModId::LFOEqGainDepth, region, tp.N, tp.X);
        case ModId::EqFrequency:
            return ModKey::createNXYZ(ModId::LFOEqFrequencyDepth, region, tp.N, tp.X);
        case ModId::EqBandwidth:
            return ModKey::createNXYZ(ModId::LFOEqBandwidthDepth, region, tp.N, tp.X);
        default:
            break;
        }
        break;

    case ModId::Envelope:
        switch (target.id()) {
        case ModId::Amplitude:
            return ModKey::createNXYZ(ModId::EGAmplitudeDepth, region, tp.N);
        case ModId::Pan:
            return ModKey::createNXYZ(ModId::EGPanDepth, region, tp.N);
        case ModId::Width:
            return ModKey::createNXYZ(ModId::EGWidthDepth, region, tp.N);
        case ModId::Position:
            return ModKey::createNXYZ(ModId::EGPositionDepth, region, tp.N);
        case ModId::Pitch:
            return ModKey::createNXYZ(ModId::EGPitchDepth, region, tp.N);
        case ModId::Volume:
            return ModKey::createNXYZ(ModId::EGVolumeDepth, region, tp.N);
        case ModId::FilCutoff:
            return ModKey::createNXYZ(ModId::EGFilCutoffDepth, region, tp.N, tp.X);
        case ModId::FilResonance:
            return ModKey::createNXYZ(ModId::EGFilResonanceDepth, region, tp.N, tp.X);
        case ModId::FilGain:
            return ModKey::createNXYZ(ModId::EGFilGainDepth, region, tp.N, tp.X);
        case ModId::EqGain:
            return ModKey::createNXYZ(ModId::EGEqGainDepth, region, tp.N, tp.X);
        case ModId::EqFrequency:
            return ModKey::createNXYZ(ModId::EGEqFrequencyDepth, region, tp.N, tp.X);
        case ModId::EqBandwidth:
            return ModKey::createNXYZ(ModId::EGEqBandwidthDepth, region, tp.N, tp.X);
        default:
            break;
        }
        break;

    default:
        break;
    }

    return {};
}

} // namespace sfz

