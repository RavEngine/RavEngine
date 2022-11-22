// License: BSD 3 Clause
// Copyright (C) 2009, Google Inc. All rights reserved.
// Copyright (C) 2015+, The LabSound Authors. All rights reserved.

#ifndef Panner_h
#define Panner_h

#include <LabSound/core/AudioNode.h>
#include <memory>

namespace lab
{

class AudioBus;
class ContextRenderLock;

// clang-format off
class Panner
{
public:
    Panner(const float sampleRate, PanningMode mode) : m_sampleRate(sampleRate) , m_panningModel(mode) {}
    virtual ~Panner() {};
    PanningMode panningModel() const { return m_panningModel; }
    virtual void pan(ContextRenderLock & r, double azimuth, double elevation, const AudioBus * inputBus, AudioBus * outputBus, int framesToProcess) = 0;
    virtual void reset() = 0;
    virtual double tailTime(ContextRenderLock & r) const = 0;
    virtual double latencyTime(ContextRenderLock & r) const = 0;
protected:
    float m_sampleRate {0.f};
    PanningMode m_panningModel;
};
// clang-format on

} 

#endif
