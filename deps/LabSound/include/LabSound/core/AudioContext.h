// License: BSD 2 Clause
// Copyright (C) 2015+, The LabSound Authors. All rights reserved.

#pragma once

#ifndef lab_audio_context_h
#define lab_audio_context_h

#include "LabSound/core/AudioDevice.h"
#include "LabSound/core/AudioHardwareDeviceNode.h"
#include "LabSound/core/AudioScheduledSourceNode.h"

#include <atomic>
#include <condition_variable>
#include <functional>
#include <memory>
#include <mutex>
#include <set>
#include <string>
#include <thread>
#include <vector>

namespace lab
{

class AudioHardwareDeviceNode;
class AudioListener;
class AudioNode;
class AudioScheduledSourceNode;
class AudioHardwareInputNode;
class AudioNodeInput;
class AudioNodeOutput;
class ContextGraphLock;
class ContextRenderLock;

class AudioContext
{
    friend class ContextGraphLock;
    friend class ContextRenderLock;

public:

    // AudioNodes should not retain the AudioContext. The majority of
    // methods that take AudioContext as a parameter do so via reference.
    // If methods from the context, such as currentTime() are
    // required in methods that do not take AudioContext as a parameter, 
    // they should instead retain a weak pointer to the 
    // AudioNodeInterface from the AudioContext they were instantiated from. 
    // Locking the AudioNodeInterface
    // pointer is a way to check if the AudioContext is still instantiated.
    // AudioContextInterface contains data that is safe for any thread, or
    // audio processing callback to read, even if the audio context is in
    // the process of stopping or destruction.
    class AudioContextInterface
    {
        friend class AudioContext;


        int _id = 0;
        AudioContext * _ac = nullptr;
        double _currentTime = 0;

    public:
        AudioContextInterface(AudioContext * ac, int id)
            : _id(id)
            , _ac(ac)
        {
        }

        ~AudioContextInterface() { }

        // the contextId of two AudioNodeInterfaces can be compared
        // to discover if they refer to the same context.
        int contextId() const { return _id; }
        double currentTime() const { return _currentTime; }
    };

    std::weak_ptr<AudioContextInterface> audioContextInterface() { return m_audioContextInterface; }


    // Debugging/Sanity Checking
    std::string m_graphLocker;
    std::string m_renderLocker;

    explicit AudioContext(bool isOffline, bool autoDispatchEvents = true);
    ~AudioContext();

    bool isInitialized() const;

    // External users shouldn't use this; it should be called by
    // LabSound::MakeRealtimeAudioContext(lab::Channels::Stereo)
    // It *is* harmless to call it though, it's just not necessary.
    void lazyInitialize();

    void setDeviceNode(std::shared_ptr<AudioNode> device);
    std::shared_ptr<AudioNode> device();

    bool isOfflineContext() const;

    // The current time, measured at the start of the render quantum currently
    // being processed. Most useful in a Node's process routine.
    double currentTime() const;

    // The current epoch (total count of previously processed render quanta)
    // Useful in recurrent graphs to discover if a node has already done its
    // processing for the present quanta.
    uint64_t currentSampleFrame() const;

    // The current time, accurate versus the audio clock. Whereas currentTime
    // advances discretely, once per render quanta, predictedCurrentTime
    // is the sum of currentTime, and the amount of realworld time that has
    // elapsed since the start of the current render quanta. This is useful on
    // the main thread of an application in order to precisely synchronize
    // expected audio events and other systems.
    double predictedCurrentTime() const;

    float sampleRate() const;

    std::shared_ptr<AudioListener> listener();

    // Called at the start of each render quantum.
    void handlePreRenderTasks(ContextRenderLock &);

    // Called at the end of each render quantum.
    void handlePostRenderTasks(ContextRenderLock &);

