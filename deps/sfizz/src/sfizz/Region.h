// SPDX-License-Identifier: BSD-2-Clause

// This code is part of the sfizz library and is licensed under a BSD 2-clause
// license. You should have receive a LICENSE.md file along with the code.
// If not, contact the sfizz maintainers at https://github.com/sfztools/sfizz

#pragma once
#include "CCMap.h"
#include "Curve.h"
#include "Defaults.h"
#include "EGDescription.h"
#include "FlexEGDescription.h"
#include "EQDescription.h"
#include "FilterDescription.h"
#include "LFODescription.h"
#include "Opcode.h"
#include "AudioBuffer.h"
#include "FileId.h"
#include "utility/NumericId.h"
#include "utility/LeakDetector.h"
#include "modulations/ModKey.h"
#include "absl/types/optional.h"
#include "absl/strings/string_view.h"
#include <bitset>
#include <string>
#include <vector>

namespace sfz {

class RegionSet;
class MidiState;

/**
 * @brief Regions are the basic building blocks for the SFZ parsing and handling code.
 * All SFZ files are made of regions that are activated when a key is pressed or a CC
 * is triggered. Most opcodes constrain the situations in which a region can be activated.
 * Once activated, the Synth object will find a voice to play the region.
 *
 * This class is mostly open as there are a ton of parameters needed for the voice to be
 * able to play the region, and using getters would incur a ton of boilerplate code. Note
 * also that some parameters may be parsed and stored in regions but no playing logi is
 * available in the voices to take advantage of them.
 *
 */
struct Region {
    explicit Region(int regionNumber, absl::string_view defaultPath = "");
    Region(const Region&) = default;
    ~Region() = default;

    /**
     * @brief Get the number which identifies this region
     */
    NumericId<Region> getId() const noexcept
    {
        return id;
    }

    /**
     * @brief Triggers on release?
     *
     * @return true
     * @return false
     */
    bool isRelease() const noexcept { return trigger == Trigger::release || trigger == Trigger::release_key; }
    /**
     * @brief Is a generator (*sine or *silence mostly)?
     *
     * @return true
     * @return false
     */
    bool isGenerator() const noexcept { return sampleId->filename().size() > 0 ? sampleId->filename()[0] == '*' : false; }
    /**
     * @brief Is an oscillator (generator or wavetable)?
     *
     * @return true
     * @return false
     */
    bool isOscillator() const noexcept
    {
        if (isGenerator())
            return true;
        else if (oscillatorEnabled != OscillatorEnabled::Auto)
            return oscillatorEnabled == OscillatorEnabled::On;
        else
            return hasWavetableSample;
    }
    /**
     * @brief Is stereo (has stereo sample or is unison oscillator)?
     *
     * @return true
     * @return false
     */
    bool isStereo() const noexcept { return hasStereoSample || (isOscillator() && oscillatorMulti >= 3); }
    /**
     * @brief Is a looping region (at least potentially)?
     *
     * @return true
     * @return false
     */
    bool shouldLoop() const noexcept { return (loopMode == LoopMode::loop_continuous || loopMode == LoopMode::loop_sustain); }

    /**
     * @brief Get the base gain of the region.
     *
     * @return float
     */
    float getBaseGain() const noexcept;
    /**
     * @brief Get the base gain of the region.
     *
     * @return float
     */
    float getPhase() const noexcept;
    /**
     * @brief Get the detuning in cents for a given bend value between -1 and 1
     *
     * @param bend
     * @return float
     */
    float getBendInCents(float bend) const noexcept;

