// License: BSD 2 Clause
// Copyright (C) 2011, Google Inc. All rights reserved.
// Copyright (C) 2015+, The LabSound Authors. All rights reserved.

#pragma once

#ifndef lab_null_device_node_h
#define lab_null_device_node_h

#include "LabSound/core/AudioDevice.h"
#include "LabSound/core/AudioHardwareDeviceNode.h"

#include <atomic>
#include <memory>
#include <thread>

namespace lab
{
class AudioBus;
class AudioContext;

class NullDeviceNode final : public AudioNode, public AudioDeviceRenderCallback
{
    std::unique_ptr<AudioBus> m_renderBus;
    std::thread m_renderThread;

    std::atomic<bool> shouldExit{false};

    bool m_startedRendering{false};
    uint32_t m_numChannels;
    double m_lengthSeconds;

    AudioContext * m_context;

    AudioStreamConfig outConfig;
    SamplingInfo info;

public:
    NullDeviceNode(AudioContext & context, 
        const AudioStreamConfig & outputConfig, const double lengthSeconds);
    virtual ~NullDeviceNode();

    static const char* static_name() { return "NulLDevice"; }
    virtual const char* name() const override { return static_name(); }

    // AudioNode Interface
    virtual void initialize() override;
    virtual void uninitialize() override;

    virtual double tailTime(ContextRenderLock & r) const override { return 0; }
    virtual double latencyTime(ContextRenderLock & r) const override { return 0; }

    virtual void process(ContextRenderLock &, int bufferSize) override {}  // NullDeviceNode is pulled by its own internal thread so this is never called
    virtual void reset(ContextRenderLock &) override{};  // @fixme

    // AudioDeviceRenderCallback interface
    virtual void render(AudioBus * src, AudioBus * dst, int frames, const SamplingInfo & info) override final;
    virtual void start() override final;
    virtual void stop() override final;
    virtual bool isRunning() const override final;
    virtual const SamplingInfo & getSamplingInfo() const override final;
    virtual const AudioStreamConfig & getOutputConfig() const override final;
    virtual const AudioStreamConfig & getInputConfig() const override final;

    void offlineRender();
    void offlineRenderFrames(size_t framesToProcess);
};

}  // namespace lab

#endif  // end lab_null_device_node_h
