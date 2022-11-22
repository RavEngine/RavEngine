// License: BSD 2 Clause
// Copyright (C) 2015+, The LabSound Authors. All rights reserved.

#include "LabSound/extended/ADSRNode.h"
#include "LabSound/extended/AudioContextLock.h"
#include "LabSound/core/AudioNodeInput.h"
#include "LabSound/core/AudioNodeOutput.h"
#include "LabSound/core/AudioProcessor.h"
#include "LabSound/core/AudioBus.h"
#include "LabSound/extended/Registry.h"

#include "internal/VectorMath.h"

#include <limits>
#include <deque>

using namespace lab;

namespace lab
{
    ///////////////////////////////////////////
    // ADSRNode::ADSRNodeImpl Implementation //
    ///////////////////////////////////////////

    class ADSRNode::ADSRNodeImpl : public lab::AudioProcessor
    {
    public:
        float cached_sample_rate;
        struct LerpTarget { float t, dvdt; };
        std:: deque<LerpTarget> _lerp;

        ADSRNodeImpl() : AudioProcessor()
        {
            envelope.reserve(AudioNode::ProcessingSizeInFrames * 4);

            m_gate = std::make_shared<AudioParam>("gate", "GATE", 0, 0, 1);

            m_oneShot = std::make_shared<AudioSetting>("oneShot", "ONE!", AudioSetting::Type::Bool);
            m_oneShot->setBool(true);

            m_attackTime = std::make_shared<AudioSetting>("attackTime", "ATKT", AudioSetting::Type::Float);
            m_attackTime->setFloat(1.125f); // 125ms

            m_attackLevel = std::make_shared<AudioSetting>("attackLevel", "ATKL", AudioSetting::Type::Float);
            m_attackLevel->setFloat(1.0f); // 1.0f

            m_decayTime = std::make_shared<AudioSetting>("decayTime", "DCYT", AudioSetting::Type::Float);
            m_decayTime->setFloat(0.125f); // 125ms

            m_sustainTime = std::make_shared<AudioSetting>("sustainTime", "SUST", AudioSetting::Type::Float);
            m_sustainTime->setFloat(0.125f); // 125ms

            m_sustainLevel = std::make_shared<AudioSetting>("sustainLevel", "SUSL", AudioSetting::Type::Float);
            m_sustainLevel->setFloat(0.5f); // 0.5f

            m_releaseTime = std::make_shared<AudioSetting>("ReleaseTime", "RELT", AudioSetting::Type::Float);
            m_releaseTime->setFloat(0.125f); // 125ms
        }

        virtual ~ADSRNodeImpl() {}

        virtual void initialize() override { }
        virtual void uninitialize() override { }

        // Processes the source to destination bus. The number of channels must match in source and destination.
        virtual void process(ContextRenderLock & r, const lab::AudioBus * sourceBus, lab::AudioBus* destinationBus, int framesToProcess) override
        {
            using std::deque;

            if (!destinationBus->numberOfChannels())
                return;

            if (!sourceBus->numberOfChannels())
            {
                destinationBus->zero();
                return;
            }

            if (framesToProcess != _gateArray.size()) 
                _gateArray.resize(framesToProcess);
            if (envelope.size() != framesToProcess)
                envelope.resize(framesToProcess);

            // scan the gate signal
            const bool gate_is_connected = m_gate->hasSampleAccurateValues();
            if (gate_is_connected)
            {
                m_gate->calculateSampleAccurateValues(r, _gateArray.data(), framesToProcess);

                // threshold the gate to on or off
                for (int i = 0; i < framesToProcess; ++i)
                    _gateArray[i] = _gateArray[i] > 0 ? 1.f : 0.f;
            }
            else
            {
                float g = m_gate->value();
                // threshold the gate to on or off
                for (int i = 0; i < framesToProcess; ++i)
                    _gateArray[i] = g > 0 ? 1.f : 0.f;
            }

            bool oneshot = m_oneShot->valueBool(); // false mains gate controls AS, otherwise, sustain param controls S

            cached_sample_rate = r.context()->sampleRate();
            for (int i = 0; i < framesToProcess; ++i)
            {
                if (_currentGate == 0 && _gateArray[i] > 0)
                {
                    // attack begin
                    _currentGate = 1;
                    _lerp.clear();  // forget all previous lerps
                    float attackLevel = m_attackLevel->valueFloat();
                    float attackDelta = m_attackLevel->valueFloat() - currentEnvelope;
                    float attackRatio = attackDelta / attackLevel;
                    float attackSteps = m_attackTime->valueFloat() * attackRatio * cached_sample_rate;
                    float attackStepSize = attackDelta / attackSteps;

                    _lerp.emplace_back(LerpTarget{ attackSteps, attackStepSize });

                    float sustainLevel = m_sustainLevel->valueFloat();
                    float decaySteps = m_decayTime->valueFloat() * cached_sample_rate;
                    float decayStepSize = (sustainLevel - attackLevel) / decaySteps;
                    _lerp.emplace_back(LerpTarget{ decaySteps, decayStepSize });

                    if (!gate_is_connected || oneshot)
                    {
                        // if the gate is not connected, automate the sustain and release.
                        float sustainSteps = m_sustainTime->valueFloat() * cached_sample_rate;
                        _lerp.emplace_back(LerpTarget{ sustainSteps, 0.f });
                        float releaseSteps = m_releaseTime->valueFloat() * cached_sample_rate;
                        _lerp.emplace_back(LerpTarget{ releaseSteps, -sustainLevel / releaseSteps });
                    }
                    envelope[i] = currentEnvelope;
                }
                else if (_currentGate > 0 && _gateArray[i] == 0)
                {
                    // release begin
                    _currentGate = 0;
                    _lerp.clear();  // forget all previous lerps
                    float releaseSteps = m_releaseTime->valueFloat() * cached_sample_rate;
                    _lerp.emplace_back(LerpTarget{ releaseSteps, -currentEnvelope / releaseSteps });
                    envelope[i] = currentEnvelope;
                }
                else
                {
                    bool assigned = false;
                    while (_lerp.size())
                    {
                        LerpTarget& front = _lerp.front();
                        if (front.t > 0)
                        {
                            currentEnvelope += front.dvdt;
                            envelope[i] = currentEnvelope;
                            front.t -= 1.f;
                            assigned = true;
                            break;
                        }
                        else
                            _lerp.pop_front();
                    }
                    if (!assigned)
                        envelope[i] = currentEnvelope;
                }
            }

            destinationBus->copyWithSampleAccurateGainValuesFrom(*sourceBus, envelope.data(), framesToProcess);
        }

