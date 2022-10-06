// SPDX-License-Identifier: BSD-2-Clause

// Copyright (c) 2019-2020, Paul Ferrand, Andrea Zanellato
// All rights reserved.

// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are met:

// 1. Redistributions of source code must retain the above copyright notice, this
//    list of conditions and the following disclaimer.
// 2. Redistributions in binary form must reproduce the above copyright notice,
//    this list of conditions and the following disclaimer in the documentation
//    and/or other materials provided with the distribution.

// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
// ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
// WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
// DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR
// ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
// (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
// LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
// ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
// (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
// SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

#pragma once
#include <limits>
#include <cstdint>
#include <type_traits>
#include "Range.h"
#include "Config.h"
#include "SfzFilter.h"
#include "SfzHelpers.h"
#include "LFOCommon.h"
#include "MathHelpers.h"
#include "utility/Macros.h"


namespace sfz
{

enum class Trigger { attack = 0, release, release_key, first, legato };
enum class LoopMode { no_loop = 0, one_shot, loop_continuous, loop_sustain };
enum class OffMode { fast = 0, normal, time };
enum class VelocityOverride { current = 0, previous };
enum class CrossfadeCurve { gain = 0, power };
enum class SelfMask { mask = 0, dontMask };
enum class OscillatorEnabled { Auto = -1, Off = 0, On = 1 };

enum OpcodeFlags : int {
    kCanBeNote = 1,
    kEnforceLowerBound = 1 << 1,
    kEnforceUpperBound = 1 << 2,
    kEnforceBounds = kEnforceLowerBound|kEnforceUpperBound,
    kPermissiveLowerBound = 1 << 3,
    kPermissiveUpperBound = 1 << 4,
    kPermissiveBounds = kPermissiveLowerBound|kPermissiveUpperBound,
    kNormalizePercent = 1 << 5,
    kNormalizeMidi = 1 << 6,
    kNormalizeBend = 1 << 7,
    kWrapPhase = 1 << 8,
    kDb2Mag = 1 << 9,
    kFillGap = 1 << 10, // Fill in the gap when converting from discrete midi values to float, so that 13 is actually 13.999999...
};

template<class T>
struct OpcodeSpec
{
    T defaultInputValue;
    Range<T> bounds;
    int flags;

    using Intermediate = typename std::conditional<
        std::is_integral<T>::value || std::is_enum<T>::value, int64_t, T>::type;

    template <class U>
    using IsNormalizable = std::integral_constant<
        bool, std::is_arithmetic<U>::value && !std::is_same<U, bool>::value>;

    /**
     * @brief Normalizes an input as needed for the spec
     *
     * @tparam U
     * @param input
     * @return U
     */
    template<class U=T>
    typename std::enable_if<IsNormalizable<U>::value, U>::type normalizeInput(U input) const
    {
        constexpr int needsOperation {
            kNormalizePercent |
            kNormalizeMidi |
            kNormalizeBend |
            kDb2Mag
        };

        if (!(flags & needsOperation))
            return input;
        else if (flags & kNormalizePercent)
            return static_cast<U>(input / U(100));
        else if (flags & kNormalizeMidi) {
            if ((flags & kFillGap) && (input <= U(126)) && input >= 0)
                return std::nextafter(static_cast<U>((input + 1.0f) / U(127)), 0.0f);
            else
                return static_cast<U>(input / U(127));
        }
        else if (flags & kNormalizeBend)
            return static_cast<U>(input / U(8191));
        else if (flags & kDb2Mag)
            return static_cast<U>(db2mag(input));
        else // just in case
            return input;
    }

    /**
     * @brief Normalizes an input as needed for the spec
     *
     * @tparam U
     * @param input
     * @return U
     */
    template<class U=T>
    typename std::enable_if<!IsNormalizable<U>::value, U>::type normalizeInput(U input) const
    {
        return input;
    }

