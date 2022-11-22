// License: BSD 2 Clause
// Copyright (C) 2012, Google Inc. All rights reserved.
// Copyright (C) 2015+, The LabSound Authors. All rights reserved.

#ifndef AudioSummingJunction_h
#define AudioSummingJunction_h

#include <memory>
#include <vector>

namespace lab
{

class AudioContext;
class AudioNodeOutput;
class AudioBus;
class ContextGraphLock;
class ContextRenderLock;

// An AudioSummingJunction represents a point where zero, one, or more AudioNodeOutputs connect.

class AudioSummingJunction
{

public:
    explicit AudioSummingJunction();
    virtual ~AudioSummingJunction();

    // This must be called whenever we modify m_outputs.
    void changedOutputs(ContextGraphLock &);

    // This copies m_outputs to m_renderingOutputs. See comments for these lists below.
    void updateRenderingState(ContextRenderLock & r);

    // will count expired pointers
    int numberOfConnections() const { return static_cast<int>(m_connectedOutputs.size()); }

    // Rendering code accesses its version of the current connections here.
    int numberOfRenderingConnections(ContextRenderLock &) const;
    std::shared_ptr<AudioNodeOutput> renderingOutput(ContextRenderLock &, int i)
    {
        return i < m_renderingOutputs.size() ? m_renderingOutputs[i].lock() : nullptr;
    }

    const std::shared_ptr<AudioNodeOutput> renderingOutput(ContextRenderLock &, int i) const
    {
        return i < m_renderingOutputs.size() ? m_renderingOutputs[i].lock() : nullptr;
    }

    bool isConnected() const { return numberOfConnections() > 0; }

    void junctionConnectOutput(std::shared_ptr<AudioNodeOutput>);
    void junctionDisconnectOutput(std::shared_ptr<AudioNodeOutput>);
    void junctionDisconnectAllOutputs();
    void setDirty() { m_renderingStateNeedUpdating = true; }

    static void handleDirtyAudioSummingJunctions(ContextRenderLock & r);

    bool isConnected(std::shared_ptr<AudioNodeOutput> o) const;

protected:
    // m_outputs contains the AudioNodeOutputs representing current connections.
    // The rendering code should never use this directly, but instead uses m_renderingOutputs.
    std::vector<std::weak_ptr<AudioNodeOutput>> m_connectedOutputs;

    // m_renderingOutputs is a copy of m_connectedOutputs which will never be modified during the graph rendering on the audio thread.
    // This is the list which is used by the rendering code.
    // Whenever m_outputs is modified, the context is told so it can later update m_renderingOutputs from m_outputs at a safe time.
    // Most of the time, m_renderingOutputs is identical to m_outputs.
    std::vector<std::weak_ptr<AudioNodeOutput>> m_renderingOutputs;

    // m_renderingStateNeedUpdating indicates outputs were changed
    bool m_renderingStateNeedUpdating;
};

}  // namespace lab

#endif  // AudioSummingJunction_h
