// License: BSD 2 Clause
// Copyright (C) 2010, Google Inc. All rights reserved.
// Copyright (C) 2015+, The LabSound Authors. All rights reserved.

#ifndef AudioBasicProcessorNode_h
#define AudioBasicProcessorNode_h

#include "LabSound/core/AudioNode.h"
#include "LabSound/core/AudioProcessor.h"
#include "LabSound/extended/AudioContextLock.h"
#include <memory>

namespace lab
{

class AudioBus;
class AudioNodeInput;

// AudioBasicProcessorNode is an AudioNode with one input and one output where
//  the input and output have the same number of channels.
class AudioBasicProcessorNode : public AudioNode
{
public:
    AudioBasicProcessorNode(AudioContext &);
    virtual ~AudioBasicProcessorNode() = default;

    // AudioNode
    virtual void process(ContextRenderLock &, int bufferSize) override;
    virtual void reset(ContextRenderLock &) override;
    virtual void initialize() override;
    virtual void uninitialize() override;

    // Returns the number of channels for both the input and the output.
    int numberOfChannels();

protected:
    virtual double tailTime(ContextRenderLock & r) const override;
    virtual double latencyTime(ContextRenderLock & r) const override;

    AudioProcessor * processor();
    AudioProcessor * processor() const;

    std::unique_ptr<AudioProcessor> m_processor;
};

}  // namespace lab

#endif  // AudioBasicProcessorNode_h
