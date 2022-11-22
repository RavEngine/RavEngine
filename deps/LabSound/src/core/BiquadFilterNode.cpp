// License: BSD 2 Clause
// Copyright (C) 2011, Google Inc. All rights reserved.
// Copyright (C) 2015+, The LabSound Authors. All rights reserved.

#include "LabSound/core/BiquadFilterNode.h"
#include "LabSound/core/AudioBus.h"
#include "LabSound/core/AudioNodeInput.h"
#include "LabSound/core/AudioNodeOutput.h"
#include "LabSound/core/AudioSetting.h"
#include "LabSound/extended/AudioContextLock.h"
#include "LabSound/extended/Registry.h"

#include "internal/Biquad.h"
#include <algorithm>

namespace lab
{

static char const * const s_filter_types[FilterType::_FilterTypeCount + 1] = {
    "None",
    "Low Pass", "High Pass", "Band Pass", "Low Shelf", "High Shelf", "Peaking", "Notch", "All Pass",
    nullptr};

class BiquadFilterNode::BiquadFilterNodeInternal : public AudioProcessor
{
    friend class BiquadFilterNode;

public:

    BiquadFilterNodeInternal() : AudioProcessor()
    {
        the_filter.reset(new Biquad());

        m_frequency = std::make_shared<AudioParam>("frequency", "FREQ", 350.0, 10.0, 22500);
        m_q = std::make_shared<AudioParam>("Q", "Q   ", 1, 0.0001, 1000.0);
        m_gain = std::make_shared<AudioParam>("gain", "GAIN", 0.0, -40, 40);
        m_detune = std::make_shared<AudioParam>("detune", "DTUN", 0.0, -4800, 4800);

        m_type = std::make_shared<AudioSetting>("type", "TYPE", s_filter_types);
        m_type->setEnumeration(static_cast<uint32_t>(FilterType::LOWPASS));

        m_type->setValueChanged([this]() {
            uint32_t type = m_type->valueUint32();
            if (type > static_cast<uint32_t>(FilterType::ALLPASS)) throw std::out_of_range("Filter type exceeds index of known types");
            m_hasJustReset = true;  // reset filter coeffs
        });
    }

    virtual ~BiquadFilterNodeInternal() 
    {
    }

    virtual void initialize() override {}
    virtual void uninitialize() override {}

    virtual void process(ContextRenderLock & r,  const lab::AudioBus * sourceBus, lab::AudioBus * destinationBus, int framesToProcess) override
    {
        checkForDirtyCoefficients(r);
        updateCoefficientsIfNecessary(r, true, false);
        the_filter->process(sourceBus->channel(0)->data(), destinationBus->channel(0)->mutableData(), framesToProcess);
    }

    virtual void reset() override {}
    virtual double tailTime(ContextRenderLock & r) const override { return 0.25f; } // fixed 250ms
    virtual double latencyTime(ContextRenderLock & r) const override { return 0; }

    // To prevent audio glitches when parameters are changed,
    // dezippering is used to slowly change the parameters.
    // |useSmoothing| implies that we want to update using the
    // smoothed values. Otherwise the final target values are
    // used. If |forceUpdate| is true, we update the coefficients even
    // if they are not dirty. (Used when computing the frequency
    // response.)
    void checkForDirtyCoefficients(ContextRenderLock & r)
    {
        // Deal with smoothing / de-zippering. Start out assuming filter parameters are not changing.
        // The BiquadDSPKernel objects rely on this value to see if they need to re-compute their internal filter coefficients.
        m_filterCoefficientsDirty = false;
        m_hasSampleAccurateValues = false;

        if (m_frequency->hasSampleAccurateValues() || m_q->hasSampleAccurateValues() || m_gain->hasSampleAccurateValues() || m_detune->hasSampleAccurateValues())
        {
            m_filterCoefficientsDirty = true;
            m_hasSampleAccurateValues = true;
        }
        else
        {
            // Snap to exact values first time after reset, then smooth for subsequent changes.
            if (m_hasJustReset)
            {
                m_frequency->resetSmoothedValue();
                m_q->resetSmoothedValue();
                m_gain->resetSmoothedValue();
                m_detune->resetSmoothedValue();
                m_filterCoefficientsDirty = true;
                m_hasJustReset = false;
            }
            else
            {
                // Smooth all of the filter parameters. If they haven't yet converged to their target value then mark coefficients as dirty.
                bool isStable1 = m_frequency->smooth(r);
                bool isStable2 = m_q->smooth(r);
                bool isStable3 = m_gain->smooth(r);
                bool isStable4 = m_detune->smooth(r);
                if (!(isStable1 && isStable2 && isStable3 && isStable4)) m_filterCoefficientsDirty = true;
            }
        }
    }

