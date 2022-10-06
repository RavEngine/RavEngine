// SPDX-License-Identifier: BSD-2-Clause

// This code is part of the sfizz library and is licensed under a BSD 2-clause
// license. You should have receive a LICENSE.md file along with the code.
// If not, contact the sfizz maintainers at https://github.com/sfztools/sfizz

#include "Layer.h"
#include "Region.h"
#include "utility/Debug.h"
#include "utility/SwapAndPop.h"
#include <absl/algorithm/container.h>

namespace sfz {

Layer::Layer(int regionNumber, absl::string_view defaultPath, const MidiState& midiState)
    : midiState_(midiState), region_(regionNumber, defaultPath)
{
    initializeActivations();
}

Layer::Layer(const Region& region, const MidiState& midiState)
    : midiState_(midiState), region_(region)
{
    initializeActivations();
}

Layer::~Layer()
{
}

void Layer::initializeActivations()
{
    const Region& region = region_;

    keySwitched_ = !region.usesKeySwitches;
    previousKeySwitched_ = !region.usesPreviousKeySwitches;
    sequenceSwitched_ = !region.usesSequenceSwitches;
    pitchSwitched_ = true;
    bpmSwitched_ = true;
    aftertouchSwitched_ = true;
    programSwitched_ = true;
    ccSwitched_.set();
}

bool Layer::isSwitchedOn() const noexcept
{
    return keySwitched_ && previousKeySwitched_ && sequenceSwitched_ && pitchSwitched_
        && programSwitched_ && bpmSwitched_ && aftertouchSwitched_ && ccSwitched_.all();
}

bool Layer::registerNoteOn(int noteNumber, float velocity, float randValue) noexcept
{
    ASSERT(velocity >= 0.0f && velocity <= 1.0f);

    const Region& region = region_;

    const bool keyOk = region.keyRange.containsWithEnd(noteNumber);
    if (keyOk) {
        // Sequence activation
        sequenceSwitched_ =
            ((sequenceCounter_++ % region.sequenceLength) == region.sequencePosition - 1);
    }

    const bool polyAftertouchActive =
        region.polyAftertouchRange.containsWithEnd(midiState_.getPolyAftertouch(noteNumber));

    if (!isSwitchedOn() || !polyAftertouchActive)
        return false;

    if (!region.triggerOnNote)
        return false;

    if (region.velocityOverride == VelocityOverride::previous)
        velocity = midiState_.getVelocityOverride();

    const bool velOk = region.velocityRange.containsWithEnd(velocity);
    const bool randOk = region.randRange.contains(randValue) || (randValue >= 1.0f && region.randRange.isValid() && region.randRange.getEnd() >= 1.0f);
    const bool firstLegatoNote = (region.trigger == Trigger::first && midiState_.getActiveNotes() == 1);
    const bool attackTrigger = (region.trigger == Trigger::attack);
    const bool notFirstLegatoNote = (region.trigger == Trigger::legato && midiState_.getActiveNotes() > 1);

    return keyOk && velOk && randOk && (attackTrigger || firstLegatoNote || notFirstLegatoNote);
}

bool Layer::registerNoteOff(int noteNumber, float velocity, float randValue) noexcept
{
    ASSERT(velocity >= 0.0f && velocity <= 1.0f);

    const Region& region = region_;

    const bool polyAftertouchActive =
        region.polyAftertouchRange.containsWithEnd(midiState_.getPolyAftertouch(noteNumber));

    if (!isSwitchedOn() || !polyAftertouchActive)
        return false;

    if (!region.triggerOnNote)
        return false;

    // Prerequisites

    const bool keyOk = region.keyRange.containsWithEnd(noteNumber);
    const bool velOk = region.velocityRange.containsWithEnd(velocity);
    const bool randOk = region.randRange.contains(randValue) || (randValue >= 1.0f && region.randRange.isValid() && region.randRange.getEnd() >= 1.0f);

    if (!(velOk && keyOk && randOk))
        return false;

    // Release logic

    if (region.trigger == Trigger::release_key)
        return true;

    if (region.trigger == Trigger::release) {
        const bool sostenutoed = isNoteSostenutoed(noteNumber);

        if (sostenutoed && !sostenutoPressed_) {
            removeFromSostenutoReleases(noteNumber);
            if (sustainPressed_)
                delaySustainRelease(noteNumber, midiState_.getNoteVelocity(noteNumber));
        }

        if (!sostenutoPressed_ || !sostenutoed) {
            if (sustainPressed_)
                delaySustainRelease(noteNumber, midiState_.getNoteVelocity(noteNumber));
            else
                return true;
        }
    }

    return false;
}

void Layer::updateCCState(int ccNumber, float ccValue) noexcept
{
    const Region& region = region_;

    if (ccNumber == region.sustainCC)
        sustainPressed_ = region.checkSustain && ccValue >= region.sustainThreshold;

    if (ccNumber == region.sostenutoCC) {
        const bool newState = region.checkSostenuto && ccValue >= region.sostenutoThreshold;
        if (!sostenutoPressed_ && newState)
            storeSostenutoNotes();

        if (!newState && sostenutoPressed_)
            delayedSostenutoReleases_.clear();

        sostenutoPressed_ = newState;
    }

    const auto conditions = region.ccConditions.get(ccNumber);

    if (!conditions)
        return;

    ccSwitched_.set(ccNumber, conditions->containsWithEnd(ccValue));
}

bool Layer::registerCC(int ccNumber, float ccValue, float randValue) noexcept
{
    const Region& region = region_;

    updateCCState(ccNumber, ccValue);

    if (!region.triggerOnCC)
        return false;

    const bool randOk = region.randRange.contains(randValue)
        || (randValue >= 1.0f && region.randRange.isValid() && region.randRange.getEnd() >= 1.0f);

    if (!randOk)
        return false;

    if (auto triggerRange = region.ccTriggers.get(ccNumber)) {
        if (!triggerRange->containsWithEnd(ccValue))
            return false;

        sequenceSwitched_ =
            ((sequenceCounter_++ % region.sequenceLength) == region.sequencePosition - 1);

        if (isSwitchedOn() && (ccValue != midiState_.getCCValue(ccNumber)))
            return true;
    }

    return false;
}

void Layer::registerPitchWheel(float pitch) noexcept
{
    pitchSwitched_ = region_.bendRange.containsWithEnd(pitch);
}

void Layer::registerProgramChange(int program) noexcept
{
    programSwitched_ = region_.programRange.containsWithEnd(program);
}

void Layer::registerAftertouch(float aftertouch) noexcept
{
    aftertouchSwitched_ = region_.aftertouchRange.containsWithEnd(aftertouch);
}

void Layer::registerTempo(float secondsPerQuarter) noexcept
{
    const float bpm = 60.0f / secondsPerQuarter;
    bpmSwitched_ = region_.bpmRange.containsWithEnd(bpm);
}

void Layer::delaySustainRelease(int noteNumber, float velocity) noexcept
{
    if (delayedSustainReleases_.size() == delayedSustainReleases_.capacity())
        return;

    delayedSustainReleases_.emplace_back(noteNumber, velocity);
}

void Layer::delaySostenutoRelease(int noteNumber, float velocity) noexcept
{
    if (delayedSostenutoReleases_.size() == delayedSostenutoReleases_.capacity())
        return;

    delayedSostenutoReleases_.emplace_back(noteNumber, velocity);
}

void Layer::removeFromSostenutoReleases(int noteNumber) noexcept
{
    swapAndPopFirst(delayedSostenutoReleases_, [=](const std::pair<int, float>& p) {
        return p.first == noteNumber;
    });
}

void Layer::storeSostenutoNotes() noexcept
{
    ASSERT(delayedSostenutoReleases_.empty());
    const Region& region = region_;
    for (int note = region.keyRange.getStart(); note <= region.keyRange.getEnd(); ++note) {
        if (midiState_.isNotePressed(note))
            delaySostenutoRelease(note, midiState_.getNoteVelocity(note));
    }
}


bool Layer::isNoteSustained(int noteNumber) const noexcept
{
    return absl::c_find_if(delayedSustainReleases_, [=](const std::pair<int, float>& p) {
        return p.first == noteNumber;
    }) != delayedSustainReleases_.end();
}

bool Layer::isNoteSostenutoed(int noteNumber) const noexcept
{
    return absl::c_find_if(delayedSostenutoReleases_, [=](const std::pair<int, float>& p) {
        return p.first == noteNumber;
    }) != delayedSostenutoReleases_.end();
}

} // namespace sfz
