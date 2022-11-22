// SPDX-License-Identifier: BSD-2-Clause
// Copyright (C) 2015+, The LabSound Authors. All rights reserved.

#ifndef LABSOUND_UTIL_H
#define LABSOUND_UTIL_H

#include <algorithm>
#include <random>
#include <string>

// clang-format off
#define NO_COPY(C) C(const C &) = delete; C & operator = (const C &) = delete
#define NO_MOVE(C) NO_COPY(C); C(C &&) = delete; C & operator = (const C &&) = delete

#if defined(_MSC_VER)
#   if defined(min)
#       undef min
#   endif
#   if defined(max)
#       undef max
#   endif
#endif

namespace lab
{

class UniformRandomGenerator
{
    std::random_device rd;
    std::mt19937_64 gen;
    std::uniform_real_distribution<float> dist_full {0.f, 1.f};

public:
    UniformRandomGenerator() : rd(), gen(rd()) {}
    float random_float() { return dist_full(gen); }  // [0.f, 1.f]
    float random_float(float max) { std::uniform_real_distribution<float> dist_user(0.f, max); return dist_user(gen); }
    float random_float(float min, float max) { std::uniform_real_distribution<float> dist_user(min, max);  return dist_user(gen); }
    uint32_t random_uint(uint32_t max) { std::uniform_int_distribution<uint32_t> dist_int(0, max); return dist_int(gen); }
    int32_t random_int(int32_t min, int32_t max) { std::uniform_int_distribution<int32_t> dist_int(min, max); return dist_int(gen); }
};
// clang-format on

template <typename T>
inline T RoundNextPow2(T v)
{
    v--;
    v |= v >> 1;
    v |= v >> 2;
    v |= v >> 4;
    v |= v >> 8;
    v |= v >> 16;
    v++;
    return v;
}

template <typename S, typename T>
inline T clampTo(S value, T min, T max)
{
    if (value >= static_cast<S>(max)) return max;
    if (value <= static_cast<S>(min)) return min;
    return static_cast<T>(value);
}

// Hard-coded to the IRCAM HRTF Database
struct HRTFDatabaseInfo
{
    std::string subjectName;
    std::string searchPath;
    float sampleRate;

    int minElevation = -45;
    int maxElevation = 90;
    int rawElevationAngleSpacing = 15;

    // Number of elevations loaded from resource
    int numberOfRawElevations = 10;  // -45 -> +90 (each 15 degrees)

    // Interpolates by this factor to get the total number of elevations from every elevation loaded from resource
    int interpolationFactor = 1;

    // Total number of elevations after interpolation.
    int numTotalElevations;

    HRTFDatabaseInfo(const std::string & subjectName, const std::string & searchPath, float sampleRate)
        : subjectName(subjectName)
        , searchPath(searchPath)
        , sampleRate(sampleRate)
    {
        numTotalElevations = numberOfRawElevations * interpolationFactor;
    }

    // Returns the index for the correct HRTFElevation given the elevation angle.
    int indexFromElevationAngle(double elevationAngle)
    {
        elevationAngle = std::max((double) minElevation, elevationAngle);
        elevationAngle = std::min((double) maxElevation, elevationAngle);
        return (int) (interpolationFactor * (elevationAngle - minElevation) / rawElevationAngleSpacing);
    }
};
}

#endif
