// License: BSD 3 Clause
// Copyright (C) 2010, Google Inc. All rights reserved.
// Copyright (C) 2015+, The LabSound Authors. All rights reserved.

#include "internal/HRTFDatabase.h"
#include "internal/Assertions.h"
#include "internal/HRTFElevation.h"

#include <stdexcept>

using namespace std;

namespace lab
{

HRTFDatabase::HRTFDatabase(float sampleRate, const std::string & searchPath)
{
    info.reset(new HRTFDatabaseInfo("Composite", searchPath, sampleRate));

    m_elevations.resize(info->numTotalElevations);

    int elevationIndex = 0;
    for (int elevation = info->minElevation; elevation <= info->maxElevation; elevation += info->rawElevationAngleSpacing)
    {
        std::unique_ptr<HRTFElevation> hrtfElevation = HRTFElevation::createForSubject(info.get(), elevation);

        // @tofix - removed ASSERT(hrtfElevation.get());
        if (!hrtfElevation.get()) return;

        m_elevations[elevationIndex] = std::move(hrtfElevation);
        elevationIndex += info->interpolationFactor;
    }

    // Go back and interpolate elevations
    if (info->interpolationFactor > 1)
    {
        for (int i = 0; i < info->numTotalElevations; i += info->interpolationFactor)
        {
            int j = (i + info->interpolationFactor);

            if (j >= info->numTotalElevations)
            {
                j = i;  // for last elevation interpolate with itself
            }

            // Create the interpolated convolution kernels and delays.
            for (int jj = 1; jj < info->interpolationFactor; ++jj)
            {
                float x = static_cast<float>(jj) / static_cast<float>(info->interpolationFactor);
                m_elevations[i + jj] = HRTFElevation::createByInterpolatingSlices(info.get(), m_elevations[i].get(), m_elevations[j].get(), x);
                ASSERT(m_elevations[i + jj].get());
            }
        }
    }
}

void HRTFDatabase::getKernelsFromAzimuthElevation(double azimuthBlend,
                                                  unsigned azimuthIndex,
                                                  double elevationAngle,
                                                  HRTFKernel *& kernelL,
                                                  HRTFKernel *& kernelR,
                                                  double & frameDelayL,
                                                  double & frameDelayR)
{

    size_t elevationIndex = info->indexFromElevationAngle(elevationAngle);

    ASSERT(elevationIndex < m_elevations.size() && m_elevations.size() > 0);

    if (!m_elevations.size())
    {
        kernelL = 0;
        kernelR = 0;
        return;
    }

    if (elevationIndex > m_elevations.size() - 1)
    {
        elevationIndex = m_elevations.size() - 1;
    }

    HRTFElevation * hrtfElevation = m_elevations[elevationIndex].get();

    ASSERT(hrtfElevation);

    if (!hrtfElevation)
    {
        throw std::runtime_error("Error getting hrtfElevation");
    }

    hrtfElevation->getKernelsFromAzimuth(azimuthBlend, azimuthIndex, kernelL, kernelR, frameDelayL, frameDelayR);
}

}  // namespace lab
