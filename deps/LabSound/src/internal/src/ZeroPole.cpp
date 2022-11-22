// License: BSD 3 Clause
// Copyright (C) 2011, Google Inc. All rights reserved.
// Copyright (C) 2015+, The LabSound Authors. All rights reserved.

#include "internal/ZeroPole.h"
#include "internal/DenormalDisabler.h"

namespace lab
{

void ZeroPole::process(const float * source, float * destination, int bufferSize, int offset, int count)
{
    float zero = m_zero;
    float pole = m_pole;

    // Gain compensation to make 0dB @ 0Hz
    const float k1 = 1 / (1 - zero);
    const float k2 = 1 - pole;

    // Member variables to locals.
    float lastX = m_lastX;
    float lastY = m_lastY;

    source += offset;
    destination += offset;

    while (count--)
    {
        float input = *source++;

        // Zero
        float output1 = k1 * (input - zero * lastX);
        lastX = input;

        // Pole
        float output2 = k2 * output1 + pole * lastY;
        lastY = output2;

        *destination++ = output2;
    }

    // Locals to member variables. Flush denormals here so we don't
    // slow down the inner loop above.
    m_lastX = DenormalDisabler::flushDenormalFloatToZero(lastX);
    m_lastY = DenormalDisabler::flushDenormalFloatToZero(lastY);
}

}  // lab
