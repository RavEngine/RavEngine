// SPDX-License-Identifier: BSD-2-Clause

// This code is part of the sfizz library and is licensed under a BSD 2-clause
// license. You should have receive a LICENSE.md file along with the code.
// If not, contact the sfizz maintainers at https://github.com/sfztools/sfizz

#include "MidiState.h"
#include "utility/Macros.h"
#include "utility/Debug.h"

sfz::MidiState::MidiState()
{
    resetEventStates();
    resetNoteStates();
}

void sfz::MidiState::noteOnEvent(int delay, int noteNumber, float velocity) noexcept
{
    ASSERT(noteNumber >= 0 && noteNumber <= 127);
    ASSERT(velocity >= 0 && velocity <= 1.0);

    if (noteNumber >= 0 && noteNumber < 128) {
        float keydelta { 0 };

        if (lastNotePlayed >= 0) {
            keydelta = static_cast<float>(noteNumber - lastNotePlayed);
            velocityOverride = lastNoteVelocities[lastNotePlayed];
        }

        lastNoteVelocities[noteNumber] = velocity;
        noteOnTimes[noteNumber] = internalClock + static_cast<unsigned>(delay);
        lastNotePlayed = noteNumber;
        noteStates[noteNumber] = true;
        ccEvent(delay, ExtendedCCs::noteOnVelocity, velocity);
        ccEvent(delay, ExtendedCCs::keyboardNoteNumber, normalize7Bits(noteNumber));
        ccEvent(delay, ExtendedCCs::unipolarRandom, unipolarDist(Random::randomGenerator));
        ccEvent(delay, ExtendedCCs::bipolarRandom, bipolarDist(Random::randomGenerator));
        ccEvent(delay, ExtendedCCs::keyboardNoteGate, activeNotes > 0 ? 1.0f : 0.0f);
        ccEvent(delay, ExtendedCCs::keydelta, keydelta);
        ccEvent(delay, ExtendedCCs::absoluteKeydelta, std::abs(keydelta));
        activeNotes++;

        ccEvent(delay, ExtendedCCs::alternate, alternate);
        alternate = alternate == 0.0f ? 1.0f : 0.0f;
    }

}

void sfz::MidiState::noteOffEvent(int delay, int noteNumber, float velocity) noexcept
{
    ASSERT(delay >= 0);
    ASSERT(noteNumber >= 0 && noteNumber <= 127);
    ASSERT(velocity >= 0.0 && velocity <= 1.0);
    UNUSED(velocity);
    if (noteNumber >= 0 && noteNumber < 128) {
        noteOffTimes[noteNumber] = internalClock + static_cast<unsigned>(delay);
        ccEvent(delay, ExtendedCCs::noteOffVelocity, velocity);
        ccEvent(delay, ExtendedCCs::keyboardNoteNumber, normalize7Bits(noteNumber));
        ccEvent(delay, ExtendedCCs::unipolarRandom, unipolarDist(Random::randomGenerator));
        ccEvent(delay, ExtendedCCs::bipolarRandom, bipolarDist(Random::randomGenerator));
        if (activeNotes > 0)
            activeNotes--;
        noteStates[noteNumber] = false;
    }

}

void sfz::MidiState::allNotesOff(int delay) noexcept
{
    for (int note = 0; note < 128; note++)
        noteOffEvent(delay, note, 0.0f);
}

void sfz::MidiState::setSampleRate(float sampleRate) noexcept
{
    this->sampleRate = sampleRate;
    internalClock = 0;
    absl::c_fill(noteOnTimes, 0);
    absl::c_fill(noteOffTimes, 0);
}

void sfz::MidiState::advanceTime(int numSamples) noexcept
{
    internalClock += numSamples;
    flushEvents();
}

void sfz::MidiState::flushEvents() noexcept
{
    auto flushEventVector = [] (EventVector& events) {
        ASSERT(!events.empty()); // CC event vectors should never be empty
        events.front().value = events.back().value;
        events.front().delay = 0;
        events.resize(1);
    };

    for (auto& events : ccEvents)
        flushEventVector(events);

    for (auto& events: polyAftertouchEvents)
        flushEventVector(events);

    flushEventVector(pitchEvents);
    flushEventVector(channelAftertouchEvents);
}


void sfz::MidiState::setSamplesPerBlock(int samplesPerBlock) noexcept
{
    auto updateEventBufferSize = [=] (EventVector& events) {
        events.shrink_to_fit();
        events.reserve(samplesPerBlock);
    };
    this->samplesPerBlock = samplesPerBlock;
    for (auto& events: ccEvents)
        updateEventBufferSize(events);

    for (auto& events: polyAftertouchEvents)
        updateEventBufferSize(events);

    updateEventBufferSize(pitchEvents);
    updateEventBufferSize(channelAftertouchEvents);
}

float sfz::MidiState::getNoteDuration(int noteNumber, int delay) const
{
    ASSERT(noteNumber >= 0 && noteNumber < 128);
    if (noteNumber < 0 || noteNumber >= 128)
        return 0.0f;

#if 0
    if (!noteStates[noteNumber])
        return 0.0f;
#endif

    const unsigned timeInSamples = internalClock + static_cast<unsigned>(delay) - noteOnTimes[noteNumber];
    return static_cast<float>(timeInSamples) / sampleRate;
}

float sfz::MidiState::getNoteVelocity(int noteNumber) const noexcept
{
    ASSERT(noteNumber >= 0 && noteNumber <= 127);

    return lastNoteVelocities[noteNumber];
}