    /**
     * @brief Parse a new opcode into the region to fill in the proper parameters.
     * This must be called multiple times for each opcode applying to this region.
     *
     * @param opcode
     * @param cleanOpcode whether the opcode should be canonicalized
     * @return true if the opcode was properly read and stored.
     * @return false
     */
    bool parseOpcode(const Opcode& opcode, bool cleanOpcode = true);
    /**
     * @brief Parse a opcode which is specific to a particular SFZv1 LFO:
     * amplfo, pitchlfo, fillfo.
     *
     * @param opcode
     * @param lfo
     * @return true if the opcode was properly read and stored.
     * @return false
     */
    bool parseLFOOpcode(const Opcode& opcode, LFODescription& lfo);
    /**
     * @brief Parse a opcode which is specific to a particular SFZv1 LFO:
     * amplfo, pitchlfo, fillfo.
     *
     * @param opcode
     * @param lfo
     * @return true if the opcode was properly read and stored.
     * @return false
     */
    bool parseLFOOpcode(const Opcode& opcode, absl::optional<LFODescription>& lfo);
    /**
     * @brief Parse a opcode which is specific to a particular SFZv1 EG:
     * ampeg, pitcheg, fileg.
     *
     * @param opcode
     * @param eg
     * @return true if the opcode was properly read and stored.
     * @return false
     */
    bool parseEGOpcode(const Opcode& opcode, EGDescription& eg);
    /**
     * @brief Parse a opcode which is specific to a particular SFZv1 EG:
     * ampeg, pitcheg, fileg.
     *
     * @param opcode
     * @param eg
     * @return true if the opcode was properly read and stored.
     * @return false
     */
    bool parseEGOpcode(const Opcode& opcode, absl::optional<EGDescription>& eg);
    /**
     * @brief Parse a opcode which is specific to a particular SFZv2 LFO: lfoN.
     *
     * @param opcode
     * @return true if the opcode was properly read and stored.
     * @return false
     */
    bool parseLFOOpcodeV2(const Opcode& opcode);
    /**
     * @brief Parse a opcode which is specific to a particular SFZv2 EG: egN.
     *
     * @param opcode
     * @return true if the opcode was properly read and stored.
     * @return false
     */
    bool parseEGOpcodeV2(const Opcode& opcode);
    /**
     * @brief Process a generic CC opcode, and fill the modulation parameters.
     *
     * @param opcode
     * @param spec
     * @param target
     * @return true if the opcode was properly read and stored.
     * @return false
     */
    bool processGenericCc(const Opcode& opcode, OpcodeSpec<float> spec, const ModKey& target);

    void offsetAllKeys(int offset) noexcept;

    /**
     * @brief Get the gain this region contributes into the input of the Nth
     *        effect bus
     */
    float getGainToEffectBus(unsigned number) const noexcept;

    /**
     * @brief Check if a region is disabled, if its sample end is weakly negative for example.
     */
    bool disabled() const noexcept;

    /**
     * @brief Extract the source depth of the unique connection identified
     *        by a given CC and NXYZ target.
     *
     * @param cc  the CC number of the modulation source
     * @param id  the ID of the modulation target, which must be regional
     * @return absl::optional<float>
     */
    absl::optional<float> ccModDepth(int cc, ModId id, uint8_t N = 0, uint8_t X = 0, uint8_t Y = 0, uint8_t Z = 0) const noexcept;

    /**
     * @brief Extract the source parameters of the unique connection identified
     *        by a given CC and NXYZ target.
     *
     * @param cc the CC number of the modulation source
     * @param cc  the CC number of the modulation source
     * @param id  the ID of the modulation target, which must be regional
     * @return absl::optional<ModKey::Parameters>
     */
    absl::optional<ModKey::Parameters> ccModParameters(int cc, ModId id, uint8_t N = 0, uint8_t X = 0, uint8_t Y = 0, uint8_t Z = 0) const noexcept;

    const NumericId<Region> id;

    // Sound source: sample playback
    std::shared_ptr<FileId> sampleId { new FileId }; // Sample
    absl::optional<int> sampleQuality {};
    float delay { Default::delay }; // delay
    float delayRandom { Default::delayRandom }; // delay_random
    CCMap<float> delayCC { Default::delayMod };
    int64_t offset { Default::offset }; // offset
    int64_t offsetRandom { Default::offsetRandom }; // offset_random
    CCMap<int64_t> offsetCC { Default::offsetMod };
    int64_t sampleEnd { Default::sampleEnd }; // end
    CCMap<int64_t> endCC { Default::sampleEndMod };
    absl::optional<uint32_t> sampleCount {}; // count
    absl::optional<LoopMode> loopMode {}; // loopmode
    UncheckedRange<int64_t> loopRange { Default::loopStart, Default::loopEnd }; //loopstart and loopend
    CCMap<int64_t> loopStartCC { Default::sampleEndMod };
    CCMap<int64_t> loopEndCC { Default::sampleEndMod };
    absl::optional<uint32_t> loopCount {}; // count
    float loopCrossfade { Default::loopCrossfade }; // loop_crossfade

