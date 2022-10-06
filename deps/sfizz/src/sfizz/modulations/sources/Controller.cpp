// SPDX-License-Identifier: BSD-2-Clause

// This code is part of the sfizz library and is licensed under a BSD 2-clause
// license. You should have receive a LICENSE.md file along with the code.
// If not, contact the sfizz maintainers at https://github.com/sfztools/sfizz

#include "Controller.h"
#include "../ModKey.h"
#include "../../Smoothers.h"
#include "../../ModifierHelpers.h"
#include "../../Resources.h"
#include "../../Config.h"
#include "../../utility/Debug.h"
#include <absl/container/flat_hash_map.h>

namespace sfz {

struct ControllerSource::Impl {
    float getLastTransformedValue(uint16_t cc, uint8_t curve) const noexcept;
    double sampleRate_ = config::defaultSampleRate;
    Resources* res_ = nullptr;
    VoiceManager* voiceManager_ = nullptr;
    absl::flat_hash_map<ModKey, Smoother> smoother_;
};

ControllerSource::ControllerSource(Resources& res, VoiceManager& manager)
    : impl_(new Impl)
{
    impl_->res_ = &res;
    impl_->voiceManager_ = &manager;
}

ControllerSource::~ControllerSource()
{
}

float ControllerSource::Impl::getLastTransformedValue(uint16_t cc, uint8_t curveIndex) const noexcept
{
    ASSERT(res_);
    const Curve& curve = res_->getCurves().getCurve(curveIndex);
    const auto lastCCValue = res_->getMidiState().getCCValue(cc);
    return curve.evalNormalized(lastCCValue);
}

void ControllerSource::resetSmoothers()
{
    for (auto& item : impl_->smoother_) {
        const ModKey::Parameters p = item.first.parameters();
        item.second.reset(impl_->getLastTransformedValue(p.cc, p.curve));
    }
}

void ControllerSource::setSampleRate(double sampleRate)
{
    if (impl_->sampleRate_ == sampleRate)
        return;

    impl_->sampleRate_ = sampleRate;

    for (auto& item : impl_->smoother_) {
        const ModKey::Parameters p = item.first.parameters();
        item.second.setSmoothing(p.smooth, sampleRate);
    }
}

void ControllerSource::setSamplesPerBlock(unsigned count)
{
    (void)count;
}

void ControllerSource::init(const ModKey& sourceKey, NumericId<Voice> voiceId, unsigned delay)
{
    (void)voiceId;
    (void)delay;

    const ModKey::Parameters p = sourceKey.parameters();
    if (p.smooth > 0) {
        Smoother s;
        s.setSmoothing(p.smooth, impl_->sampleRate_);
        s.reset(impl_->getLastTransformedValue(p.cc, p.curve));
        impl_->smoother_[sourceKey] = s;
    }
    else {
        impl_->smoother_.erase(sourceKey);
    }
}

void ControllerSource::generate(const ModKey& sourceKey, NumericId<Voice> voiceId, absl::Span<float> buffer)
{
    const ModKey::Parameters p = sourceKey.parameters();
    const Resources& res = *impl_->res_;
    const Curve& curve = res.getCurves().getCurve(p.curve);
    const MidiState& ms = res.getMidiState();
    bool canShortcut = false;

    auto transformValue = [&] (float x) {
        return curve.evalNormalized(x);
    };

    auto quantize = [&] (float x) {
        if (p.step > 0.0f)
            return std::trunc(x / p.step) * p.step;

        return x;
    };

    switch(p.cc) {
    case ExtendedCCs::polyphonicAftertouch: {
            const auto voice = impl_->voiceManager_->getVoiceById(voiceId);
            const float fillValue =
                voice && voice->getTriggerEvent().type == TriggerEventType::NoteOn ?
                impl_->res_->getMidiState().getPolyAftertouch(voice->getTriggerEvent().number) : 0.0f;

            sfz::fill(buffer, quantize(transformValue(fillValue)));
            canShortcut = true;
            break;
        }
    case ExtendedCCs::noteOnVelocity: {
            const auto voice = impl_->voiceManager_->getVoiceById(voiceId);
            const float fillValue =
                voice && voice->getTriggerEvent().type == TriggerEventType::NoteOn ?
                voice->getTriggerEvent().value : 0.0f;

            sfz::fill(buffer, quantize(transformValue(fillValue)));
            canShortcut = true;
            break;
        }
    case ExtendedCCs::noteOffVelocity: {
            const auto voice = impl_->voiceManager_->getVoiceById(voiceId);
            const float fillValue =
                voice && voice->getTriggerEvent().type == TriggerEventType::NoteOff ?
                voice->getTriggerEvent().value : 0.0f;

            sfz::fill(buffer, quantize(transformValue(fillValue)));
            canShortcut = true;
            break;
        }
    case ExtendedCCs::keyboardNoteNumber: {
            const auto voice = impl_->voiceManager_->getVoiceById(voiceId);
            const float fillValue = voice ? normalize7Bits(voice->getTriggerEvent().number) : 0.0f;
            sfz::fill(buffer, quantize(transformValue(fillValue)));
            canShortcut = true;
            break;
        }
    case ExtendedCCs::keyboardNoteGate: {
            const auto voice = impl_->voiceManager_->getVoiceById(voiceId);
            const float fillValue = voice ? voice->getExtendedCCValues().noteGate : 0.0f;
            sfz::fill(buffer, quantize(transformValue(fillValue)));
            canShortcut = true;
            break;
        }
    case ExtendedCCs::unipolarRandom: {
            const auto voice = impl_->voiceManager_->getVoiceById(voiceId);
            const float fillValue = voice ? voice->getExtendedCCValues().unipolar : 0.0f;
            sfz::fill(buffer, quantize(transformValue(fillValue)));
            canShortcut = true;
            break;
        }
    case ExtendedCCs::bipolarRandom: {
            const auto voice = impl_->voiceManager_->getVoiceById(voiceId);
            const float fillValue = voice ? voice->getExtendedCCValues().bipolar : 0.0f;
            sfz::fill(buffer, quantize(transformValue(fillValue)));
            canShortcut = true;
            break;
        }
    case ExtendedCCs::alternate: {
            const auto voice = impl_->voiceManager_->getVoiceById(voiceId);
            const float fillValue = voice ? voice->getExtendedCCValues().alternate : 0.0f;
            sfz::fill(buffer, quantize(transformValue(fillValue)));
            canShortcut = true;
            break;
        }
    case ExtendedCCs::keydelta: {
            const auto voice = impl_->voiceManager_->getVoiceById(voiceId);
            const float fillValue = voice ? voice->getExtendedCCValues().keydelta : 0.0f;
            sfz::fill(buffer, quantize(fillValue));
            canShortcut = true;
            break;
        }
    case ExtendedCCs::absoluteKeydelta: {
            const auto voice = impl_->voiceManager_->getVoiceById(voiceId);
            const float fillValue = voice ? std::abs(voice->getExtendedCCValues().keydelta) : 0.0f;
            sfz::fill(buffer, quantize(fillValue));
            canShortcut = true;
            break;
        }
    case ExtendedCCs::pitchBend: // fallthrough
    case ExtendedCCs::channelAftertouch: {
            const EventVector& events = ms.getCCEvents(p.cc);
            linearEnvelope(events, buffer, [](float x) { return x; }, p.step);
            canShortcut = events.size() == 1;
            break;
        }
    default: {
            const EventVector& events = ms.getCCEvents(p.cc);
            linearEnvelope(events, buffer, transformValue, p.step);
            canShortcut = events.size() == 1;
        }
    }

    auto it = impl_->smoother_.find(sourceKey);
    if (it != impl_->smoother_.end()) {
        Smoother& s = it->second;
        s.process(buffer, buffer, canShortcut);
    }
}

} // namespace sfz
