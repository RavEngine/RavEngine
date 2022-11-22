// License: BSD 3 Clause
// Copyright (C) 2010, Google Inc. All rights reserved.
// Copyright (C) 2015+, The LabSound Authors. All rights reserved.

#include "internal/Distance.h"
#include "internal/Assertions.h"

#include <algorithm>
#include <math.h>

using namespace std;

namespace lab
{

DistanceEffect::DistanceEffect()
{
}

double DistanceEffect::gain(double distance)
{
    // don't go beyond maximum distance
    distance = min(distance, m_maxDistance);

    // if clamped, don't get closer than reference distance
    if (m_isClamped)
        distance = max(distance, m_refDistance);

    switch (m_model)
    {
        default:
        case ModelLinear:
            return linearGain(distance);
        case ModelInverse:
            return inverseGain(distance);
        case ModelExponential:
            return exponentialGain(distance);
    }
    ASSERT_NOT_REACHED();
    return 0.0;
}

double DistanceEffect::linearGain(double distance)
{
    // We want a gain that decreases linearly from m_refDistance to
    // m_maxDistance. The gain is 1 at m_refDistance.
    return (1.0 - m_rolloffFactor * (distance - m_refDistance) / (m_maxDistance - m_refDistance));
}

double DistanceEffect::inverseGain(double distance)
{
    return m_refDistance / (m_refDistance + m_rolloffFactor * (distance - m_refDistance));
}

double DistanceEffect::exponentialGain(double distance)
{
    return pow(distance / m_refDistance, -m_rolloffFactor);
}

}  // namespace lab

