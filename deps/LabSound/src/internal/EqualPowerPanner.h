// License: BSD 2 Clause
// Copyright (C) 2010, Google Inc. All rights reserved.
// Copyright (C) 2015+, The LabSound Authors. All rights reserved.

#ifndef EqualPowerPanner_h
#define EqualPowerPanner_h

#include "internal/Panner.h"

namespace lab
{

// Common type of stereo panner as found in normal audio mixing equipment.
class EqualPowerPanner : public Panner
{

public:
    EqualPowerPanner(const float sampleRate);

    virtual void pan(ContextRenderLock &, double azimuth, double elevation,
                     const AudioBus * inputBus, AudioBus * outputBuf,
                     int framesToProcess) override;

    virtual void reset() override { m_isFirstRender = true; }

    virtual double tailTime(ContextRenderLock & r) const override { return 0; }
    virtual double latencyTime(ContextRenderLock & r) const override { return 0; }

private:
    // For smoothing / de-zippering
    bool m_isFirstRender = true;
    double m_smoothingConstant;

    double m_gainL = 0.0;
    double m_gainR = 0.0;
};

}  // namespace lab

#endif
