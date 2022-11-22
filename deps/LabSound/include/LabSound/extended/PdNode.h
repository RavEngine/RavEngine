// Copyright (c) 2003-2013 Nick Porcino, All rights reserved.
// License is MIT: http://opensource.org/licenses/MIT

// PureDataNode wraps an instance of pure-data as a signal processing node

#pragma once

#ifdef PD

#include "LabSound/core/AudioBasicProcessorNode.h"

namespace pd
{
class PdBase;
class PdMidiReceiver;
class PdReceiver;
}

namespace lab
{

class PureDataNode : public AudioBasicProcessorNode
{

public:
    PureDataNode(AudioContext *, float sampleRate);
    virtual ~PureDataNode();

    static const char* static_name() { return "PureData"; }
    virtual const char* name() const override { return static_name(); }

    pd::PdBase & pd() const;

    virtual double tailTime(ContextRenderLock & r) const override { return 0; }
    virtual double latencyTime(ContextRenderLock & r) const override { return 0; }

private:
    class PureDataNodeInternal;
    PureDataNodeInternal * data;
};

}  // end namespace lab

#endif
