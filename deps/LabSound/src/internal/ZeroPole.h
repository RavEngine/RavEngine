// License: BSD 3 Clause
// Copyright (C) 2011, Google Inc. All rights reserved.
// Copyright (C) 2015+, The LabSound Authors. All rights reserved.

#ifndef ZeroPole_h
#define ZeroPole_h

#include <cstddef>

namespace lab
{

// ZeroPole is a simple filter with one zero and one pole.
class ZeroPole
{
    float m_zero = 0.f;
    float m_pole = 0.f;
    float m_lastX = 0.f;
    float m_lastY = 0.f;

public:
    ZeroPole() {}

    void process(const float * source, float * destination, int bufferSize, int offset, int count);

    // Reset filter state.
    void reset()
    {
        m_lastX = 0;
        m_lastY = 0;
    }

    void setZero(float zero) { m_zero = zero; }
    void setPole(float pole) { m_pole = pole; }

    float zero() const { return m_zero; }
    float pole() const { return m_pole; }
};

}  // namespace lab

#endif  // ZeroPole_h
