// License: BSD 2 Clause
// Copyright (C) 2012, Google Inc. All rights reserved.
// Copyright (C) 2015+, The LabSound Authors. All rights reserved.

#ifndef audio_hardware_input_node
#define audio_hardware_input_node

#include "LabSound/core/AudioNode.h"
#include "LabSound/core/AudioSourceProvider.h"

namespace lab
{
class AudioContext;

class AudioHardwareInputNode : public AudioNode
{
    // As an audio source, we will never propagate silence.
    virtual bool propagatesSilence(ContextRenderLock & r) const override { return false; }
    AudioSourceProvider * m_audioSourceProvider;
    int m_sourceNumberOfChannels{0};

public:
    AudioHardwareInputNode(AudioContext & ac, AudioSourceProvider * provider_from_context);
    virtual ~AudioHardwareInputNode();

    static const char* static_name() { return "AudioHardwareDevice"; }
    virtual const char* name() const override { return static_name(); }

    virtual double tailTime(ContextRenderLock & r) const override { return 0; }
    virtual double latencyTime(ContextRenderLock & r) const override { return 0; }

    // AudioNode interface
    virtual void process(ContextRenderLock &, int bufferSize) override;
    virtual void reset(ContextRenderLock &) override;
};

}  // namespace lab

#endif
