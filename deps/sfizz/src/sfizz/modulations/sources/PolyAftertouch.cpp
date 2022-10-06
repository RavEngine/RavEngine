// SPDX-License-Identifier: BSD-2-Clause

// This code is part of the sfizz library and is licensed under a BSD 2-clause
// license. You should have receive a LICENSE.md file along with the code.
// If not, contact the sfizz maintainers at https://github.com/sfztools/sfizz

#include "PolyAftertouch.h"
#include "../../ModifierHelpers.h"
#include "../../ADSREnvelope.h"

namespace sfz {

PolyAftertouchSource::PolyAftertouchSource(VoiceManager& manager, MidiState& state)
    : midiState_(state), manager_(manager)
{

}

void PolyAftertouchSource::init(const ModKey& sourceKey, NumericId<Voice> voiceId, unsigned delay)
{
    UNUSED(sourceKey);
    UNUSED(voiceId);
    UNUSED(delay);
}

void PolyAftertouchSource::generate(const ModKey& sourceKey, NumericId<Voice> voiceId, absl::Span<float> buffer)
{
    UNUSED(sourceKey);
    Voice* voice = manager_.getVoiceById(voiceId);
    if (!voice || voice->getTriggerEvent().type == TriggerEventType::CC) {
        fill(buffer, 0.0f);
        return;
    }

    const int noteNumber = voice->getTriggerEvent().number;

    const EventVector& events = midiState_.getPolyAftertouchEvents(noteNumber);
    linearEnvelope(events, buffer, [](float x) { return x; });
}

} // namespace sfz