        virtual void reset() override { }

        virtual double tailTime(ContextRenderLock & r) const override { return 0; }
        virtual double latencyTime(ContextRenderLock & r) const override { return 0; }

        float _currentGate{ 0.0 };

        float currentEnvelope{ 0.f };
        std::vector<float> envelope;

        std::vector<float> _gateArray;

        std::shared_ptr<AudioParam> m_gate;

        std::shared_ptr<AudioSetting> m_oneShot;
        std::shared_ptr<AudioSetting> m_attackTime;
        std::shared_ptr<AudioSetting> m_attackLevel;
        std::shared_ptr<AudioSetting> m_decayTime;
        std::shared_ptr<AudioSetting> m_sustainTime;
        std::shared_ptr<AudioSetting> m_sustainLevel;
        std::shared_ptr<AudioSetting> m_releaseTime;
    };

    /////////////////////
    // Public ADSRNode //
    /////////////////////

    ADSRNode::ADSRNode(AudioContext& ac) 
        : AudioNode(ac), adsr_impl(new ADSRNodeImpl)
    {
        addInput(std::unique_ptr<AudioNodeInput>(new AudioNodeInput(this)));
        addOutput(std::unique_ptr<AudioNodeOutput>(new AudioNodeOutput(this, 1)));

        m_params.push_back(adsr_impl->m_gate);
        m_settings.push_back(adsr_impl->m_oneShot);
        m_settings.push_back(adsr_impl->m_attackTime);
        m_settings.push_back(adsr_impl->m_attackLevel);
        m_settings.push_back(adsr_impl->m_decayTime);
        m_settings.push_back(adsr_impl->m_sustainLevel);
        m_settings.push_back(adsr_impl->m_sustainTime);
        m_settings.push_back(adsr_impl->m_releaseTime);

        initialize();
    }

    ADSRNode::~ADSRNode()
    {
        uninitialize();
        delete adsr_impl;
    }

    std::shared_ptr<AudioParam> ADSRNode::gate() const
    {
        return adsr_impl->m_gate;
    }

    void ADSRNode::set(float attack_time, float attack_level, float decay_time, float sustain_time, float sustain_level, float release_time)
    {
        adsr_impl->m_attackTime->setFloat(attack_time);
        adsr_impl->m_attackLevel->setFloat(attack_level);
        adsr_impl->m_decayTime->setFloat(decay_time);
        adsr_impl->m_sustainTime->setFloat(sustain_time);
        adsr_impl->m_sustainLevel->setFloat(sustain_level);
        adsr_impl->m_releaseTime->setFloat(release_time);
    }

    // clang-format off
    std::shared_ptr<AudioSetting> ADSRNode::oneShot() const      { return adsr_impl->m_oneShot;      }
    std::shared_ptr<AudioSetting> ADSRNode::attackTime() const   { return adsr_impl->m_attackTime;   }
    std::shared_ptr<AudioSetting> ADSRNode::attackLevel() const  { return adsr_impl->m_attackLevel;  }
    std::shared_ptr<AudioSetting> ADSRNode::decayTime() const    { return adsr_impl->m_decayTime;    } 
    std::shared_ptr<AudioSetting> ADSRNode::sustainTime() const  { return adsr_impl->m_sustainTime;  }
    std::shared_ptr<AudioSetting> ADSRNode::sustainLevel() const { return adsr_impl->m_sustainLevel; }
    std::shared_ptr<AudioSetting> ADSRNode::releaseTime() const  { return adsr_impl->m_releaseTime;  }
    //clang-format on

    bool ADSRNode::finished(ContextRenderLock& r)
    {
        if (!r.context())
            return true;

        double now = r.context()->currentTime();
        return adsr_impl->_lerp.size() > 0;
    }

    void ADSRNode::process(ContextRenderLock& r, int bufferSize)
    {
        AudioBus* destinationBus = output(0)->bus(r);
        AudioBus* sourceBus = input(0)->bus(r);
        if (!isInitialized() || !input(0)->isConnected())
        {
            destinationBus->zero();
            return;
        }

        int numberOfInputChannels = input(0)->numberOfChannels(r);
        if (numberOfInputChannels != output(0)->numberOfChannels())
        {
            output(0)->setNumberOfChannels(r, numberOfInputChannels);
            destinationBus = output(0)->bus(r);
        }

        // process entire buffer
        adsr_impl->process(r, sourceBus, destinationBus, bufferSize);
    }

    void ADSRNode::reset(ContextRenderLock&)
    {
        gate()->setValue(0.f);
    }


} // End namespace lab
