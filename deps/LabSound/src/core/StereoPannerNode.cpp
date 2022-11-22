// License: BSD 2 Clause
// Copyright (C) 2014, The Chromium Authors. All rights reserved.
// Copyright (C) 2015+, The LabSound Authors. All rights reserved.

#include "LabSound/core/StereoPannerNode.h"
#include "LabSound/core/AudioBus.h"
#include "LabSound/core/AudioContext.h"
#include "LabSound/core/AudioNodeInput.h"
#include "LabSound/core/AudioNodeOutput.h"
#include "LabSound/core/Macros.h"
#include "LabSound/core/Mixing.h"
#include "LabSound/core/SampledAudioNode.h"
#include "LabSound/extended/Util.h"

#include "LabSound/extended/AudioContextLock.h"
#include "LabSound/extended/Registry.h"

#include "internal/Assertions.h"
#include "internal/AudioUtilities.h"
#include "internal/Panner.h"

namespace lab
{

class Spatializer
{

public:
    // @tofix - this only supports equal power
    enum PanningModel
    {
        PanningModelEqualPower = 0
    };

    Spatializer(const float sampleRate, PanningModel model)
    {
        // Convert smoothing time (50ms) to a per-sample time value.
        m_smoothingConstant = AudioUtilities::discreteTimeConstantForSampleRate(SmoothingTimeConstant, sampleRate);
    }

    virtual ~Spatializer()
    {
    }

    // Handle sample-accurate panning by AudioParam automation.
    virtual void panWithSampleAccurateValues(const AudioBus * inputBus, AudioBus * outputBus, const float * panValues, int framesToProcess)
    {
        size_t numberOfInputChannels = inputBus->numberOfChannels();

        bool isInputSafe = inputBus && (inputBus->numberOfChannels() == Channels::Mono || 
            inputBus->numberOfChannels() == Channels::Stereo) && framesToProcess <= inputBus->length();

        ASSERT(isInputSafe);

        if (!isInputSafe)
            return;

        bool isOutputSafe = outputBus && outputBus->numberOfChannels() == Channels::Stereo && framesToProcess <= outputBus->length();

        ASSERT(isOutputSafe);

        if (!isOutputSafe)
            return;

        const float * sourceL = inputBus->channel(0)->data();
        const float * sourceR = numberOfInputChannels > Channels::Mono ? inputBus->channel(1)->data() : sourceL;

        float * destinationL = outputBus->channelByType(Channel::Left)->mutableData();
        float * destinationR = outputBus->channelByType(Channel::Right)->mutableData();

        if (!sourceL || !sourceR || !destinationL || !destinationR)
            return;

        double gainL, gainR, panRadian;
        size_t n = framesToProcess;

        // For mono source case.
        if (numberOfInputChannels == Channels::Mono)
        {
            while (n--)
            {
                float inputL = *sourceL++;

                m_pan = clampTo(*panValues++, -1.0, 1.0);

                // Pan from left to right [-1; 1] will be normalized as [0; 1].
                panRadian = (m_pan * 0.5 + 0.5) * LAB_HALF_PI;

                gainL = std::cos(panRadian);
                gainR = std::sin(panRadian);

                *destinationL++ = static_cast<float>(inputL * gainL);
                *destinationR++ = static_cast<float>(inputL * gainR);
            }
        }
        // For stereo source case.
        else
        {
            while (n--)
            {
                float inputL = *sourceL++;
                float inputR = *sourceR++;

                m_pan = clampTo(*panValues++, -1.0, 1.0);

                // Normalize [-1; 0] to [0; 1]. Do nothing when [0; 1].
                panRadian = (m_pan <= 0 ? m_pan + 1 : m_pan) * LAB_HALF_PI;

                gainL = std::cos(panRadian);
                gainR = std::sin(panRadian);

                if (m_pan <= 0)
                {
                    *destinationL++ = static_cast<float>(inputL + inputR * gainL);
                    *destinationR++ = static_cast<float>(inputR * gainR);
                }
                else
                {
                    *destinationL++ = static_cast<float>(inputL * gainL);
                    *destinationR++ = static_cast<float>(inputR + inputL * gainR);
                }
            }
        }
    }

