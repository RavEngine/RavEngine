// License: BSD 2 Clause
// Copyright (C) 2014, The Chromium Authors. All rights reserved.
// Copyright (C) 2015+, The LabSound Authors. All rights reserved.

#ifndef StereoPannerNode_h
#define StereoPannerNode_h

#include "LabSound/core/AudioArray.h"
#include "LabSound/core/AudioNode.h"
#include "LabSound/core/AudioParam.h"
#include "LabSound/core/Macros.h"

#include <memory>
#include <string>

namespace lab
{

class Spatializer;

// StereoPannerNode is an AudioNode with one input and one output. It is
// specifically designed for equal-power stereo panning.
// irrespective of the number of input channels, the output channel count is always two.

class StereoPannerNode : public AudioNode
{
public:
    StereoPannerNode() = delete;
    explicit StereoPannerNode(AudioContext& ac);
    virtual ~StereoPannerNode();

    static const char* static_name() { return "StereoPanner"; }
    virtual const char* name() const override { return static_name(); }

    std::shared_ptr<AudioParam> pan() { return m_pan; }

private:
    // AudioNode
    virtual void process(ContextRenderLock &, int bufferSize) override;
    virtual void reset(ContextRenderLock &) override;

    virtual void initialize() override;
    virtual void uninitialize() override;

    virtual double tailTime(ContextRenderLock & r) const override { return 0; }
    virtual double latencyTime(ContextRenderLock & r) const override { return 0; }

    virtual void conformChannelCounts() override {};

    std::shared_ptr<AudioParam> m_pan;

    std::unique_ptr<Spatializer> m_stereoPanner;
    std::unique_ptr<AudioFloatArray> m_sampleAccuratePanValues;
};

}  // namespace lab

#endif  // StereoPannerNode_h
