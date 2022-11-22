// License: BSD 3 Clause
// Copyright (C) 2010, Google Inc. All rights reserved.
// Copyright (C) 2015+, The LabSound Authors. All rights reserved.

#ifndef HRTFDatabase_h
#define HRTFDatabase_h

#include "LabSound/extended/Util.h"
#include "internal/HRTFElevation.h"

#include <vector>

namespace lab
{

class HRTFKernel;
class HRTFDatabase
{

    NO_MOVE(HRTFDatabase);

public:
    HRTFDatabase(float sampleRate, const std::string & searchPath);

    // getKernelsFromAzimuthElevation() returns a left and right ear kernel, and an interpolated left and right frame delay for the given azimuth and elevation.
    // azimuthBlend must be in the range 0 -> 1.
    // Valid values for azimuthIndex are 0 -> HRTFElevation::NumberOfTotalAzimuths - 1 (corresponding to angles of 0 -> 360).
    // Valid values for elevationAngle are MinElevation -> MaxElevation.
    void getKernelsFromAzimuthElevation(double azimuthBlend, unsigned azimuthIndex, double elevationAngle, HRTFKernel *& kernelL, HRTFKernel *& kernelR, double & frameDelayL, double & frameDelayR);

    // Returns the number of different azimuth angles.
    static unsigned numberOfAzimuths() { return HRTFElevation::NumberOfTotalAzimuths; }

private:
    std::vector<std::unique_ptr<HRTFElevation>> m_elevations;

    std::unique_ptr<HRTFDatabaseInfo> info;
};

}  // namespace lab

#endif  // HRTFDatabase_h
