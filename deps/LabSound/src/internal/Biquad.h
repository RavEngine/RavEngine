// License: BSD 3 Clause
// Copyright (C) 2010, Google Inc. All rights reserved.
// Copyright (C) 2015+, The LabSound Authors. All rights reserved.

#ifndef Biquad_h
#define Biquad_h

#include "LabSound/core/AudioArray.h"

#include "LabSound/core/Macros.h"

#include <complex>
#include <sys/types.h>

namespace lab
{

// A basic biquad (two-zero / two-pole digital filter)
// It can be configured to a number of common and very useful filters:
// lowpass, highpass, shelving, parameteric, notch, allpass, ...
class Biquad
{
public:
    Biquad();
    virtual ~Biquad();

    void process(const float * sourceP, float * destP, int framesToProcess);

    // frequency is 0 - 1 normalized, resonance and dbGain are in decibels.
    // Q is a unitless quality factor.
    void setLowpassParams(double frequency, double resonance);
    void setHighpassParams(double frequency, double resonance);
    void setBandpassParams(double frequency, double Q);
    void setLowShelfParams(double frequency, double dbGain);
    void setHighShelfParams(double frequency, double dbGain);
    void setPeakingParams(double frequency, double Q, double dbGain);
    void setAllpassParams(double frequency, double Q);
    void setNotchParams(double frequency, double Q);

    // Set the biquad coefficients given a single zero (other zero will be conjugate)
    // and a single pole (other pole will be conjugate)
    void setZeroPolePairs(const std::complex<double> & zero, const std::complex<double> & pole);

    // Set the biquad coefficients given a single pole (other pole will be conjugate)
    // (The zeroes will be the inverse of the poles)
    void setAllpassPole(const std::complex<double> & pole);

    // Resets filter state
    void reset();

    // Filter response at a set of n frequencies. The magnitude and
    // phase response are returned in magResponse and phaseResponse.
    // The phase response is in radians.
    void getFrequencyResponse(size_t nFrequencies, const float * frequency, float * magResponse, float * phaseResponse);

private:
    void setNormalizedCoefficients(double b0, double b1, double b2, double a0, double a1, double a2);

    // Filter coefficients. The filter is defined as
    // y[n] + m_a1*y[n-1] + m_a2*y[n-2] = m_b0*x[n] + m_b1*x[n-1] + m_b2*x[n-2].
    double m_b0;
    double m_b1;
    double m_b2;
    double m_a1;
    double m_a2;

    // Filter memory
    double m_x1;  // input delayed by 1 sample
    double m_x2;  // input delayed by 2 samples
    double m_y1;  // output delayed by 1 sample
    double m_y2;  // output delayed by 2 samples
};

}  // namespace lab

#endif  // Biquad_h
