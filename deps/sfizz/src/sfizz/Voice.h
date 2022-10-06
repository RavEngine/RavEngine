// SPDX-License-Identifier: BSD-2-Clause

// This code is part of the sfizz library and is licensed under a BSD 2-clause
// license. You should have receive a LICENSE.md file along with the code.
// If not, contact the sfizz maintainers at https://github.com/sfztools/sfizz

#pragma once
#include "ADSREnvelope.h"
#include "TriggerEvent.h"
#include "Region.h"
#include "Resources.h"
#include "AudioSpan.h"
#include "utility/NumericId.h"
#include "utility/LeakDetector.h"
#include <memory>

namespace sfz {
enum InterpolatorModel : int;
class LFO;
class FlexEnvelope;
struct Layer;

struct ExtendedCCValues {
    float unipolar {};
    float bipolar {};
    float noteGate {};
    float alternate {};
    float keydelta {};
};

/**
 * @brief The SFZ voice are the polyphony holders. They get activated by the synth
 * and tasked to play a given region until the end, stopping on note-offs, off-groups
 * or natural sample decay.
 *
 */
class Voice {
public:
    Voice() = delete;
    /**
     * @brief Construct a new voice with the midistate singleton
     *
     * @param voiceNumber
     * @param midiState
     */
    Voice(int voiceNumber, Resources& resources);
    ~Voice();
    Voice(const Voice& other) = delete;
    Voice& operator=(const Voice& other) = delete;
    Voice(Voice&& other) noexcept;
    Voice& operator=(Voice&& other) noexcept;

    /**
     * @brief Get the unique identifier of this voice in a synth
     */
    NumericId<Voice> getId() const noexcept;

    enum class State {
        idle,
        playing,
        cleanMeUp,
    };

    class StateListener {
    public:
        virtual void onVoiceStateChanging(NumericId<Voice> /*id*/, State /*state*/) {}
    };

    /**
     * @brief Return true if the voice is to be cleaned up (zombie state)
     */
    bool toBeCleanedUp() const;

    /**
     * @brief Sets the listener which is called when the voice state changes.
     */
    void setStateListener(StateListener *l) noexcept;

    /**
     * @brief Change the sample rate of the voice. This is used to compute all
     * pitch related transformations so it needs to be propagated from the synth
     * at all times.
     *
     * @param sampleRate
     */
    void setSampleRate(float sampleRate) noexcept;
    /**
     * @brief Set the expected block size. If the block size is not fixed, set an
     * upper bound. The voice will adapt at each callback to the actual number of
     * samples requested but this function will allocate temporary buffers that are
     * needed for proper functioning.
     *
     * @param samplesPerBlock
     */
    void setSamplesPerBlock(int samplesPerBlock) noexcept;
    /**
     * @brief Get the sample rate of the voice.
     *
     * @return float
     */
    float getSampleRate() const noexcept;
    /**
     * @brief Get the expected block size.
     *
     * @return int
     */
    int getSamplesPerBlock() const noexcept;

    /**
     * @brief Start playing a region after a short delay for different triggers (note on, off, cc)
     *
     * @param layer
     * @param delay
     * @param evebt
     * @return bool
     */
    bool startVoice(Layer* layer, int delay, const TriggerEvent& event) noexcept;

    /**
     * @brief Get the sample quality determined by the active region.
     *
     * @return int
     */
    int getCurrentSampleQuality() const noexcept;

    /**
     * @brief Get the oscillator quality determined by the active region.
     *
     * @return int
     */
    int getCurrentOscillatorQuality() const noexcept;

    /**
     * @brief Register a note-off event; this may trigger a release.
     *
     * @param delay
     * @param noteNumber
     * @param velocity
     */
    void registerNoteOff(int delay, int noteNumber, float velocity) noexcept;
    /**
     * @brief Register a CC event; this may trigger a release. If the voice is playing and its
     * region has CC modifiers, it will use this value to compute the CC envelope to apply to the
     * parameter.
     *
     * @param delay
     * @param ccNumber
     * @param ccValue
     */
    void registerCC(int delay, int ccNumber, float ccValue) noexcept;
    /**
     * @brief Register a pitch wheel event; for now this does nothing
     *
     * @param delay
     * @param pitch
     */
    void registerPitchWheel(int delay, float pitch) noexcept;
    /**
     * @brief Register an aftertouch event; for now this does nothing
     *
     * @param delay
     * @param aftertouch
     */
    void registerAftertouch(int delay, float aftertouch) noexcept;

