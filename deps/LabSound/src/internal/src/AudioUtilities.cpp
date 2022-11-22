// License: BSD 2 Clause
// Copyright (C) 2010, Google Inc. All rights reserved.
// Copyright (C) 2015+, The LabSound Authors. All rights reserved.

#include "internal/AudioUtilities.h"
#include "internal/Assertions.h"

#include "LabSound/core/Macros.h"

float lab::AudioUtilities::decibelsToLinear(float decibels)
{
    return std::pow(10.f, 0.05f * decibels);
}

float lab::AudioUtilities::linearToDecibels(float linear)
{
    // It's not possible to calculate decibels for a zero linear value since it would be -Inf.
    // -1000.0 dB represents a very tiny linear value in case we ever reach this case.
    if (!linear) return -1000.0;
    return (20.f * std::log10(linear));
}

double lab::AudioUtilities::discreteTimeConstantForSampleRate(double timeConstant, double sampleRate)
{
    return 1.0 - std::exp(-1.0 / (sampleRate * timeConstant));
}
