// SPDX-License-Identifier: BSD-2-Clause

// This code is part of the sfizz library and is licensed under a BSD 2-clause
// license. You should have receive a LICENSE.md file along with the code.
// If not, contact the sfizz maintainers at https://github.com/sfztools/sfizz

#include "ADSREnvelope.h"
#include "../../ADSREnvelope.h"
#include "../../Synth.h"
#include "../../Voice.h"
#include "../../Config.h"
#include "../../utility/Debug.h"

namespace sfz {

ADSREnvelopeSource::ADSREnvelopeSource(VoiceManager& manager)
    : voiceManager_(manager)
{
}

ADSREnvelope* getEG(Voice* voice, const ModKey& key)
{
    ADSREnvelope* eg = nullptr;
    if (!voice)
        return eg;

    switch (key.id()) {
    case ModId::AmpEG:
        eg = voice->getAmplitudeEG();
        break;
    case ModId::PitchEG:
        eg = voice->getPitchEG();
        break;
    case ModId::FilEG:
        eg = voice->getFilterEG();
        break;
    default:
        return eg;
    }

    return eg;
}

const EGDescription* getEGDescription(const Region* region, const ModKey& key)
{
    const EGDescription* desc = nullptr;
    if (!region)
        return desc;

    switch (key.id()) {
    case ModId::AmpEG:
        desc = &region->amplitudeEG;
        break;
    case ModId::PitchEG:
        desc = &*region->pitchEG;
        break;
    case ModId::FilEG:
        desc = &*region->filterEG;
        break;
    default:
        return desc;
    }

    return desc;
}

void ADSREnvelopeSource::init(const ModKey& sourceKey, NumericId<Voice> voiceId, unsigned delay)
{
    Voice* voice = voiceManager_.getVoiceById(voiceId);
    if (!voice) {
        ASSERTFALSE;
        return;
    }

    const Region* region = voice->getRegion();
    ADSREnvelope* eg = getEG(voice, sourceKey);
    const EGDescription* desc = getEGDescription(region, sourceKey);
    ASSERT(eg);

    const TriggerEvent& triggerEvent = voice->getTriggerEvent();
    const float sampleRate = voice->getSampleRate();
    eg->reset(*desc, *region, delay, triggerEvent.value, sampleRate);
}

void ADSREnvelopeSource::release(const ModKey& sourceKey, NumericId<Voice> voiceId, unsigned delay)
{
    Voice* voice = voiceManager_.getVoiceById(voiceId);
    if (!voice) {
        ASSERTFALSE;
        return;
    }

    ADSREnvelope* eg = getEG(voice, sourceKey);
    ASSERT(eg);

    eg->startRelease(delay);
}

void ADSREnvelopeSource::cancelRelease(const ModKey& sourceKey, NumericId<Voice> voiceId, unsigned delay)
{
    Voice* voice = voiceManager_.getVoiceById(voiceId);
    if (!voice) {
        ASSERTFALSE;
        return;
    }

    ADSREnvelope* eg = getEG(voice, sourceKey);
    ASSERT(eg);

    eg->cancelRelease(delay);
}

void ADSREnvelopeSource::generate(const ModKey& sourceKey, NumericId<Voice> voiceId, absl::Span<float> buffer)
{
    Voice* voice = voiceManager_.getVoiceById(voiceId);
    if (!voice) {
        ASSERTFALSE;
        return;
    }

    ADSREnvelope* eg = getEG(voice, sourceKey);
    ASSERT(eg);

    eg->getBlock(buffer);
}

} // namespace sfz
