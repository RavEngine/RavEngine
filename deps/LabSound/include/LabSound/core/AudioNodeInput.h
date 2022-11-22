// License: BSD 2 Clause
// Copyright (C) 2010, Google Inc. All rights reserved.
// Copyright (C) 2015+, The LabSound Authors. All rights reserved.

#ifndef AudioNodeInput_h
#define AudioNodeInput_h

#include "LabSound/core/AudioNode.h"
#include "LabSound/core/AudioSummingJunction.h"

#include <set>

namespace lab
{

class AudioNode;
class AudioNodeOutput;
class AudioBus;

// An AudioNodeInput represents an input to an AudioNode and can be connected from one or more AudioNodeOutputs.
// In the case of multiple connections, the input will act as a unity-gain summing junction, mixing all the outputs.
// The number of channels of the input's bus is the maximum of the number of channels of all its connections.
class AudioNodeInput : public AudioSummingJunction
{
    AudioNode * m_destinationNode;
    std::unique_ptr<AudioBus> m_internalSummingBus;

public:
    explicit AudioNodeInput(AudioNode * audioNode, int processingSizeInFrames = AudioNode::ProcessingSizeInFrames);
    virtual ~AudioNodeInput();

    // Can be called from any thread.
    AudioNode * destinationNode() const { return m_destinationNode; }

    // Must be called with the context's graph lock. Static because a shared pointer to this is required
    static void connect(ContextGraphLock &, std::shared_ptr<AudioNodeInput> fromInput, std::shared_ptr<AudioNodeOutput> toOutput);
    static void disconnect(ContextGraphLock &, std::shared_ptr<AudioNodeInput> fromInput, std::shared_ptr<AudioNodeOutput> toOutput);
    static void disconnectAll(ContextGraphLock&, std::shared_ptr<AudioNodeInput> fromInput);

    // pull() processes all of the AudioNodes connected to this NodeInput.
    // In the case of multiple connections, the result is summed onto the internal summing bus.
    // In the single connection case, it allows in-place processing where possible using inPlaceBus.
    // It returns the bus which it rendered into, returning inPlaceBus if in-place processing was performed.
    AudioBus * pull(ContextRenderLock &, AudioBus * inPlaceBus, int bufferSize);

    // bus() contains the rendered audio after pull() has been called for each time quantum.
    AudioBus * bus(ContextRenderLock &);

    // updateInternalBus() updates m_internalSummingBus appropriately for the number of channels.
    // This must be called when we own the context's graph lock in the audio thread at the very start or end of the render quantum.
    void updateInternalBus(ContextRenderLock &);

    // The number of channels of the connection with the largest number of channels.
    // Only valid during render quantum because it is dependent on the active bus
    int numberOfChannels(ContextRenderLock &) const;
};

}  // namespace lab

#endif  // AudioNodeInput_h