    operator T() const { return normalizeInput(defaultInputValue); }
};

namespace Default
{
    extern const OpcodeSpec<float> delay;
    extern const OpcodeSpec<float> delayRandom;
    extern const OpcodeSpec<float> delayMod;
    extern const OpcodeSpec<int64_t> offset;
    extern const OpcodeSpec<int64_t> offsetMod;
    extern const OpcodeSpec<int64_t> offsetRandom;
    extern const OpcodeSpec<int64_t> sampleEnd;
    extern const OpcodeSpec<int64_t> sampleEndMod;
    extern const OpcodeSpec<uint32_t> sampleCount;
    extern const OpcodeSpec<int64_t> loopStart;
    extern const OpcodeSpec<int64_t> loopEnd;
    extern const OpcodeSpec<int64_t> loopMod;
    extern const OpcodeSpec<uint32_t> loopCount;
    extern const OpcodeSpec<float> loopCrossfade;
    extern const OpcodeSpec<float> oscillatorPhase;
    extern const OpcodeSpec<OscillatorEnabled> oscillator;
    extern const OpcodeSpec<int32_t> oscillatorMode;
    extern const OpcodeSpec<int32_t> oscillatorMulti;
    extern const OpcodeSpec<float> oscillatorDetune;
    extern const OpcodeSpec<float> oscillatorDetuneMod;
    extern const OpcodeSpec<float> oscillatorModDepth;
    extern const OpcodeSpec<float> oscillatorModDepthMod;
    extern const OpcodeSpec<int32_t> oscillatorQuality;
    extern const OpcodeSpec<int64_t> group;
    extern const OpcodeSpec<uint16_t> output;
    extern const OpcodeSpec<float> offTime;
    extern const OpcodeSpec<uint32_t> polyphony;
    extern const OpcodeSpec<uint32_t> notePolyphony;
    extern const OpcodeSpec<uint8_t> key;
    extern const OpcodeSpec<uint8_t> loKey;
    extern const OpcodeSpec<uint8_t> hiKey;
    extern const OpcodeSpec<float> loVel;
    extern const OpcodeSpec<float> hiVel;
    extern const OpcodeSpec<float> loCC;
    extern const OpcodeSpec<float> hiCC;
    extern const OpcodeSpec<float> xfoutLo;
    extern const OpcodeSpec<float> xfoutHi;
    extern const OpcodeSpec<float> xfinHi;
    extern const OpcodeSpec<float> xfinLo;
    extern const OpcodeSpec<float> loBend;
    extern const OpcodeSpec<float> hiBend;
    extern const OpcodeSpec<uint8_t> loProgram;
    extern const OpcodeSpec<uint8_t> hiProgram;
    extern const OpcodeSpec<float> loNormalized;
    extern const OpcodeSpec<float> hiNormalized;
    extern const OpcodeSpec<float> loBipolar;
    extern const OpcodeSpec<float> hiBipolar;
    extern const OpcodeSpec<float> loChannelAftertouch;
    extern const OpcodeSpec<float> hiChannelAftertouch;
    extern const OpcodeSpec<float> loPolyAftertouch;
    extern const OpcodeSpec<float> hiPolyAftertouch;
    extern const OpcodeSpec<uint16_t> ccNumber;
    extern const OpcodeSpec<uint8_t> curveCC;
    extern const OpcodeSpec<uint16_t> smoothCC;
    extern const OpcodeSpec<uint8_t> sustainCC;
    extern const OpcodeSpec<uint8_t> sostenutoCC;
    extern const OpcodeSpec<bool> checkSustain;
    extern const OpcodeSpec<bool> checkSostenuto;
    extern const OpcodeSpec<float> sustainThreshold;
    extern const OpcodeSpec<float> sostenutoThreshold;
    extern const OpcodeSpec<float> loBPM;
    extern const OpcodeSpec<float> hiBPM;
    extern const OpcodeSpec<uint8_t> sequence;
    extern const OpcodeSpec<float> volume;
    extern const OpcodeSpec<float> volumeMod;
    extern const OpcodeSpec<float> amplitude;
    extern const OpcodeSpec<float> amplitudeMod;
    extern const OpcodeSpec<float> pan;
    extern const OpcodeSpec<float> panMod;
    extern const OpcodeSpec<float> position;
    extern const OpcodeSpec<float> positionMod;
    extern const OpcodeSpec<float> width;
    extern const OpcodeSpec<float> widthMod;
    extern const OpcodeSpec<float> ampKeytrack;
    extern const OpcodeSpec<float> ampVeltrack;
    extern const OpcodeSpec<float> ampVeltrackMod;
    extern const OpcodeSpec<float> ampVelcurve;
    extern const OpcodeSpec<float> ampRandom;
    extern const OpcodeSpec<bool> rtDead;
    extern const OpcodeSpec<float> rtDecay;
    extern const OpcodeSpec<float> filterCutoff;
    extern const OpcodeSpec<float> filterCutoffMod;
    extern const OpcodeSpec<float> filterResonance;
    extern const OpcodeSpec<float> filterResonanceMod;
    extern const OpcodeSpec<float> filterGain;
    extern const OpcodeSpec<float> filterGainMod;
    extern const OpcodeSpec<float> filterRandom;
    extern const OpcodeSpec<float> filterKeytrack;
    extern const OpcodeSpec<float> filterVeltrack;
    extern const OpcodeSpec<float> filterVeltrackMod;
    extern const OpcodeSpec<float> eqBandwidth;
    extern const OpcodeSpec<float> eqBandwidthMod;
    extern const OpcodeSpec<float> eqFrequency;
    extern const OpcodeSpec<float> eqFrequencyMod;
    extern const OpcodeSpec<float> eqGain;
    extern const OpcodeSpec<float> eqGainMod;
    extern const OpcodeSpec<float> eqVel2Frequency;
    extern const OpcodeSpec<float> eqVel2Gain;
    extern const OpcodeSpec<float> pitchKeytrack;
    extern const OpcodeSpec<float> pitchRandom;
    extern const OpcodeSpec<float> pitchVeltrack;
    extern const OpcodeSpec<float> pitchVeltrackMod;
    extern const OpcodeSpec<float> transpose;
    extern const OpcodeSpec<float> pitch;
    extern const OpcodeSpec<float> pitchMod;
    extern const OpcodeSpec<float> bendUp;
    extern const OpcodeSpec<float> bendDown;
    extern const OpcodeSpec<float> bendStep;
    extern const OpcodeSpec<float> ampLFODepth;
    extern const OpcodeSpec<float> pitchLFODepth;
    extern const OpcodeSpec<float> filLFODepth;
    extern const OpcodeSpec<float> lfoFreq;
    extern const OpcodeSpec<float> lfoFreqMod;
    extern const OpcodeSpec<float> lfoBeats;
    extern const OpcodeSpec<float> lfoBeatsMod;
    extern const OpcodeSpec<float> lfoPhase;
    extern const OpcodeSpec<float> lfoPhaseMod;
    extern const OpcodeSpec<float> lfoDelay;
    extern const OpcodeSpec<float> lfoDelayMod;
    extern const OpcodeSpec<float> lfoFade;
    extern const OpcodeSpec<float> lfoFadeMod;
    extern const OpcodeSpec<uint32_t> lfoCount;
    extern const OpcodeSpec<uint32_t> lfoSteps;
    extern const OpcodeSpec<float> lfoStepX;
    extern const OpcodeSpec<LFOWave> lfoWave;
    extern const OpcodeSpec<float> lfoOffset;
    extern const OpcodeSpec<float> lfoRatio;
    extern const OpcodeSpec<float> lfoScale;
    extern const OpcodeSpec<float> egTime;
    extern const OpcodeSpec<float> egRelease;
    extern const OpcodeSpec<float> egTimeMod;
    extern const OpcodeSpec<float> egSustain;
    extern const OpcodeSpec<float> egPercent;
    extern const OpcodeSpec<float> egPercentMod;
    extern const OpcodeSpec<float> egDepth;
    extern const OpcodeSpec<float> egVel2Depth;
    extern const OpcodeSpec<bool> egDynamic;
    extern const OpcodeSpec<bool> flexEGAmpeg;
    extern const OpcodeSpec<bool> flexEGDynamic;
    extern const OpcodeSpec<int32_t> flexEGSustain;
    extern const OpcodeSpec<float> flexEGPointTime;
    extern const OpcodeSpec<float> flexEGPointTimeMod;
    extern const OpcodeSpec<float> flexEGPointLevel;
    extern const OpcodeSpec<float> flexEGPointLevelMod;
    extern const OpcodeSpec<float> flexEGPointShape;
    extern const OpcodeSpec<int32_t> sampleQuality;
    extern const OpcodeSpec<int32_t> freewheelingSampleQuality;
    extern const OpcodeSpec<int32_t> freewheelingOscillatorQuality;
    extern const OpcodeSpec<int32_t> octaveOffset;
    extern const OpcodeSpec<int32_t> noteOffset;
    extern const OpcodeSpec<float> effect;
    extern const OpcodeSpec<float> effectPercent;
    extern const OpcodeSpec<LFOWave> apanWaveform;
    extern const OpcodeSpec<float> apanFrequency;
    extern const OpcodeSpec<float> apanPhase;
    extern const OpcodeSpec<float> apanLevel;
    extern const OpcodeSpec<float> distoTone;
    extern const OpcodeSpec<float> distoDepth;
    extern const OpcodeSpec<uint32_t> distoStages;
    extern const OpcodeSpec<float> compAttack;
    extern const OpcodeSpec<float> compRelease;
    extern const OpcodeSpec<float> compThreshold;
    extern const OpcodeSpec<bool> compSTLink;
    extern const OpcodeSpec<float> compRatio;
    extern const OpcodeSpec<float> compGain;
    extern const OpcodeSpec<float> fverbSize;
    extern const OpcodeSpec<float> fverbPredelay;
    extern const OpcodeSpec<float> fverbTone;
    extern const OpcodeSpec<float> fverbDamp;
    extern const OpcodeSpec<float> gateAttack;
    extern const OpcodeSpec<float> gateRelease;
    extern const OpcodeSpec<bool> gateSTLink;
    extern const OpcodeSpec<float> gateHold;
    extern const OpcodeSpec<float> gateThreshold;
    extern const OpcodeSpec<float> lofiBitred;
    extern const OpcodeSpec<float> lofiDecim;
    extern const OpcodeSpec<float> rectify;
    extern const OpcodeSpec<uint32_t> stringsNumber;
    extern const OpcodeSpec<Trigger> trigger;
    extern const OpcodeSpec<OffMode> offMode;
    extern const OpcodeSpec<LoopMode> loopMode;
    extern const OpcodeSpec<CrossfadeCurve> crossfadeCurve;
    extern const OpcodeSpec<VelocityOverride> velocityOverride;
    extern const OpcodeSpec<SelfMask> selfMask;
    extern const OpcodeSpec<FilterType> filter;
    extern const OpcodeSpec<EqType> eq;
    extern const OpcodeSpec<bool> sustainCancelsRelease;

    // Default/max count for objects
    constexpr int numEQs { 3 };
    constexpr int numFilters { 2 };
    constexpr int numFlexEGs { 4 };
    constexpr int numFlexEGPoints { 8 };
    constexpr int numLFOs { 4 };
    constexpr int numLFOSubs { 2 };
    constexpr int numLFOSteps { 8 };
    constexpr int maxDistoStages { 4 };
    constexpr unsigned maxStrings { 88 };

    // Default values for ranges
    constexpr Range<uint8_t> crossfadeKeyInRange { 0, 0 };
    constexpr Range<uint8_t> crossfadeKeyOutRange { 127, 127 };
    constexpr Range<float> crossfadeVelInRange { 0.0f, 0.0f };
    constexpr Range<float> crossfadeVelOutRange { 1.0f, 1.0f };
    constexpr Range<float> crossfadeCCInRange { 0.0f, 0.0f };
    constexpr Range<float> crossfadeCCOutRange { 1.0f, 1.0f };

    // Various defaut values
    // e.g. "additional" or multiple defautl values
    constexpr float globalVolume { -7.35f };
    constexpr float defaultEQFreq [numEQs] { 50.0f, 500.0f, 5000.0f };
} // namespace Default

} // namespace sfz
