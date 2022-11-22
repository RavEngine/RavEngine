// License: BSD 3 Clause
// Copyright (C) 2010, Google Inc. All rights reserved.
// Copyright (C) 2015+, The LabSound Authors. All rights reserved.

#include "LabSound/core/AudioBus.h"
#include "LabSound/extended/AudioFileReader.h"

#include "internal/Assertions.h"
#include "internal/Biquad.h"
#include "internal/FFTFrame.h"
#include "internal/HRTFElevation.h"
#include "internal/HRTFPanner.h"

#include <algorithm>
#include <iostream>
#include <map>
#include <math.h>
#include <string>

using namespace std;

#if defined(_MSC_VER)
#pragma warning(disable:4996)
#endif

namespace lab
{

// The range of elevations for the IRCAM impulse responses varies depending on azimuth, but the minimum elevation appears to always be -45.
static int maxElevations[] = {
    // Azimuth
    90,  // 0
    45,  // 15
    60,  // 30
    45,  // 45
    75,  // 60
    45,  // 75
    60,  // 90
    45,  // 105
    75,  // 120
    45,  // 135
    60,  // 150
    45,  // 165
    75,  // 180
    45,  // 195
    60,  // 210
    45,  // 225
    75,  // 240
    45,  // 255
    60,  // 270
    45,  // 285
    75,  // 300
    45,  // 315
    60,  // 330
    45  // 345
};

const uint32_t HRTFElevation::AzimuthSpacing = 15;
const uint32_t HRTFElevation::NumberOfRawAzimuths = 360 / AzimuthSpacing;
const uint32_t HRTFElevation::InterpolationFactor = 8;
const uint32_t HRTFElevation::NumberOfTotalAzimuths = NumberOfRawAzimuths * InterpolationFactor;

// Takes advantage of the symmetry and creates a composite version of the two measured versions.  For example, we have both azimuth 30 and -30 degrees
// where the roles of left and right ears are reversed with respect to each other.
bool HRTFElevation::calculateSymmetricKernelsForAzimuthElevation(HRTFDatabaseInfo * info, int azimuth, int elevation, std::shared_ptr<HRTFKernel> & kernelL, std::shared_ptr<HRTFKernel> & kernelR)
{
    std::shared_ptr<HRTFKernel> kernelL1;
    std::shared_ptr<HRTFKernel> kernelR1;

    bool success = calculateKernelsForAzimuthElevation(info, azimuth, elevation, kernelL1, kernelR1);

    if (!success)
        return false;

    // And symmetric version
    int symmetricAzimuth = !azimuth ? 0 : 360 - azimuth;

    std::shared_ptr<HRTFKernel> kernelL2;
    std::shared_ptr<HRTFKernel> kernelR2;

    success = calculateKernelsForAzimuthElevation(info, symmetricAzimuth, elevation, kernelL2, kernelR2);

    if (!success) return false;

    // Notice L/R reversal in symmetric version.
    kernelL = MakeInterpolatedKernel(kernelL1.get(), kernelR2.get(), 0.5f);
    kernelR = MakeInterpolatedKernel(kernelR1.get(), kernelL2.get(), 0.5f);

    return true;
}

bool HRTFElevation::calculateKernelsForAzimuthElevation(HRTFDatabaseInfo * info, int azimuth, int elevation, std::shared_ptr<HRTFKernel> & kernelL, std::shared_ptr<HRTFKernel> & kernelR)
{
    // Valid values for azimuth are 0 -> 345 in 15 degree increments.
    // Valid values for elevation are -45 -> +90 in 15 degree increments.

    bool isAzimuthGood = azimuth >= 0 && azimuth <= 345 && (azimuth / 15) * 15 == azimuth;
    ASSERT(isAzimuthGood);
    if (!isAzimuthGood) return false;

    bool isElevationGood = elevation >= -45 && elevation <= 90 && (elevation / 15) * 15 == elevation;
    ASSERT(isElevationGood);
    if (!isElevationGood) return false;

    // Construct the resource name from the subject name, azimuth, and elevation, for example:
    // "IRC_Composite_C_R0195_T015_P000"
    int positiveElevation = elevation < 0 ? elevation + 360 : elevation;

    char tempStr[16];

    // Located in $searchPath / [format] .wav
    // @tofix - this assumes we want to open this path and read via libnyquist fopen.
    // ... will need to change for Android. Maybe MakeBusFromInternalResource / along with a LoadInternalResources requried by LabSound
    sprintf(tempStr, "%03d_P%03d", azimuth, positiveElevation);
    std::string resourceName = info->searchPath + "/" + "IRC_" + info->subjectName + "_C_R0195_T" + tempStr + ".wav";

    std::shared_ptr<AudioBus> impulseResponse = lab::MakeBusFromFile(resourceName.c_str(), false);

    if (!impulseResponse)
    {
        LOG_ERROR("impulse not found %s (bad path?)", resourceName.c_str());
        return false;
    }

    // The impulse files are 44.1k so we need to resample them if the graph is playing back at any other rate
    if (info->sampleRate != 44100.0f)
    {
        std::unique_ptr<AudioBus> resampledImpulseResponse = AudioBus::createBySampleRateConverting(impulseResponse.get(), false, info->sampleRate);
        impulseResponse = std::move(resampledImpulseResponse);
        // LOG_VERBOSE("converting %s impulse to %f hz", resourceName.c_str(), info->sampleRate);
    }

    // Check number of channels. For now these are fixed and known.
    bool isBusGood = (impulseResponse->numberOfChannels() == Channels::Stereo);

    ASSERT(isBusGood);
    if (!isBusGood)
    {
        return false;
    }

    AudioChannel * leftEarImpulseResponse = impulseResponse->channelByType(Channel::Left);
    AudioChannel * rightEarImpulseResponse = impulseResponse->channelByType(Channel::Right);

    // Note that depending on the fftSize returned by the panner, we may be truncating the impulse response we just loaded in.
    const int fftSize = HRTFPanner::fftSizeForSampleRate(info->sampleRate);
    kernelL = std::make_shared<HRTFKernel>(leftEarImpulseResponse, fftSize, info->sampleRate);
    kernelR = std::make_shared<HRTFKernel>(rightEarImpulseResponse, fftSize, info->sampleRate);

    return true;
}

std::unique_ptr<HRTFElevation> HRTFElevation::createForSubject(HRTFDatabaseInfo * info, int elevation)
{
    bool isElevationGood = elevation >= -45 && elevation <= 90 && (elevation / 15) * 15 == elevation;
    ASSERT(isElevationGood);

    if (!isElevationGood)
        return nullptr;

    std::unique_ptr<HRTFKernelList> kernelListL = std::unique_ptr<HRTFKernelList>(new HRTFKernelList(NumberOfTotalAzimuths));
    std::unique_ptr<HRTFKernelList> kernelListR = std::unique_ptr<HRTFKernelList>(new HRTFKernelList(NumberOfTotalAzimuths));

    // Load convolution kernels from HRTF files.
    int interpolatedIndex = 0;
    for (uint32_t rawIndex = 0; rawIndex < NumberOfRawAzimuths; ++rawIndex)
    {
        // Don't let elevation exceed maximum for this azimuth.
        int maxElevation = maxElevations[rawIndex];
        int actualElevation = min(elevation, maxElevation);

        bool success = calculateKernelsForAzimuthElevation(info, rawIndex * AzimuthSpacing, actualElevation, kernelListL->at(interpolatedIndex), kernelListR->at(interpolatedIndex));
        if (!success)
            return nullptr;

        interpolatedIndex += InterpolationFactor;
    }

    // Now go back and interpolate intermediate azimuth values.
    for (uint32_t i = 0; i < NumberOfTotalAzimuths; i += InterpolationFactor)
    {
        int j = (i + InterpolationFactor) % NumberOfTotalAzimuths;

        // Create the interpolated convolution kernels and delays.
        for (uint32_t jj = 1; jj < InterpolationFactor; ++jj)
        {
            float x = float(jj) / float(InterpolationFactor);  // interpolate from 0 -> 1

            (*kernelListL)[i + jj] = MakeInterpolatedKernel(kernelListL->at(i).get(), kernelListL->at(j).get(), x);
            (*kernelListR)[i + jj] = MakeInterpolatedKernel(kernelListR->at(i).get(), kernelListR->at(j).get(), x);
        }
    }

    return std::unique_ptr<HRTFElevation>(new HRTFElevation(info, std::move(kernelListL), std::move(kernelListR), elevation));
}

std::unique_ptr<HRTFElevation> HRTFElevation::createByInterpolatingSlices(HRTFDatabaseInfo * info, HRTFElevation * hrtfElevation1, HRTFElevation * hrtfElevation2, float x)
{
    ASSERT(hrtfElevation1 && hrtfElevation2);
    if (!hrtfElevation1 || !hrtfElevation2)
        return nullptr;

    ASSERT(x >= 0.0 && x < 1.0);

    std::unique_ptr<HRTFKernelList> kernelListL = std::unique_ptr<HRTFKernelList>(new HRTFKernelList(NumberOfTotalAzimuths));
    std::unique_ptr<HRTFKernelList> kernelListR = std::unique_ptr<HRTFKernelList>(new HRTFKernelList(NumberOfTotalAzimuths));

    HRTFKernelList * kernelListL1 = hrtfElevation1->kernelListL();
    HRTFKernelList * kernelListR1 = hrtfElevation1->kernelListR();
    HRTFKernelList * kernelListL2 = hrtfElevation2->kernelListL();
    HRTFKernelList * kernelListR2 = hrtfElevation2->kernelListR();

    // Interpolate kernels of corresponding azimuths of the two elevations.
    for (uint32_t i = 0; i < NumberOfTotalAzimuths; ++i)
    {
        (*kernelListL)[i] = MakeInterpolatedKernel(kernelListL1->at(i).get(), kernelListL2->at(i).get(), x);
        (*kernelListR)[i] = MakeInterpolatedKernel(kernelListR1->at(i).get(), kernelListR2->at(i).get(), x);
    }

    // Interpolate elevation angle.
    double angle = (1.0 - x) * hrtfElevation1->elevationAngle() + x * hrtfElevation2->elevationAngle();

    return std::unique_ptr<HRTFElevation>(new HRTFElevation(info, std::move(kernelListL), std::move(kernelListR), (int) angle));
}

void HRTFElevation::getKernelsFromAzimuth(double azimuthBlend, unsigned azimuthIndex, HRTFKernel *& kernelL, HRTFKernel *& kernelR, double & frameDelayL, double & frameDelayR)
{
    bool checkAzimuthBlend = azimuthBlend >= 0.0 && azimuthBlend < 1.0;
    ASSERT(checkAzimuthBlend);
    if (!checkAzimuthBlend)
    {
        azimuthBlend = 0.0;
    }

    size_t numKernels = m_kernelListL->size();

    bool isIndexGood = azimuthIndex < numKernels;
    ASSERT(isIndexGood);
    if (!isIndexGood)
    {
        kernelL = 0;
        kernelR = 0;
        return;
    }

    // Return the left and right kernels.
    kernelL = m_kernelListL->at(azimuthIndex).get();
    kernelR = m_kernelListR->at(azimuthIndex).get();

    frameDelayL = m_kernelListL->at(azimuthIndex)->frameDelay();
    frameDelayR = m_kernelListR->at(azimuthIndex)->frameDelay();

    int azimuthIndex2 = (azimuthIndex + 1) % numKernels;
    double frameDelay2L = m_kernelListL->at(azimuthIndex2)->frameDelay();
    double frameDelay2R = m_kernelListR->at(azimuthIndex2)->frameDelay();

    // Linearly interpolate delays.
    frameDelayL = (1.0 - azimuthBlend) * frameDelayL + azimuthBlend * frameDelay2L;
    frameDelayR = (1.0 - azimuthBlend) * frameDelayR + azimuthBlend * frameDelay2R;
}

}  // namespace lab