    /**
     * @brief Register a polyphonic aftertouch event; for now this does nothing
     *
     * @param delay
     * @param noteNumber
     * @param aftertouch
     */
    void registerPolyAftertouch(int delay, int noteNumber, float aftertouch) noexcept;

    /**
     * @brief Register a tempo event; for now this does nothing
     *
     * @param delay
     * @param pitch
     */
    void registerTempo(int delay, float secondsPerQuarter) noexcept;
    /**
     * @brief Checks if the voice should be offed by another starting in the group specified.
     * This will trigger the release if true.
     *
     * @param delay
     * @param noteNumber
     * @param group
     * @return true
     * @return false
     */
    bool checkOffGroup(const Region* other, int delay, int noteNumber) noexcept;

    /**
     * @brief Render a block of data for this voice into the span
     *
     * @param buffer
     */
    void renderBlock(AudioSpan<float, 2> buffer) noexcept;

    /**
     * @brief Is the voice free?
     *
     * @return true
     * @return false
     */
    bool isFree() const noexcept;
    /**
     * @brief Is the voice released?
     *
     * @return true
     * @return false
     */
    bool released() const noexcept;
    /**
     * @brief Can the voice be reused (i.e. is it releasing after being killed or free)
     *
     * @return true
     * @return false
     */
    bool offedOrFree() const noexcept;
    /**
     * @brief Get the event that triggered the voice
     *
     * @return int
     */
    const TriggerEvent& getTriggerEvent() const noexcept;

    /**
     * @brief Reset the voice to its initial values
     *
     */
    void reset() noexcept;

    /**
     * @brief Set the next voice in the "sister voice" ring
     * The sister voices are voices that started on the same event.
     * This has to be set by the synth. A voice will remove itself from
     * the ring upon reset.
     *
     * @param voice
     */
    void setNextSisterVoice(Voice* voice) noexcept;

    /**
     * @brief Set the previous voice in the "sister voice" ring
     * The sister voices are voices that started on the same event.
     * This has to be set by the synth. A voice will remove itself from
     * the ring upon reset.
     *
     * @param voice
     */
    void setPreviousSisterVoice(Voice* voice) noexcept;

    /**
     * @brief Get the next sister voice in the ring
     *
     * @return Voice*
     */
    Voice* getNextSisterVoice() const noexcept { return nextSisterVoice_; };

    /**
     * @brief Get the previous sister voice in the ring
     *
     * @return Voice*
     */
    Voice* getPreviousSisterVoice() const noexcept { return previousSisterVoice_; };

    /**
     * @brief Get the mean squared power of the last rendered block. This is used
     * to determine which voice to steal if there are too many notes flying around.
     *
     * @return float
     */
    float getAveragePower() const noexcept;

    /**
     * @brief Enable the power follower
     *
     */

    void enablePowerFollower() noexcept;
    /**
     * @brief Disable the power follower
     *
     */
    void disablePowerFollower() noexcept;

    /**
     * Returns the region that is currently playing. May be null if the voice is not active!
     *
     * @return
     */
    const Region* getRegion() const noexcept;
    /**
     * @brief Get the LFO designated by the given index
     *
     * @param index
     */
    LFO* getLFO(size_t index);
    /**
     * @brief Get the Flex EG designated by the given index
     *
     * @param index
     */
    FlexEnvelope* getFlexEG(size_t index);
    /**
     * @brief Set the max number of filters per voice
     *
     * @param numFilters
     */
    void setMaxFiltersPerVoice(size_t numFilters);
    /**
     * @brief Set the max number of EQs per voice
     *
     * @param numEQs
     */
    void setMaxEQsPerVoice(size_t numEQs);
    /**
     * @brief Set the max number of LFOs per voice
     *
     * @param numLFOs
     */
    void setMaxLFOsPerVoice(size_t numLFOs);
    /**
     * @brief Set the max number of Flex EGs per voice
     *
     * @param numFlexEGs
     */
    void setMaxFlexEGsPerVoice(size_t numFlexEGs);
    /**
     * @brief Set whether SFZv1 pitch EG is enabled on this voice
     *
     * @param havePitchEG
     */
    void setPitchEGEnabledPerVoice(bool havePitchEG);
    /**
     * @brief Set whether SFZv1 filter EG is enabled on this voice
     *
     * @param haveFilterEG
     */
    void setFilterEGEnabledPerVoice(bool haveFilterEG);
    /**
     * @brief Set whether SFZv1 amplitude LFO is enabled on this voice
     *
     * @param haveAmplitudeLFO
     */
    void setAmplitudeLFOEnabledPerVoice(bool haveAmplitudeLFO);
    /**
     * @brief Set whether SFZv1 pitch LFO is enabled on this voice
     *
     * @param havePitchLFO
     */
    void setPitchLFOEnabledPerVoice(bool havePitchLFO);
    /**
     * @brief Set whether SFZv1 filter LFO is enabled on this voice
     *
     * @param haveFilterLFO
     */
    void setFilterLFOEnabledPerVoice(bool haveFilterLFO);
    /**
     * @brief Release the voice after a given delay
     *
     * @param delay
     */
    void release(int delay) noexcept;

