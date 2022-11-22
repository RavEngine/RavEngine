// License: BSD 3 Clause
// Copyright (C) 2010, Google Inc. All rights reserved.
// Copyright (C) 2015+, The LabSound Authors. All rights reserved.

#include "internal/Biquad.h"
#include "internal/DenormalDisabler.h"

#include "LabSound/core/Macros.h"

#include <algorithm>
#include <stdio.h>

using namespace std;

namespace lab
{

Biquad::Biquad()
{
    // Initialize as pass-thru (straight-wire, no filter effect)
    setNormalizedCoefficients(1, 0, 0, 1, 0, 0);
    reset();  // clear filter memory
}

Biquad::~Biquad() { }

void Biquad::process(const float * sourceP, float * destP, int framesToProcess)
{
    int n = framesToProcess;

    // Biquad needs to be double precision at 48khz for frequencies less than 500Hz
    // according to multiple references.

    // Create local copies of member variables
    double x1 = m_x1;
    double x2 = m_x2;
    double y1 = m_y1;
    double y2 = m_y2;

    double b0 = m_b0;
    double b1 = m_b1;
    double b2 = m_b2;
    double a1 = m_a1;
    double a2 = m_a2;

    while (n--)
    {
        float x = *sourceP++;
        float y = static_cast<float>(b0 * x + b1 * x1 + b2 * x2 - a1 * y1 - a2 * y2);

        *destP++ = y;

        // Update state variables
        x2 = x1;
        x1 = x;
        y2 = y1;
        y1 = y;
    }

    m_x1 = x1;// DenormalDisabler::flushDenormalFloatToZero(x1);
    m_x2 = x2;// DenormalDisabler::flushDenormalFloatToZero(x2);
    m_y1 = y1;// DenormalDisabler::flushDenormalFloatToZero(y1);
    m_y2 = y2;// DenormalDisabler::flushDenormalFloatToZero(y2);

    m_b0 = b0;
    m_b1 = b1;
    m_b2 = b2;
    m_a1 = a1;
    m_a2 = a2;
}

void Biquad::reset()
{
    m_x1 = m_x2 = m_y1 = m_y2 = 0;
}

void Biquad::setLowpassParams(double cutoff, double resonance)
{
    // Limit cutoff to 0 to 1.
    cutoff = std::max(0.0, std::min(cutoff, 1.0));

    if (cutoff == 1)
    {
        // When cutoff is 1, the z-transform is 1.
        setNormalizedCoefficients(1, 0, 0,
                                  1, 0, 0);
    }
    else if (cutoff > 0)
    {
        // Compute biquad coefficients for lowpass filter
        resonance = std::max(0.0, resonance);  // can't go negative
        double g = pow(10.0, 0.05 * resonance);
        double d = sqrt((4 - sqrt(16 - 16 / (g * g))) / 2);

        double theta = static_cast<double>(LAB_PI) * cutoff;
        double sn = 0.5 * d * sin(theta);
        double beta = 0.5 * (1 - sn) / (1 + sn);
        double gamma = (0.5 + beta) * cos(theta);
        double alpha = 0.25 * (0.5 + beta - gamma);

        double b0 = 2 * alpha;
        double b1 = 2 * 2 * alpha;
        double b2 = 2 * alpha;
        double a1 = 2 * -gamma;
        double a2 = 2 * beta;

        setNormalizedCoefficients(b0, b1, b2, 1, a1, a2);
    }
    else
    {
        // When cutoff is zero, nothing gets through the filter, so set
        // coefficients up correctly.
        setNormalizedCoefficients(0, 0, 0,
                                  1, 0, 0);
    }
}

void Biquad::setHighpassParams(double cutoff, double resonance)
{
    // Limit cutoff to 0 to 1.
    cutoff = std::max(0.0, std::min(cutoff, 1.0));

    if (cutoff == 1)
    {
        // The z-transform is 0.
        setNormalizedCoefficients(0, 0, 0,
                                  1, 0, 0);
    }
    else if (cutoff > 0)
    {
        // Compute biquad coefficients for highpass filter
        resonance = std::max(0.0, resonance);  // can't go negative
        double g = pow(10.0, 0.05 * resonance);
        double d = sqrt((4 - sqrt(16 - 16 / (g * g))) / 2);

        double theta = static_cast<double>(LAB_PI) * cutoff;
        double sn = 0.5 * d * sin(theta);
        double beta = 0.5 * (1 - sn) / (1 + sn);
        double gamma = (0.5 + beta) * cos(theta);
        double alpha = 0.25 * (0.5 + beta + gamma);

        double b0 = 2 * alpha;
        double b1 = 2 * -2 * alpha;
        double b2 = 2 * alpha;
        double a1 = 2 * -gamma;
        double a2 = 2 * beta;

        setNormalizedCoefficients(b0, b1, b2, 1, a1, a2);
    }
    else
    {
        // When cutoff is zero, we need to be careful because the above
        // gives a quadratic divided by the same quadratic, with poles
        // and zeros on the unit circle in the same place. When cutoff
        // is zero, the z-transform is 1.
        setNormalizedCoefficients(1, 0, 0,
                                  1, 0, 0);
    }
}

void Biquad::setNormalizedCoefficients(double b0, double b1, double b2, double a0, double a1, double a2)
{
    double a0Inverse = 1 / a0;

    m_b0 = b0 * a0Inverse;
    m_b1 = b1 * a0Inverse;
    m_b2 = b2 * a0Inverse;
    m_a1 = a1 * a0Inverse;
    m_a2 = a2 * a0Inverse;
}

void Biquad::setLowShelfParams(double frequency, double dbGain)
{
    // Clip frequencies to between 0 and 1, inclusive.
    frequency = std::max(0.0, std::min(frequency, 1.0));

    double A = pow(10.0, dbGain / 40);

    if (frequency == 1)
    {
        // The z-transform is a constant gain.
        setNormalizedCoefficients(A * A, 0, 0,
                                  1, 0, 0);
    }
    else if (frequency > 0)
    {
        double w0 = static_cast<double>(LAB_PI) * frequency;
        double S = 1;  // filter slope (1 is max value)
        double alpha = 0.5 * sin(w0) * sqrt((A + 1 / A) * (1 / S - 1) + 2);
        double k = cos(w0);
        double k2 = 2 * sqrt(A) * alpha;
        double aPlusOne = A + 1;
        double aMinusOne = A - 1;

        double b0 = A * (aPlusOne - aMinusOne * k + k2);
        double b1 = 2 * A * (aMinusOne - aPlusOne * k);
        double b2 = A * (aPlusOne - aMinusOne * k - k2);
        double a0 = aPlusOne + aMinusOne * k + k2;
        double a1 = -2 * (aMinusOne + aPlusOne * k);
        double a2 = aPlusOne + aMinusOne * k - k2;

        setNormalizedCoefficients(b0, b1, b2, a0, a1, a2);
    }
    else
    {
        // When frequency is 0, the z-transform is 1.
        setNormalizedCoefficients(1, 0, 0,
                                  1, 0, 0);
    }
}

void Biquad::setHighShelfParams(double frequency, double dbGain)
{
    // Clip frequencies to between 0 and 1, inclusive.
    frequency = std::max(0.0, std::min(frequency, 1.0));

    double A = pow(10.0, dbGain / 40);

    if (frequency == 1)
    {
        // The z-transform is 1.
        setNormalizedCoefficients(1, 0, 0,
                                  1, 0, 0);
    }
    else if (frequency > 0)
    {
        double w0 = static_cast<double>(LAB_PI) * frequency;
        double S = 1;  // filter slope (1 is max value)
        double alpha = 0.5 * sin(w0) * sqrt((A + 1 / A) * (1 / S - 1) + 2);
        double k = cos(w0);
        double k2 = 2 * sqrt(A) * alpha;
        double aPlusOne = A + 1;
        double aMinusOne = A - 1;

        double b0 = A * (aPlusOne + aMinusOne * k + k2);
        double b1 = -2 * A * (aMinusOne + aPlusOne * k);
        double b2 = A * (aPlusOne + aMinusOne * k - k2);
        double a0 = aPlusOne - aMinusOne * k + k2;
        double a1 = 2 * (aMinusOne - aPlusOne * k);
        double a2 = aPlusOne - aMinusOne * k - k2;

        setNormalizedCoefficients(b0, b1, b2, a0, a1, a2);
    }
    else
    {
        // When frequency = 0, the filter is just a gain, A^2.
        setNormalizedCoefficients(A * A, 0, 0,
                                  1, 0, 0);
    }
}

void Biquad::setPeakingParams(double frequency, double Q, double dbGain)
{
    // Clip frequencies to between 0 and 1, inclusive.
    frequency = std::max(0.0, std::min(frequency, 1.0));

    // Don't let Q go negative, which causes an unstable filter.
    Q = std::max(0.0, Q);

    double A = pow(10.0, dbGain / 40);

    if (frequency > 0 && frequency < 1)
    {
        if (Q > 0)
        {
            double w0 = static_cast<double>(LAB_PI) * frequency;
            double alpha = sin(w0) / (2 * Q);
            double k = cos(w0);

            double b0 = 1 + alpha * A;
            double b1 = -2 * k;
            double b2 = 1 - alpha * A;
            double a0 = 1 + alpha / A;
            double a1 = -2 * k;
            double a2 = 1 - alpha / A;

            setNormalizedCoefficients(b0, b1, b2, a0, a1, a2);
        }
        else
        {
            // When Q = 0, the above formulas have problems. If we look at
            // the z-transform, we can see that the limit as Q->0 is A^2, so
            // set the filter that way.
            setNormalizedCoefficients(A * A, 0, 0,
                                      1, 0, 0);
        }
    }
    else
    {
        // When frequency is 0 or 1, the z-transform is 1.
        setNormalizedCoefficients(1, 0, 0,
                                  1, 0, 0);
    }
}

void Biquad::setAllpassParams(double frequency, double Q)
{
    // Clip frequencies to between 0 and 1, inclusive.
    frequency = std::max(0.0, std::min(frequency, 1.0));

    // Don't let Q go negative, which causes an unstable filter.
    Q = std::max(0.0, Q);

    if (frequency > 0 && frequency < 1)
    {
        if (Q > 0)
        {
            double w0 = static_cast<double>(LAB_PI) * frequency;
            double alpha = sin(w0) / (2 * Q);
            double k = cos(w0);

            double b0 = 1 - alpha;
            double b1 = -2 * k;
            double b2 = 1 + alpha;
            double a0 = 1 + alpha;
            double a1 = -2 * k;
            double a2 = 1 - alpha;

            setNormalizedCoefficients(b0, b1, b2, a0, a1, a2);
        }
        else
        {
            // When Q = 0, the above formulas have problems. If we look at
            // the z-transform, we can see that the limit as Q->0 is -1, so
            // set the filter that way.
            setNormalizedCoefficients(-1, 0, 0,
                                      1, 0, 0);
        }
    }
    else
    {
        // When frequency is 0 or 1, the z-transform is 1.
        setNormalizedCoefficients(1, 0, 0,
                                  1, 0, 0);
    }
}

void Biquad::setNotchParams(double frequency, double Q)
{
    // Clip frequencies to between 0 and 1, inclusive.
    frequency = std::max(0.0, std::min(frequency, 1.0));

    // Don't let Q go negative, which causes an unstable filter.
    Q = std::max(0.0, Q);

    if (frequency > 0 && frequency < 1)
    {
        if (Q > 0)
        {
            double w0 = static_cast<double>(LAB_PI) * frequency;
            double alpha = sin(w0) / (2 * Q);
            double k = cos(w0);

            double b0 = 1;
            double b1 = -2 * k;
            double b2 = 1;
            double a0 = 1 + alpha;
            double a1 = -2 * k;
            double a2 = 1 - alpha;

            setNormalizedCoefficients(b0, b1, b2, a0, a1, a2);
        }
        else
        {
            // When Q = 0, the above formulas have problems. If we look at
            // the z-transform, we can see that the limit as Q->0 is 0, so
            // set the filter that way.
            setNormalizedCoefficients(0, 0, 0,
                                      1, 0, 0);
        }
    }
    else
    {
        // When frequency is 0 or 1, the z-transform is 1.
        setNormalizedCoefficients(1, 0, 0,
                                  1, 0, 0);
    }
}

void Biquad::setBandpassParams(double frequency, double Q)
{
    // No negative frequencies allowed.
    frequency = std::max(0.0, frequency);

    // Don't let Q go negative, which causes an unstable filter.
    Q = std::max(0.0, Q);

    if (frequency > 0 && frequency < 1)
    {
        double w0 = static_cast<double>(LAB_PI) * frequency;
        if (Q > 0)
        {
            double alpha = sin(w0) / (2 * Q);
            double k = cos(w0);

            double b0 = alpha;
            double b1 = 0;
            double b2 = -alpha;
            double a0 = 1 + alpha;
            double a1 = -2 * k;
            double a2 = 1 - alpha;

            setNormalizedCoefficients(b0, b1, b2, a0, a1, a2);
        }
        else
        {
            // When Q = 0, the above formulas have problems. If we look at
            // the z-transform, we can see that the limit as Q->0 is 1, so
            // set the filter that way.
            setNormalizedCoefficients(1, 0, 0,
                                      1, 0, 0);
        }
    }
    else
    {
        // When the cutoff is zero, the z-transform approaches 0, if Q
        // > 0. When both Q and cutoff are zero, the z-transform is
        // pretty much undefined. What should we do in this case?
        // For now, just make the filter 0. When the cutoff is 1, the
        // z-transform also approaches 0.
        setNormalizedCoefficients(0, 0, 0,
                                  1, 0, 0);
    }
}

void Biquad::setZeroPolePairs(const complex<double> & zero, const complex<double> & pole)
{
    double b0 = 1;
    double b1 = -2 * zero.real();

    double zeroMag = abs(zero);
    double b2 = zeroMag * zeroMag;

    double a1 = -2 * pole.real();

    double poleMag = abs(pole);
    double a2 = poleMag * poleMag;
    setNormalizedCoefficients(b0, b1, b2, 1, a1, a2);
}

void Biquad::setAllpassPole(const complex<double> & pole)
{
    complex<double> zero = complex<double>(1, 0) / pole;
    setZeroPolePairs(zero, pole);
}

void Biquad::getFrequencyResponse(size_t nFrequencies,
                                  const float * frequency,
                                  float * magResponse,
                                  float * phaseResponse)
{
    // Evaluate the Z-transform of the filter at given normalized
    // frequency from 0 to 1.  (1 corresponds to the Nyquist
    // frequency.)
    //
    // The z-transform of the filter is
    //
    // H(z) = (b0 + b1*z^(-1) + b2*z^(-2))/(1 + a1*z^(-1) + a2*z^(-2))
    //
    // Evaluate as
    //
    // b0 + (b1 + b2*z1)*z1
    // --------------------
    // 1 + (a1 + a2*z1)*z1
    //
    // with z1 = 1/z and z = exp(j*pi*frequency). Hence z1 = exp(-j*pi*frequency)

    // Make local copies of the coefficients as a micro-optimization.
    double b0 = m_b0;
    double b1 = m_b1;
    double b2 = m_b2;
    double a1 = m_a1;
    double a2 = m_a2;

    for (size_t k = 0; k < nFrequencies; ++k)
    {
        double omega = -static_cast<double>(LAB_PI) * frequency[k];
        complex<double> z = complex<double>(cos(omega), sin(omega));
        complex<double> numerator = b0 + (b1 + b2 * z) * z;
        complex<double> denominator = complex<double>(1, 0) + (a1 + a2 * z) * z;
        complex<double> response = numerator / denominator;
        magResponse[k] = static_cast<float>(abs(response));
        phaseResponse[k] = static_cast<float>(atan2(imag(response), real(response)));
    }
}

}  // namespace lab
