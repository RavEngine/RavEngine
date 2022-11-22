// License: BSD 2 Clause
// Copyright (C) 2010, Google Inc. All rights reserved.
// Copyright (C) 2015+, The LabSound Authors. All rights reserved.

#ifndef DelayNode_h
#define DelayNode_h

#include "LabSound/core/AudioBasicProcessorNode.h"
#include "LabSound/core/Macros.h"

namespace lab
{

class AudioParam;
class DelayProcessor;

// TempoSync is commonly used by subclasses of DelayNode
enum TempoSync
{
    TS_32,
    TS_16T,
    TS_32D,
    TS_16,
    TS_8T,
    TS_16D,
    TS_8,
    TS_4T,
    TS_8D,
    TS_4,
    TS_2T,
    TS_4D,
    TS_2,
    TS_2D,
};

class DelayNode : public AudioBasicProcessorNode
{
    DelayProcessor * delayProcessor();

public:
    // default maximum delay of 100ms
    DelayNode(AudioContext & ac, double maxDelayTime = 2.0);

    virtual ~DelayNode() = default;

    static const char* static_name() { return "Delay"; }
    virtual const char* name() const override { return static_name(); }

    std::shared_ptr<AudioSetting> delayTime();
};

}  // namespace lab

#endif  // DelayNode_h