    // Wavetable oscillator
    float oscillatorPhase { Default::oscillatorPhase };
    OscillatorEnabled oscillatorEnabled { Default::oscillator }; // oscillator
    bool hasWavetableSample { false }; // (set according to sample file)
    int oscillatorMode { Default::oscillatorMode };
    int oscillatorMulti { Default::oscillatorMulti };
    float oscillatorDetune { Default::oscillatorDetune };
    float oscillatorModDepth { Default::oscillatorModDepth };
    absl::optional<int> oscillatorQuality;

    // Instrument settings: voice lifecycle
    int64_t group { Default::group }; // group
    uint16_t output { Default::output }; // output
    absl::optional<int64_t> offBy {}; // off_by
    OffMode offMode { Default::offMode }; // off_mode
    float offTime { Default::offTime }; // off_mode
    absl::optional<uint32_t> notePolyphony {}; // note_polyphony
    uint32_t polyphony { config::maxVoices }; // polyphony
    SelfMask selfMask { Default::selfMask };
    bool rtDead { Default::rtDead };

    // Region logic: key mapping
    UncheckedRange<uint8_t> keyRange { Default::loKey, Default::hiKey }; //lokey, hikey and key
    UncheckedRange<float> velocityRange { Default::loVel, Default::hiVel }; // hivel and lovel

    // Region logic: MIDI conditions
    UncheckedRange<float> bendRange { Default::loBend, Default::hiBend }; // hibend and lobend
    UncheckedRange<uint8_t> programRange { Default::loProgram, Default::hiProgram }; // loprog and hiprog
    CCMap<UncheckedRange<float>> ccConditions {{ Default::loCC, Default::hiCC }};
    absl::optional<uint8_t> lastKeyswitch {}; // sw_last
    absl::optional<UncheckedRange<uint8_t>> lastKeyswitchRange {}; // sw_last
    absl::optional<std::string> keyswitchLabel {};
    absl::optional<uint8_t> upKeyswitch {}; // sw_up
    absl::optional<uint8_t> downKeyswitch {}; // sw_down
    absl::optional<uint8_t> previousKeyswitch {}; // sw_previous
    absl::optional<uint8_t> defaultSwitch {};
    VelocityOverride velocityOverride { Default::velocityOverride }; // sw_vel
    bool checkSustain { Default::checkSustain }; // sustain_sw
    bool checkSostenuto { Default::checkSostenuto }; // sostenuto_sw
    uint16_t sustainCC { Default::sustainCC }; // sustain_cc
    uint16_t sostenutoCC { Default::sostenutoCC }; // sustain_cc
    float sustainThreshold { Default::sustainThreshold }; // sustain_cc
    float sostenutoThreshold { Default::sostenutoThreshold }; // sustain_cc

    bool usesKeySwitches { false };
    bool usesPreviousKeySwitches { false };

    // Region logic: internal conditions
    UncheckedRange<float> aftertouchRange { Default::loChannelAftertouch, Default::hiChannelAftertouch }; // hichanaft and lochanaft
    UncheckedRange<float> polyAftertouchRange { Default::loPolyAftertouch, Default::hiPolyAftertouch }; // hipolyaft and lopolyaft
    UncheckedRange<float> bpmRange { Default::loBPM, Default::hiBPM }; // hibpm and lobpm
    UncheckedRange<float> randRange { Default::loNormalized, Default::hiNormalized }; // hirand and lorand
    uint8_t sequenceLength { Default::sequence }; // seq_length
    uint8_t sequencePosition { Default::sequence }; // seq_position

    bool usesSequenceSwitches { false };

    // Region logic: triggers
    Trigger trigger { Default::trigger }; // trigger
    CCMap<UncheckedRange<float>> ccTriggers {{ Default::loCC, Default::hiCC }}; // on_loccN on_hiccN

