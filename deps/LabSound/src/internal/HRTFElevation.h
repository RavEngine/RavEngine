// License: BSD 3 Clause
// Copyright (C) 2010, Google Inc. All rights reserved.
// Copyright (C) 2015+, The LabSound Authors. All rights reserved.

#ifndef HRTFElevation_h
#define HRTFElevation_h

#include "LabSound/extended/Util.h"

#include "internal/HRTFKernel.h"

#include <string>

namespace lab
{

// HRTFElevation contains all of the HRTFKernels (one left ear and one right ear per azimuth angle) for a particular elevation.
class HRTFElevation
{

public:
    // Loads and returns an HRTFElevation with the given HRTF database subject name and elevation from resources.
    // Normally, there will only be a single HRTF database set, but this API supports the possibility of multiple ones with different names.
    // Interpolated azimuths will be generated based on InterpolationFactor.
    // Valid values for elevation are -45 -> +90 in 15 degree increments.
    static std::unique_ptr<HRTFElevation> createForSubject(HRTFDatabaseInfo * info, int elevation);

    // Given two HRTFElevations, and an interpolation factor x: 0 -> 1, returns an interpolated HRTFElevation.
    static std::unique_ptr<HRTFElevation> createByInterpolatingSlices(HRTFDatabaseInfo * info, HRTFElevation * hrtfElevation1, HRTFElevation * hrtfElevation2, float x);

    // Returns the list of left or right ear HRTFKernels for all the azimuths going from 0 to 360 degrees.
    HRTFKernelList * kernelListL() { return m_kernelListL.get(); }
    HRTFKernelList * kernelListR() { return m_kernelListR.get(); }

    double elevationAngle() const { return m_elevationAngle; }
    unsigned numberOfAzimuths() const { return NumberOfTotalAzimuths; }

    // Returns the left and right kernels for the given azimuth index.
    // The interpolated delays based on azimuthBlend: 0 -> 1 are returned in frameDelayL and frameDelayR.
    void getKernelsFromAzimuth(double azimuthBlend, unsigned azimuthIndex, HRTFKernel *& kernelL, HRTFKernel *& kernelR, double & frameDelayL, double & frameDelayR);

    // Spacing, in degrees, between every azimuth loaded from resource.
    static const unsigned AzimuthSpacing;

    // Number of azimuths loaded from resource.
    static const unsigned NumberOfRawAzimuths;

    // Interpolates by this factor to get the total number of azimuths from every azimuth loaded from resource.
    static const unsigned InterpolationFactor;

    // Total number of azimuths after interpolation.
    static const unsigned NumberOfTotalAzimuths;

    // Given a specific azimuth and elevation angle, returns the left and right HRTFKernel.
    // Valid values for azimuth are 0 -> 345 in 15 degree increments.
    // Valid values for elevation are -45 -> +90 in 15 degree increments.
    // Returns true on success.
    static bool calculateKernelsForAzimuthElevation(HRTFDatabaseInfo * info, int azimuth, int elevation, std::shared_ptr<HRTFKernel> & kernelL, std::shared_ptr<HRTFKernel> & kernelR);

    // Given a specific azimuth and elevation angle, returns the left and right HRTFKernel in kernelL and kernelR.
    // This method averages the measured response using symmetry of azimuth (for example by averaging the -30.0 and +30.0 azimuth responses).
    // Returns true on success.
    static bool calculateSymmetricKernelsForAzimuthElevation(HRTFDatabaseInfo * info, int azimuth, int elevation, std::shared_ptr<HRTFKernel> & kernelL, std::shared_ptr<HRTFKernel> & kernelR);

private:
    HRTFElevation(HRTFDatabaseInfo * info, std::unique_ptr<HRTFKernelList> kernelListL, std::unique_ptr<HRTFKernelList> kernelListR, int elevation)
        : m_kernelListL(std::move(kernelListL))
        , m_kernelListR(std::move(kernelListR))
        , m_elevationAngle(elevation)
    {
    }

    std::unique_ptr<HRTFKernelList> m_kernelListL;
    std::unique_ptr<HRTFKernelList> m_kernelListR;

    double m_elevationAngle;
};

}  // namespace lab

#endif  // HRTFElevation_h
