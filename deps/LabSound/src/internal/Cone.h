// License: BSD 3 Clause
// Copyright (C) 2010, Google Inc. All rights reserved.
// Copyright (C) 2015+, The LabSound Authors. All rights reserved.

#ifndef Cone_h
#define Cone_h

#include "LabSound/core/FloatPoint3D.h"

namespace lab
{

// Cone gain is defined according to the OpenAL specification

class ConeEffect
{
public:
    ConeEffect();

    // Returns scalar gain for the given source/listener positions/orientations
    double gain(FloatPoint3D sourcePosition, FloatPoint3D sourceOrientation, FloatPoint3D listenerPosition);

    // Angles in degrees
    void setInnerAngle(double innerAngle) { m_innerAngle = innerAngle; }
    double innerAngle() const { return m_innerAngle; }

    void setOuterAngle(double outerAngle) { m_outerAngle = outerAngle; }
    double outerAngle() const { return m_outerAngle; }

    void setOuterGain(double outerGain) { m_outerGain = outerGain; }
    double outerGain() const { return m_outerGain; }

protected:
    double m_innerAngle;
    double m_outerAngle;
    double m_outerGain;
};

}  // namespace lab

#endif  // Cone_h