    // Performance parameters: amplifier
    float volume { Default::volume }; // volume
    float amplitude { Default::amplitude }; // amplitude
    float pan { Default::pan }; // pan
    float width { Default::width }; // width
    float position { Default::position }; // position
    uint8_t ampKeycenter { Default::key }; // amp_keycenter
    float ampKeytrack { Default::ampKeytrack }; // amp_keytrack
    float ampVeltrack { Default::ampVeltrack }; // amp_veltrack
    CCMap<ModifierCurvePair<float>> ampVeltrackCC { ModifierCurvePair<float>{ Default::ampVeltrackMod, Default::curveCC } };
    std::vector<std::pair<uint8_t, float>> velocityPoints; // amp_velcurve_N
    absl::optional<Curve> velCurve {};
    float ampRandom { Default::ampRandom }; // amp_random
    UncheckedRange<uint8_t> crossfadeKeyInRange { Default::crossfadeKeyInRange };
    UncheckedRange<uint8_t> crossfadeKeyOutRange { Default::crossfadeKeyOutRange };
    UncheckedRange<float> crossfadeVelInRange { Default::crossfadeVelInRange };
    UncheckedRange<float> crossfadeVelOutRange { Default::crossfadeVelOutRange };
    CrossfadeCurve crossfadeKeyCurve { Default::crossfadeCurve };
    CrossfadeCurve crossfadeVelCurve { Default::crossfadeCurve };
    CrossfadeCurve crossfadeCCCurve { Default::crossfadeCurve };
    CCMap<UncheckedRange<float>> crossfadeCCInRange { Default::crossfadeCCInRange }; // xfin_loccN xfin_hiccN
    CCMap<UncheckedRange<float>> crossfadeCCOutRange { Default::crossfadeCCOutRange }; // xfout_loccN xfout_hiccN
    float rtDecay { Default::rtDecay }; // rt_decay

    float globalAmplitude { 1.0 }; // global_amplitude
    float masterAmplitude { 1.0 }; // master_amplitude
    float groupAmplitude { 1.0 }; // group_amplitude
    float globalVolume { 0.0 }; // global_volume
    float masterVolume { 0.0 }; // master_volume
    float groupVolume { 0.0 }; // group_volume

    // Filters and EQs
    std::vector<EQDescription> equalizers;
    std::vector<FilterDescription> filters;

    // Performance parameters: pitch
    uint8_t pitchKeycenter { Default::key }; // pitch_keycenter
    bool pitchKeycenterFromSample { false };
    float pitchKeytrack { Default::pitchKeytrack }; // pitch_keytrack
    float pitchRandom { Default::pitchRandom }; // pitch_random
    float pitchVeltrack { Default::pitchVeltrack }; // pitch_veltrack
    CCMap<ModifierCurvePair<float>> pitchVeltrackCC { ModifierCurvePair<float>{ Default::pitchVeltrackMod, Default::curveCC } };
    float transpose { Default::transpose }; // transpose
    float pitch { Default::pitch }; // tune
    float bendUp { Default::bendUp };
    float bendDown { Default::bendDown };
    float bendStep { Default::bendStep };
    uint16_t bendSmooth { Default::smoothCC };

    // Envelopes
    EGDescription amplitudeEG;
    absl::optional<EGDescription> pitchEG;
    absl::optional<EGDescription> filterEG;

    // Envelopes
    std::vector<FlexEGDescription> flexEGs;
    absl::optional<uint8_t> flexAmpEG; // egN_ampeg

    // LFOs
    std::vector<LFODescription> lfos;
    absl::optional<LFODescription> amplitudeLFO;
    absl::optional<LFODescription> pitchLFO;
    absl::optional<LFODescription> filterLFO;

    bool hasStereoSample { false };

    // Effects
    std::vector<float> gainToEffect;

    bool triggerOnCC { false }; // whether the region triggers on CC events or note events
    bool triggerOnNote { true };

    // Modulation matrix connections
    struct Connection {
        ModKey source;
        ModKey target;
        float sourceDepth = 0.0f;
        ModKey sourceDepthMod;
        float velToDepth = 0.0f;
    };
    std::vector<Connection> connections;
    Connection* getConnection(const ModKey& source, const ModKey& target);
    Connection& getOrCreateConnection(const ModKey& source, const ModKey& target);
    Connection* getConnectionFromCC(int sourceCC, const ModKey& target);

    // Parent
    RegionSet* parent { nullptr };

    std::string defaultPath;

    LEAK_DETECTOR(Region);
};

} // namespace sfz
