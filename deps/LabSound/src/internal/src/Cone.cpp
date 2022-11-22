// License: BSD 3 Clause
// Copyright (C) 2010, Google Inc. All rights reserved.
// Copyright (C) 2015+, The LabSound Authors. All rights reserved.

#include "internal/Cone.h"
#include "LabSound/core/Macros.h"

namespace lab
{

ConeEffect::ConeEffect()
    : m_innerAngle(360.0)
    , m_outerAngle(360.0)
    , m_outerGain(0.0)
{
}

double ConeEffect::gain(FloatPoint3D sourcePosition, FloatPoint3D sourceOrientation, FloatPoint3D listenerPosition)
{
    if (is_zero(sourceOrientation) || ((m_innerAngle == 360.0) && (m_outerAngle == 360.0)))
        return 1.0;  // no cone specified - unity gain

    // Normalized source-listener vector
    FloatPoint3D sourceToListener = listenerPosition - sourcePosition;
    sourceToListener = normalize(sourceToListener);

    FloatPoint3D normalizedSourceOrientation = sourceOrientation;
    normalizedSourceOrientation = normalize(normalizedSourceOrientation);

    // Angle between the source orientation vector and the source-listener vector
    double dotProduct = dot(sourceToListener, normalizedSourceOrientation);
    double angle = 180.0 * acos(dotProduct) / static_cast<double>(LAB_PI);
    double absAngle = fabs(angle);

    // Divide by 2.0 here since API is entire angle (not half-angle)
    double absInnerAngle = fabs(m_innerAngle) / 2.0;
    double absOuterAngle = fabs(m_outerAngle) / 2.0;
    double gain = 1.0;

    if (absAngle <= absInnerAngle)
        // No attenuation
        gain = 1.0;
    else if (absAngle >= absOuterAngle)
        // Max attenuation
        gain = m_outerGain;
    else
    {
        // Between inner and outer cones
        // inner -> outer, x goes from 0 -> 1
        double x = (absAngle - absInnerAngle) / (absOuterAngle - absInnerAngle);
        gain = (1.0 - x) + m_outerGain * x;
    }

    return gain;
}

}  // namespace lab
