// SPDX-License-Identifier: BSD-2-Clause

// This code is part of the sfizz library and is licensed under a BSD 2-clause
// license. You should have receive a LICENSE.md file along with the code.
// If not, contact the sfizz maintainers at https://github.com/sfztools/sfizz

#include "FlexEnvelope.h"
#include "../../FlexEnvelope.h"
#include "../../Synth.h"
#include "../../Voice.h"
#include "../../SIMDHelpers.h"
#include "../../Config.h"
#include "../../utility/Debug.h"

namespace sfz {

FlexEnvelopeSource::FlexEnvelopeSource(VoiceManager& manager)
    : voiceManager_(manager)
{

}

void FlexEnvelopeSource::init(const ModKey& sourceKey, NumericId<Voice> voiceId, unsigned delay)
{
    unsigned egIndex = sourceKey.parameters().N;

    Voice* voice = voiceManager_.getVoiceById(voiceId);
    if (!voice) {
        ASSERTFALSE;
        return;
    }

    const Region* region = voice->getRegion();
    if (egIndex >= region->flexEGs.size()) {
        ASSERTFALSE;
        return;
    }

    FlexEnvelope* eg = voice->getFlexEG(egIndex);
    eg->configure(&region->flexEGs[egIndex]);
    bool freeRunning = (
        (region->loopMode == LoopMode::one_shot && region->isOscillator())
    );
    if (freeRunning && region->flexAmpEG && egIndex == *region->flexAmpEG)
        eg->setFreeRunning(true);

    eg->start(delay);
}

void FlexEnvelopeSource::release(const ModKey& sourceKey, NumericId<Voice> voiceId, unsigned delay)
{
    unsigned egIndex = sourceKey.parameters().N;

    Voice* voice = voiceManager_.getVoiceById(voiceId);
    if (!voice) {
        ASSERTFALSE;
        return;
    }

    const Region* region = voice->getRegion();
    if (egIndex >= region->flexEGs.size()) {
        ASSERTFALSE;
        return;
    }

    FlexEnvelope* eg = voice->getFlexEG(egIndex);
    eg->release(delay);
}

void FlexEnvelopeSource::cancelRelease(const ModKey& sourceKey, NumericId<Voice> voiceId, unsigned delay)
{
    unsigned egIndex = sourceKey.parameters().N;

    Voice* voice = voiceManager_.getVoiceById(voiceId);
    if (!voice) {
        ASSERTFALSE;
        return;
    }

    const Region* region = voice->getRegion();
    if (egIndex >= region->flexEGs.size()) {
        ASSERTFALSE;
        return;
    }

    FlexEnvelope* eg = voice->getFlexEG(egIndex);
    eg->cancelRelease(delay);
}

void FlexEnvelopeSource::generate(const ModKey& sourceKey, NumericId<Voice> voiceId, absl::Span<float> buffer)
{
    unsigned egIndex = sourceKey.parameters().N;

    Voice* voice = voiceManager_.getVoiceById(voiceId);
    if (!voice) {
        ASSERTFALSE;
        return;
    }

    const Region* region = voice->getRegion();
    if (egIndex >= region->flexEGs.size()) {
        ASSERTFALSE;
        return;
    }

    FlexEnvelope* eg = voice->getFlexEG(egIndex);
    eg->process(buffer);
}

} // namespace sfz