    void updateCoefficientsIfNecessary(ContextRenderLock & r, bool useSmoothing, bool forceUpdate)
    {
        /* bool isStable1 = */ m_frequency->smooth(r);
        /* bool isStable2 = */ m_q->smooth(r);
        /* bool isStable3 = */ m_gain->smooth(r);
        /* bool isStable4 = */ m_detune->smooth(r);
        
        if (forceUpdate || m_filterCoefficientsDirty)
        {
            double freq;
            double q_val;
            double gain;
            double detune;

            if (m_hasSampleAccurateValues)
            {
                /// @fixme these values should be per sample, not per quantum
                freq = m_frequency->finalValue(r);
                q_val = m_q->finalValue(r);
                gain = m_gain->finalValue(r);
                detune = m_detune->finalValue(r);
            }
            else if (useSmoothing)
            {
                /// @fixme these values should be per sample, not per quantum
                freq = m_frequency->smoothedValue();
                q_val = m_q->smoothedValue();
                gain = m_gain->smoothedValue();
                detune = m_detune->smoothedValue();
            }
            else
            {
                freq = m_frequency->value();
                q_val = m_q->value();
                gain = m_gain->value();
                detune = m_detune->value();
            }

            // Convert from Hertz to normalized frequency 0 -> 1.
            double nyquist = r.context()->sampleRate() * 0.5f;
            double normalizedFrequency = freq / nyquist;

            // Offset frequency by detune
            if (detune)
            {
                normalizedFrequency *= std::pow(2.0, detune / 1200.0);
            }

            // Configure the biquad with the new filter parameters for the appropriate type of filter.
            // clang-format off
            switch (m_type->valueUint32())
            {
                case FilterType::LOWPASS:   the_filter->setLowpassParams(normalizedFrequency, q_val);       break;
                case FilterType::HIGHPASS:  the_filter->setHighpassParams(normalizedFrequency, q_val);      break;
                case FilterType::BANDPASS:  the_filter->setBandpassParams(normalizedFrequency, q_val);      break;
                case FilterType::LOWSHELF:  the_filter->setLowShelfParams(normalizedFrequency, gain);       break;
                case FilterType::HIGHSHELF: the_filter->setHighShelfParams(normalizedFrequency, gain);      break;
                case FilterType::PEAKING:   the_filter->setPeakingParams(normalizedFrequency, q_val, gain); break;
                case FilterType::NOTCH:     the_filter->setNotchParams(normalizedFrequency, q_val);         break;
                case FilterType::ALLPASS:   the_filter->setAllpassParams(normalizedFrequency, q_val);        break;
                default: break;
            }
            // clang-format on
        }
    }

    void getFrequencyResponse(ContextRenderLock & r, const std::vector<float> & frequencyHz, std::vector<float> & magResponse, std::vector<float> & phaseResponse)
    {
        if (!frequencyHz.size() || !magResponse.size() || !phaseResponse.size())
            return;

        size_t n = std::min(frequencyHz.size(), std::min(magResponse.size(), phaseResponse.size()));

        // @todo
        if (n)
        {
            // std::unique_ptr<BiquadDSPKernel> responseKernel(new BiquadDSPKernel(this));
            // responseKernel->getFrequencyResponse(r, nFrequencies, frequencyHz, magResponse, phaseResponse);
            //biquadProcessor()->getFrequencyResponse(r, n, &frequencyHz[0], &magResponse[0], &phaseResponse[0]);
        }
    }

    // so DSP kernels know when to re-compute coefficients
    bool m_filterCoefficientsDirty {true};

    // Set to true if any of the filter parameters are sample-accurate.
    bool m_hasSampleAccurateValues {false};

    bool m_hasJustReset {true};

    std::shared_ptr<AudioSetting> m_type;
    std::shared_ptr<AudioParam> m_frequency;
    std::shared_ptr<AudioParam> m_q;
    std::shared_ptr<AudioParam> m_gain;
    std::shared_ptr<AudioParam> m_detune;

    std::unique_ptr<Biquad> the_filter;
};
 
BiquadFilterNode::BiquadFilterNode(AudioContext& ac) : AudioBasicProcessorNode(ac)
{
    biquad_impl = new BiquadFilterNodeInternal();
    m_processor.reset(biquad_impl);

    m_params.push_back(biquad_impl->m_frequency);
    m_params.push_back(biquad_impl->m_q);
    m_params.push_back(biquad_impl->m_gain);
    m_params.push_back(biquad_impl->m_detune);
    m_settings.push_back(biquad_impl->m_type);
    initialize();
}

BiquadFilterNode::~BiquadFilterNode()
{
    if (isInitialized()) uninitialize();
}

void BiquadFilterNode::setType(FilterType type)
{
    biquad_impl->m_type->setUint32(uint32_t(type));
}

void BiquadFilterNode::getFrequencyResponse(ContextRenderLock & r, const std::vector<float> & frequencyHz, std::vector<float> & magResponse, std::vector<float> & phaseResponse)
{
    biquad_impl->getFrequencyResponse(r, frequencyHz, magResponse, phaseResponse);
}

FilterType BiquadFilterNode::type() const
{
    return (FilterType) biquad_impl->m_type->valueUint32();
}

std::shared_ptr<AudioParam> BiquadFilterNode::frequency()
{
    return biquad_impl->m_frequency;
}

std::shared_ptr<AudioParam> BiquadFilterNode::q()
{
    return biquad_impl->m_q;
}

std::shared_ptr<AudioParam> BiquadFilterNode::gain()
{
    return biquad_impl->m_gain;
}

std::shared_ptr<AudioParam> BiquadFilterNode::detune()
{
    return biquad_impl->m_detune;
}

}  // namespace lab