    // AudioContext can pull node(s) at the end of each render quantum even
    // when they are not connected to any downstream nodes. These two methods
    // are called by the nodes who want to add/remove themselves into/from the
    // automatic pull lists.
    void addAutomaticPullNode(std::shared_ptr<AudioNode>);
    void removeAutomaticPullNode(std::shared_ptr<AudioNode>);

    // Called right before handlePostRenderTasks() to handle nodes which need to
    // be pulled even when they are not connected to anything.
    // Only an AudioHardwareDeviceNode should call this.
    void processAutomaticPullNodes(ContextRenderLock &, int framesToProcess);

    void connect(std::shared_ptr<AudioNode> destination, std::shared_ptr<AudioNode> source, int destIdx = 0, int srcIdx = 0);
    void disconnect(std::shared_ptr<AudioNode> destination, std::shared_ptr<AudioNode> source, int destIdx = 0, int srcidx = 0);
    bool isConnected(std::shared_ptr<AudioNode> destination, std::shared_ptr<AudioNode> source);

    // completely disconnect the node from the graph
    void disconnect(std::shared_ptr<AudioNode> node, int destIdx = 0);

    // connect a parameter to receive the indexed output of a node
    void connectParam(std::shared_ptr<AudioParam> param, std::shared_ptr<AudioNode> driver, int index);

    // connect destinationNode's named parameter input to driver's indexed output
    void connectParam(std::shared_ptr<AudioNode> destinationNode, char const*const parameterName, std::shared_ptr<AudioNode> driver, int index);

    // disconnect a parameter from the indexed output of a node
    void disconnectParam(std::shared_ptr<AudioParam> param, std::shared_ptr<AudioNode> driver, int index);

    // connecting and disconnecting busses and parameters occurs asynchronously.
    // synchronizeConnections will block until there are no pending connections,
    // or until the timeout occurs.
    void synchronizeConnections(int timeOut_ms = 1000);

    void startOfflineRendering();
    std::function<void()> offlineRenderCompleteCallback;

    // suspend the progression of time in the audio context, any queued samples will play
    void suspend();

    // if the context was suspended, resume the progression of time and processing in the audio context
    void resume();
    
    // event dispatching will be called automatically, depending on constructor
    // argument. If not automatically dispatching, it is the user's responsibility
    // to call dispatchEvents often enough to satisfy the user's needs.
    void enqueueEvent(std::function<void()> &);
    void dispatchEvents();

    void appendDebugBuffer(AudioBus* bus, int channel, int count);
    void flushDebugBuffer(char const* const wavFilePath);

private:
    // @TODO migrate most of the internal datastructures such as
    // PendingConnection into Internals as there's no need to expose these at all.
    struct Internals;
    std::unique_ptr<Internals> m_internal;

    std::shared_ptr<AudioContextInterface> m_audioContextInterface;

    std::mutex m_graphLock;
    std::mutex m_renderLock;
    std::mutex m_updateMutex;
    std::condition_variable cv;

    // -1 means run forever, 0 means stop, n > 0 means run this many times
    // n > 0 will decrement to zero each time update runs.
    std::atomic<int> updateThreadShouldRun{-1};
    std::thread graphUpdateThread;
    float graphKeepAlive{0.f};
    float lastGraphUpdateTime{0.f};

    bool m_isInitialized = false;
    bool m_isAudioThreadFinished = false;
    bool m_isOfflineContext = false;
    bool m_automaticPullNodesNeedUpdating = false;  // indicates m_automaticPullNodes was modified.

    friend class NullDeviceNode; // needs to be able to call update()
    void update();
    void updateAutomaticPullNodes();
    void uninitialize();

    AudioDeviceRenderCallback * device_callback{nullptr};
    std::shared_ptr<AudioNode> m_device;

    std::shared_ptr<AudioListener> m_listener;

    std::set<std::shared_ptr<AudioNode>> m_automaticPullNodes;  // queue for added pull nodes
    std::vector<std::shared_ptr<AudioNode>> m_renderingAutomaticPullNodes;  // vector of known pull nodes
};

}  // End namespace lab

#endif  // lab_audio_context_h