    // Handle de-zippered panning to a target value.
    virtual void panToTargetValue(const AudioBus * inputBus, AudioBus * outputBus, float panValue, size_t framesToProcess)
    {
        size_t numberOfInputChannels = inputBus->numberOfChannels();

        bool isInputSafe = inputBus && (inputBus->numberOfChannels() == Channels::Mono || inputBus->numberOfChannels() == Channels::Stereo) && framesToProcess <= inputBus->length();

        ASSERT(isInputSafe);

        if (!isInputSafe)
            return;

        bool isOutputSafe = outputBus && framesToProcess <= outputBus->length();

        ASSERT(isOutputSafe);

        if (!isOutputSafe)
            return;

        const float * sourceL = inputBus->channel(0)->data();
        const float * sourceR = numberOfInputChannels > Channels::Mono ? inputBus->channel(1)->data() : sourceL;

        float * destinationL = outputBus->channelByType(Channel::Left)->mutableData();
        float * destinationR = outputBus->channelByType(Channel::Right)->mutableData();

        if (!sourceL || !sourceR || !destinationL || !destinationR)
            return;

        float targetPan = clampTo(panValue, -1.f, 1.f);

        // Don't de-zipper on first render call.
        if (m_isFirstRender)
        {
            m_isFirstRender = false;
            m_pan = targetPan;
        }

        double gainL, gainR, panRadian;
        const double smoothingConstant = m_smoothingConstant;
        size_t n = framesToProcess;

        // For mono source case.
        if (numberOfInputChannels == Channels::Mono)
        {
            while (n--)
            {
                float inputL = *sourceL++;

                m_pan += (targetPan - m_pan) * smoothingConstant;

                // Pan from left to right [-1; 1] will be normalized as [0; 1].
                panRadian = (m_pan * 0.5 + 0.5) * LAB_HALF_PI;

                gainL = std::cos(panRadian);
                gainR = std::sin(panRadian);

                *destinationL++ = static_cast<float>(inputL * gainL);
                *destinationR++ = static_cast<float>(inputL * gainR);
            }
        }
        // For stereo source case.
        else
        {
            while (n--)
            {
                float inputL = *sourceL++;
                float inputR = *sourceR++;

                m_pan += (targetPan - m_pan) * smoothingConstant;

                // Normalize [-1; 0] to [0; 1] for the left pan position (<= 0), and
                // do nothing when [0; 1].

                panRadian = (m_pan <= 0 ? m_pan + 1 : m_pan) * LAB_HALF_PI;

                gainL = std::cos(panRadian);
                gainR = std::sin(panRadian);

                // The pan value should be checked every sample when de-zippering.
                // See crbug.com/470559.
                if (m_pan <= 0)
                {
                    // When [-1; 0], keep left channel intact and equal-power pan the
                    // right channel only.
                    *destinationL++ = static_cast<float>(inputL + inputR * gainL);
                    *destinationR++ = static_cast<float>(inputR * gainR);
                }
                else
                {
                    // When [0; 1], keep right channel intact and equal-power pan the
                    // left channel only.
                    *destinationL++ = static_cast<float>(inputL * gainL);
                    *destinationR++ = static_cast<float>(inputR + inputL * gainR);
                }
            }
        }
    }

    virtual void reset()
    {
        // No-op
    }

    virtual double tailTime(ContextRenderLock & r) const { return 0; }
    virtual double latencyTime(ContextRenderLock & r) const { return 0; }

private:
    bool m_isFirstRender = true;
    double m_pan = 0.0;

    double m_smoothingConstant;

    // Use a 50ms smoothing / de-zippering time-constant.
    const float SmoothingTimeConstant = 0.050f;
};

using namespace std;

StereoPannerNode::StereoPannerNode(AudioContext& ac)
    : AudioNode(ac)
{
    m_sampleAccuratePanValues.reset(new AudioFloatArray(AudioNode::ProcessingSizeInFrames));

    addInput(std::unique_ptr<AudioNodeInput>(new AudioNodeInput(this)));
    addOutput(std::unique_ptr<AudioNodeOutput>(new AudioNodeOutput(this, 2)));

    m_stereoPanner.reset(new Spatializer(ac.sampleRate(), Spatializer::PanningModelEqualPower));

    m_pan = std::make_shared<AudioParam>("pan", "PAN", 0.5, 0.0, 1.0);
    m_params.push_back(m_pan);

    initialize();
}

StereoPannerNode::~StereoPannerNode()
{
    uninitialize();
}

void StereoPannerNode::process(ContextRenderLock & r, int bufferSize)
{
    AudioBus * outputBus = output(0)->bus(r);

    if (!isInitialized() || !input(0)->isConnected() || !m_stereoPanner.get())
    {
        outputBus->zero();
        return;
    }

    AudioBus * inputBus = input(0)->bus(r);

    if (!inputBus)
    {
        inputBus->zero();
        return;
    }

    if (m_pan->hasSampleAccurateValues())
    {
        // Apply sample-accurate panning specified by AudioParam automation.
        ASSERT(bufferSize <= m_sampleAccuratePanValues->size());

        if (bufferSize <= m_sampleAccuratePanValues->size())
        {
            float * panValues = m_sampleAccuratePanValues->data();
            m_pan->calculateSampleAccurateValues(r, panValues, bufferSize);
            m_stereoPanner->panWithSampleAccurateValues(inputBus, outputBus, panValues, bufferSize);
        }
    }
    else
    {
        m_stereoPanner->panToTargetValue(inputBus, outputBus, m_pan->value(), bufferSize);
    }

    outputBus->clearSilentFlag();
}

void StereoPannerNode::reset(ContextRenderLock &)
{
    // No-op
}

void StereoPannerNode::initialize()
{
    if (isInitialized()) 
        return;

    AudioNode::initialize();
}

void StereoPannerNode::uninitialize()
{
    if (!isInitialized()) 
        return;

    AudioNode::uninitialize();
}

}  // namespace lab