    /**
     * @brief Off the voice (steal). This will respect the off mode of the region
     *      and set the envelopes if necessary.
     *
     * @param delay
     * @param fast whether to apply a fast release regardless of the off mode
     */
    void off(int delay, bool fast = false) noexcept;

    /**
     * @brief gets the age of the Voice
     *
     * @return
     */
    int getAge() const noexcept;

    double getLastDataDuration() const noexcept;
    double getLastAmplitudeDuration() const noexcept;
    double getLastFilterDuration() const noexcept;
    double getLastPanningDuration() const noexcept;

    /**
     * @brief Get the SFZv1 amplitude LFO, if existing
     */
    LFO* getAmplitudeLFO();
    /**
     * @brief Get the SFZv1 pitch LFO, if existing
     */
    LFO* getPitchLFO();
    /**
     * @brief Get the SFZv1 filter LFO, if existing
     */
    LFO* getFilterLFO();

    /**
     * @brief Get the SFZv1 amplitude EG, if existing
     */
    ADSREnvelope* getAmplitudeEG();
    /**
     * @brief Get the SFZv1 pitch EG, if existing
     */
    ADSREnvelope* getPitchEG();
    /**
     * @brief Get the SFZv1 filter EG, if existing
     */
    ADSREnvelope* getFilterEG();

    /**
     * @brief Get the trigger event
     */
    const TriggerEvent& getTriggerEvent();

    /**
     * @brief Get the extended CC values
     *
     * @return const ExtendedCCValues&
     */
    const ExtendedCCValues& getExtendedCCValues() const noexcept;

    /**
     * @brief Get the remaining delay before the sample starts, in samples
     *
     * @return int
     */
    int getRemainingDelay() const noexcept;
    /**
     * @brief Get the current position in the source sample
     *
     * @return int
     */
    int getSourcePosition() const noexcept;

public:
    /**
     * @brief Check if the voice already belongs to a sister ring
     */
    bool isInSisterRing() const noexcept { return this != nextSisterVoice_; }

private:
    struct Impl;
    std::unique_ptr<Impl> impl_;

    /**
     * @brief Remove the voice from the sister ring
     *
     */
    void removeVoiceFromRing() noexcept;
    Voice* nextSisterVoice_ { this };
    Voice* previousSisterVoice_ { this };

    LEAK_DETECTOR(Voice);
};

inline bool sisterVoices(const Voice* lhs, const Voice* rhs)
{
    if (lhs->getAge() != rhs->getAge())
        return false;

    const TriggerEvent& lhsTrigger = lhs->getTriggerEvent();
    const TriggerEvent& rhsTrigger = rhs->getTriggerEvent();

    if (lhsTrigger.number != rhsTrigger.number)
        return false;

    if (lhsTrigger.value != rhsTrigger.value)
        return false;

    if (lhsTrigger.type != rhsTrigger.type)
        return false;

    return true;
}

inline bool voiceOrdering(const Voice* lhs, const Voice* rhs)
{
    if (lhs->getAge() != rhs->getAge())
        return lhs->getAge() > rhs->getAge();

    const TriggerEvent& lhsTrigger = lhs->getTriggerEvent();
    const TriggerEvent& rhsTrigger = rhs->getTriggerEvent();

    if (lhsTrigger.number != rhsTrigger.number)
        return lhsTrigger.number < rhsTrigger.number;

    if (lhsTrigger.value != rhsTrigger.value)
        return lhsTrigger.value < rhsTrigger.value;

    if (lhsTrigger.type != rhsTrigger.type)
        return lhsTrigger.type > rhsTrigger.type;

    return false;
}

} // namespace sfz
