// SPDX-License-Identifier: BSD-2-Clause

// This code is part of the sfizz library and is licensed under a BSD 2-clause
// license. You should have receive a LICENSE.md file along with the code.
// If not, contact the sfizz maintainers at https://github.com/sfztools/sfizz

#include "ChannelAftertouch.h"
#include "../../ModifierHelpers.h"
#include "../../ADSREnvelope.h"

namespace sfz {

ChannelAftertouchSource::ChannelAftertouchSource(VoiceManager& manager, MidiState& state)
    : midiState_(state)
{
    (void)manager;
}

void ChannelAftertouchSource::init(const ModKey& sourceKey, NumericId<Voice> voiceId, unsigned delay)
{
    UNUSED(sourceKey);
    UNUSED(voiceId);
    UNUSED(delay);
}

void ChannelAftertouchSource::generate(const ModKey& sourceKey, NumericId<Voice> voiceId, absl::Span<float> buffer)
{
    UNUSED(sourceKey);
    UNUSED(voiceId);
    const EventVector& events = midiState_.getChannelAftertouchEvents();
    linearEnvelope(events, buffer, [](float x) { return x; });
}

} // namespace sfz