float sfz::MidiState::getVelocityOverride() const noexcept
{
    return velocityOverride;
}

void sfz::MidiState::insertEventInVector(EventVector& events, int delay, float value)
{
    const auto insertionPoint = absl::c_lower_bound(events, delay, MidiEventDelayComparator {});
    if (insertionPoint == events.end() || insertionPoint->delay != delay)
        events.insert(insertionPoint, { delay, value });
    else
        insertionPoint->value = value;
}

void sfz::MidiState::pitchBendEvent(int delay, float pitchBendValue) noexcept
{
    ASSERT(pitchBendValue >= -1.0f && pitchBendValue <= 1.0f);
    insertEventInVector(pitchEvents, delay, pitchBendValue);
}

float sfz::MidiState::getPitchBend() const noexcept
{
    ASSERT(pitchEvents.size() > 0);
    return pitchEvents.back().value;
}

void sfz::MidiState::channelAftertouchEvent(int delay, float aftertouch) noexcept
{
    ASSERT(aftertouch >= -1.0f && aftertouch <= 1.0f);
    insertEventInVector(channelAftertouchEvents, delay, aftertouch);
}

void sfz::MidiState::polyAftertouchEvent(int delay, int noteNumber, float aftertouch) noexcept
{
    ASSERT(aftertouch >= 0.0f && aftertouch <= 1.0f);
    if (noteNumber < 0 || noteNumber >= static_cast<int>(polyAftertouchEvents.size()))
        return;

    insertEventInVector(polyAftertouchEvents[noteNumber], delay, aftertouch);
}

float sfz::MidiState::getChannelAftertouch() const noexcept
{
    ASSERT(channelAftertouchEvents.size() > 0);
    return channelAftertouchEvents.back().value;
}

float sfz::MidiState::getPolyAftertouch(int noteNumber) const noexcept
{
    if (noteNumber < 0 || noteNumber > 127)
        return 0.0f;

    ASSERT(polyAftertouchEvents[noteNumber].size() > 0);
    return polyAftertouchEvents[noteNumber].back().value;
}

void sfz::MidiState::ccEvent(int delay, int ccNumber, float ccValue) noexcept
{
    insertEventInVector(ccEvents[ccNumber], delay, ccValue);
}

float sfz::MidiState::getCCValue(int ccNumber) const noexcept
{
    ASSERT(ccNumber >= 0 && ccNumber < config::numCCs);
    return ccEvents[ccNumber].back().value;
}

float sfz::MidiState::getCCValueAt(int ccNumber, int delay) const noexcept
{
    ASSERT(ccNumber >= 0 && ccNumber < config::numCCs);
    const auto ccEvent = absl::c_lower_bound(
        ccEvents[ccNumber], delay, MidiEventDelayComparator {});
    if (ccEvent != ccEvents[ccNumber].end())
        return ccEvent->value;
    else
        return ccEvents[ccNumber].back().value;
}

void sfz::MidiState::resetNoteStates() noexcept
{
    for (auto& velocity: lastNoteVelocities)
        velocity = 0.0f;

    velocityOverride = 0.0f;
    activeNotes = 0;
    internalClock = 0;
    lastNotePlayed = -1;
    alternate = 0.0f;

    auto setEvents = [] (EventVector& events, float value) {
        events.clear();
        events.push_back({ 0, value });
    };

    setEvents(ccEvents[ExtendedCCs::noteOnVelocity], 0.0f);
    setEvents(ccEvents[ExtendedCCs::keyboardNoteNumber], 0.0f);
    setEvents(ccEvents[ExtendedCCs::unipolarRandom], 0.0f);
    setEvents(ccEvents[ExtendedCCs::bipolarRandom], 0.0f);
    setEvents(ccEvents[ExtendedCCs::keyboardNoteGate], 0.0f);
    setEvents(ccEvents[ExtendedCCs::alternate], 0.0f);

    noteStates.reset();
    absl::c_fill(noteOnTimes, 0);
    absl::c_fill(noteOffTimes, 0);
}

void sfz::MidiState::resetEventStates() noexcept
{
    auto clearEvents = [] (EventVector& events) {
        events.clear();
        events.push_back({ 0, 0.0f });
    };

    for (auto& events : ccEvents)
        clearEvents(events);

   for (auto& events : polyAftertouchEvents)
        clearEvents(events);

    clearEvents(pitchEvents);
    clearEvents(channelAftertouchEvents);
}

const sfz::EventVector& sfz::MidiState::getCCEvents(int ccIdx) const noexcept
{
    if (ccIdx < 0 || ccIdx >= config::numCCs)
        return nullEvent;

    return ccEvents[ccIdx];
}

const sfz::EventVector& sfz::MidiState::getPitchEvents() const noexcept
{
    return pitchEvents;
}

const sfz::EventVector& sfz::MidiState::getChannelAftertouchEvents() const noexcept
{
    return channelAftertouchEvents;
}

const sfz::EventVector& sfz::MidiState::getPolyAftertouchEvents(int noteNumber) const noexcept
{
    if (noteNumber < 0 || noteNumber > 127)
        return nullEvent;

    return polyAftertouchEvents[noteNumber];
}

int sfz::MidiState::getProgram() const noexcept
{
    return currentProgram;
}

void sfz::MidiState::programChangeEvent(int delay, int program) noexcept
{
    ASSERT(program >= 0 && program <= 127);
    currentProgram = program;
}
