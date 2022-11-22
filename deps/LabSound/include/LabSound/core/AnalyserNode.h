// License: BSD 2 Clause
// Copyright (C) 2010, Google Inc. All rights reserved.
// Copyright (C) 2015+, The LabSound Authors. All rights reserved.

#ifndef AnalyserNode_h
#define AnalyserNode_h

#include "LabSound/core/AudioBasicInspectorNode.h"

namespace lab
{
class AudioSetting;

// If the analyserNode is intended to run without it's output
// being connected to an AudioDestination, the AnalyserNode must be
// registered with the AudioContext via addAutomaticPullNode.

// params:
// settings: fftSize, minDecibels, maxDecibels, smoothingTimeConstant
//
class AnalyserNode : public AudioBasicInspectorNode
{
    void shared_construction(int fftSize);

    virtual double tailTime(ContextRenderLock & r) const override { return 0; }
    virtual double latencyTime(ContextRenderLock & r) const override { return 0; }

    struct Detail;
    Detail * _detail = nullptr;

public:
    AnalyserNode(AudioContext & ac);
    AnalyserNode(AudioContext & ac, int fftSize);
    virtual ~AnalyserNode();

    static const char* static_name() { return "Analyser"; }
    virtual const char* name() const override { return static_name(); }

    virtual void process(ContextRenderLock &, int bufferSize) override;
    virtual void reset(ContextRenderLock &) override;

    void setFftSize(ContextRenderLock &, int fftSize);
    size_t fftSize() const;

    // a value large enough to hold all the data return from get*FrequencyData
    size_t frequencyBinCount() const;

    void setMinDecibels(double k);
    double minDecibels() const;

    void setMaxDecibels(double k);
    double maxDecibels() const;

    void setSmoothingTimeConstant(double k);
    double smoothingTimeConstant() const;

    // frequency bins, reported in db
    void getFloatFrequencyData(std::vector<float> & array);

    // frequency bins, reported as a linear mapping of minDecibels to maxDecibles onto 0-255.
    // if resample is true, then the computed values will be linearly resampled
    void getByteFrequencyData(std::vector<uint8_t> & array, bool resample = false);
    void getFloatTimeDomainData(std::vector<float> & array);
    void getByteTimeDomainData(std::vector<uint8_t> & array);
};

}  // namespace lab

#endif  // AnalyserNode_h
