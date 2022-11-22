// License: BSD 3 Clause
// Copyright (C) 2010, Google Inc. All rights reserved.
// Copyright (C) 2015+, The LabSound Authors. All rights reserved.

#ifndef Distance_h
#define Distance_h

namespace lab
{

// Distance models are defined according to the OpenAL specification:
// http://connect.creativelabs.com/openal/Documentation/OpenAL%201.1%20Specification.htm.
class DistanceEffect
{

public:
    enum ModelType
    {
        ModelLinear = 0,
        ModelInverse = 1,
        ModelExponential = 2,
        _Count = 3
    };

    DistanceEffect();

    // Returns scalar gain for the given distance the current distance model is used
    double gain(double distance);

    ModelType model() { return m_model; }

    void setModel(ModelType model, bool clamped)
    {
        m_model = model;
        m_isClamped = clamped;
    }

    // Distance params
    void setRefDistance(double refDistance) { m_refDistance = refDistance; }
    void setMaxDistance(double maxDistance) { m_maxDistance = maxDistance; }
    void setRolloffFactor(double rolloffFactor) { m_rolloffFactor = rolloffFactor; }

    double refDistance() const { return m_refDistance; }
    double maxDistance() const { return m_maxDistance; }
    double rolloffFactor() const { return m_rolloffFactor; }

protected:
    double linearGain(double distance);
    double inverseGain(double distance);
    double exponentialGain(double distance);

    ModelType m_model = ModelInverse;

    bool m_isClamped = true;
    double m_refDistance = 1.0;
    double m_maxDistance = 10000.0;
    double m_rolloffFactor = 1.0;
};

}  // namespace lab

#endif  // Distance_h
