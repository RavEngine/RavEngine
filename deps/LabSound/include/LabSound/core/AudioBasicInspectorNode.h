// License: BSD 2 Clause
// Copyright (C) 2012, Intel Corporation. All rights reserved.
// Copyright (C) 2015+, The LabSound Authors. All rights reserved.

#ifndef AudioBasicInspectorNode_h
#define AudioBasicInspectorNode_h

#include "LabSound/core/AudioNode.h"

namespace lab
{

// AudioBasicInspectorNode is an AudioNode with one input and one output where
// the output might not necessarily connect to another node's input.
// If the output is not connected to any other node, then the
// AudioBasicInspectorNode's processIfNecessary() function can be called
// automatically by AudioContext before the end of each render quantum so that
// it can inspect the audio stream.

// In order that the automatic behavior is invoked, the AudioBasicInspectorNode
// must be registered with the AudioContext, via addAutomaticPullNode.

// NONFINAL NODE

class AudioBasicInspectorNode : public AudioNode
{
public:
    AudioBasicInspectorNode(AudioContext & ac, int outputChannelCount);
    virtual ~AudioBasicInspectorNode() = default;
};

}  // namespace lab

#endif  // AudioBasicInspectorNode_h
