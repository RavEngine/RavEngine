// License: BSD 2 Clause
// Copyright (C) 2010, Google Inc. All rights reserved.
// Copyright (C) 2015+, The LabSound Authors. All rights reserved.

#include "LabSound/core/AudioBus.h"
#include "LabSound/core/Macros.h"
#include "LabSound/core/Mixing.h"

#include "internal/Assertions.h"
#include "internal/AudioUtilities.h"
#include "internal/EqualPowerPanner.h"

#include "LabSound/extended/AudioContextLock.h"

#include <algorithm>

// Use a 50ms smoothing / de-zippering time-constant.
const float SmoothingTimeConstant = 0.050f;

using namespace std;

namespace lab
{

EqualPowerPanner::EqualPowerPanner(const float sampleRate)
    : Panner(sampleRate, PanningMode::EQUALPOWER)
{
}

void EqualPowerPanner::pan(ContextRenderLock & r, double azimuth, double /*elevation*/, const AudioBus * inputBus, AudioBus * outputBus, int framesToProcess)
{
    m_smoothingConstant = AudioUtilities::discreteTimeConstantForSampleRate(SmoothingTimeConstant, r.context()->sampleRate());

    bool isInputSafe = inputBus && (inputBus->numberOfChannels() == Channels::Mono || inputBus->numberOfChannels() == Channels::Stereo) && framesToProcess <= inputBus->length();
    ASSERT(isInputSafe);
    if (!isInputSafe)
        return;

    unsigned numberOfInputChannels = inputBus->numberOfChannels();

    bool isOutputSafe = outputBus && outputBus->numberOfChannels() == Channels::Stereo && framesToProcess <= outputBus->length();
    ASSERT(isOutputSafe);
    if (!isOutputSafe)
        return;

    const float * sourceL = inputBus->channelByType(Channel::Left)->data();
    const float * sourceR = numberOfInputChannels > 1 ? inputBus->channelByType(Channel::Right)->data() : sourceL;
    float * destinationL = outputBus->channelByType(Channel::Left)->mutableData();
    float * destinationR = outputBus->channelByType(Channel::Right)->mutableData();

    if (!sourceL || !sourceR || !destinationL || !destinationR)
        return;

    // Clamp azimuth to allowed range of -180 -> +180.
    azimuth = max(-180.0, azimuth);
    azimuth = min(180.0, azimuth);

    // Alias the azimuth ranges behind us to in front of us:
    // -90 -> -180 to -90 -> 0 and 90 -> 180 to 90 -> 0
    if (azimuth < -90)
        azimuth = -180 - azimuth;
    else if (azimuth > 90)
        azimuth = 180 - azimuth;

    double desiredPanPosition;
    double desiredGainL;
    double desiredGainR;

    if (numberOfInputChannels == 1)
    {  // For mono source case.
        // Pan smoothly from left to right with azimuth going from -90 -> +90 degrees.
        desiredPanPosition = (azimuth + 90) / 180;
    }
    else
    {  // For stereo source case.
        if (azimuth <= 0)
        {  // from -90 -> 0
            // sourceL -> destL and "equal-power pan" sourceR as in mono case
            // by transforming the "azimuth" value from -90 -> 0 degrees into the range -90 -> +90.
            desiredPanPosition = (azimuth + 90) / 90;
        }
        else
        {  // from 0 -> +90
            // sourceR -> destR and "equal-power pan" sourceL as in mono case
            // by transforming the "azimuth" value from 0 -> +90 degrees into the range -90 -> +90.
            desiredPanPosition = azimuth / 90;
        }
    }

    desiredGainL = cos(0.5 * static_cast<double>(LAB_PI) * desiredPanPosition);
    desiredGainR = sin(0.5 * static_cast<double>(LAB_PI) * desiredPanPosition);

    // Don't de-zipper on first render call.
    if (m_isFirstRender)
    {
        m_isFirstRender = false;
        m_gainL = desiredGainL;
        m_gainR = desiredGainR;
    }

    // Cache in local variables.
    double gainL = m_gainL;
    double gainR = m_gainR;

    // Get local copy of smoothing constant.
    const double SmoothingConstant = m_smoothingConstant;

    int n = framesToProcess;

    if (numberOfInputChannels == 1)
    {  // For mono source case.
        while (n--)
        {
            float inputL = *sourceL++;
            gainL += (desiredGainL - gainL) * SmoothingConstant;
            gainR += (desiredGainR - gainR) * SmoothingConstant;
            *destinationL++ = static_cast<float>(inputL * gainL);
            *destinationR++ = static_cast<float>(inputL * gainR);
        }
    }
    else
    {  // For stereo source case.
        if (azimuth <= 0)
        {  // from -90 -> 0
            while (n--)
            {
                float inputL = *sourceL++;
                float inputR = *sourceR++;
                gainL += (desiredGainL - gainL) * SmoothingConstant;
                gainR += (desiredGainR - gainR) * SmoothingConstant;
                *destinationL++ = static_cast<float>(inputL + inputR * gainL);
                *destinationR++ = static_cast<float>(inputR * gainR);
            }
        }
        else
        {  // from 0 -> +90
            while (n--)
            {
                float inputL = *sourceL++;
                float inputR = *sourceR++;
                gainL += (desiredGainL - gainL) * SmoothingConstant;
                gainR += (desiredGainR - gainR) * SmoothingConstant;
                *destinationL++ = static_cast<float>(inputL * gainL);
                *destinationR++ = static_cast<float>(inputR + inputL * gainR);
            }
        }
    }

    m_gainL = gainL;
    m_gainR = gainR;
}

}  // namespace lab
