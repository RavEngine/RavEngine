// License: BSD 2 Clause
// Copyright (C) 2011, Google Inc. All rights reserved.
// Copyright (C) 2015+, The LabSound Authors. All rights reserved.

#include "LabSound/core/NullDeviceNode.h"
#include "LabSound/core/AudioBus.h"
#include "LabSound/core/AudioContext.h"
#include "LabSound/core/AudioNodeInput.h"
#include "LabSound/extended/AudioContextLock.h"
#include "LabSound/extended/Logging.h"
#include "LabSound/extended/Registry.h"

#include "internal/Assertions.h"

#include <algorithm>

using namespace lab;

static const int offlineRenderSizeQuantum = AudioNode::ProcessingSizeInFrames;

NullDeviceNode::NullDeviceNode(AudioContext & ac, 
    const AudioStreamConfig & outputConfig, double lengthSeconds)
    : AudioNode(ac)
    , m_numChannels(outputConfig.desired_channels)
    , m_lengthSeconds(lengthSeconds)
    , m_context(&ac)
    , outConfig(outputConfig)
{
    addInput(std::unique_ptr<AudioNodeInput>(new AudioNodeInput(this)));

    m_renderBus = std::unique_ptr<AudioBus>(new AudioBus(m_numChannels, offlineRenderSizeQuantum));

    m_channelCount = m_numChannels;
    m_channelCountMode = ChannelCountMode::Explicit;
    m_channelInterpretation = ChannelInterpretation::Discrete;

    // We need to partially fill the the info struct here so that the context's graph thread
    // has enough initial info to make connections/disconnections appropriately
    info = {};
    info.sampling_rate = outConfig.desired_samplerate;
    info.epoch[0] = info.epoch[1] = std::chrono::high_resolution_clock::now();

    ContextGraphLock glock(&ac, "NullDeviceNode");
    AudioNode::setChannelCount(glock, m_numChannels);
    initialize();
}

NullDeviceNode::~NullDeviceNode()
{
    uninitialize();
}

void NullDeviceNode::initialize()
{
    if (isInitialized()) return;
    AudioNode::initialize();
}

void NullDeviceNode::uninitialize()
{
    if (!isInitialized()) return;

    if (m_renderThread.joinable())
    {
        m_renderThread.join();
    }

    AudioNode::uninitialize();
}

void NullDeviceNode::start()
{
    if (!m_startedRendering)
    {
        m_startedRendering = true;
        shouldExit = false;

        m_renderThread = std::thread(&NullDeviceNode::offlineRender, this);

        // @tofix - ability to update main thread from here. Currently blocks until complete
        if (m_renderThread.joinable())
        {
            m_renderThread.join();
        }

        LOG_TRACE("offline rendering completed");

        if (m_context->offlineRenderCompleteCallback)
        {
            m_context->offlineRenderCompleteCallback();
        }

        m_startedRendering = false;
    }
    else
    {
        LOG_WARN("NullDeviceNode rendering already started");
    }
}

void NullDeviceNode::stop()
{
    shouldExit = true;
}

bool NullDeviceNode::isRunning() const
{
    return m_startedRendering;
}

void NullDeviceNode::render(AudioBus * src, AudioBus * dst, int frames, const SamplingInfo & info)
{
    ProfileScope profile(graphTime);
    totalTime.zero();

    pull_graph(m_context, input(0).get(), src, dst, frames, info, nullptr);
    profile.finalize(); // ensure profile is not prematurely destructed
}

const SamplingInfo & NullDeviceNode::getSamplingInfo() const
{
    return info;
}

const AudioStreamConfig & NullDeviceNode::getOutputConfig() const
{
    return outConfig;
}

const AudioStreamConfig & NullDeviceNode::getInputConfig() const
{
    static AudioStreamConfig conf;
    return conf;
}

void NullDeviceNode::offlineRender()
{
    ASSERT(m_renderBus.get());
    if (!m_renderBus.get())
        return;

    bool isRenderBusAllocated = m_renderBus->length() >= offlineRenderSizeQuantum;
    ASSERT(isRenderBusAllocated);
    if (!isRenderBusAllocated)
        return;

    bool isAudioContextInitialized = m_context->isInitialized();
    ASSERT(isAudioContextInitialized);
    if (!isAudioContextInitialized)
        return;

    // Break up the desired length into smaller "render quantum" sized pieces.
    int framesToProcess = static_cast<int>((m_lengthSeconds * outConfig.desired_samplerate) / offlineRenderSizeQuantum);

    LOG_TRACE("offline rendering started");

    while (framesToProcess > 0 && !shouldExit)
    {
        m_context->update();
        render(0, m_renderBus.get(), offlineRenderSizeQuantum, info);

        // Update sampling info
        const int index = 1 - (info.current_sample_frame & 1);
        const uint64_t t = info.current_sample_frame & ~1;
        info.current_sample_frame = t + offlineRenderSizeQuantum + index;
        info.current_time = info.current_sample_frame / static_cast<double>(info.sampling_rate);
        info.epoch[index] += std::chrono::nanoseconds {static_cast<uint64_t>(
            1.e9 * (double) framesToProcess / (double) offlineRenderSizeQuantum)};

        framesToProcess--;
    }
}

void NullDeviceNode::offlineRenderFrames(size_t framesToProcess)
{
    ASSERT(framesToProcess % offlineRenderSizeQuantum == 0);

    ASSERT(m_renderBus.get());
    if (!m_renderBus.get())
        return;

    bool isRenderBusAllocated = m_renderBus->length() >= offlineRenderSizeQuantum;
    ASSERT(isRenderBusAllocated);
    if (!isRenderBusAllocated)
        return;

    bool isAudioContextInitialized = m_context->isInitialized();
    ASSERT(isAudioContextInitialized);
    if (!isAudioContextInitialized)
        return;

    //LOG_TRACE("offline render processing");

    while (framesToProcess > 0 && !shouldExit)
    {
        m_context->update();
        render(0, m_renderBus.get(), offlineRenderSizeQuantum, info);

        // Update sampling info
        const int index = 1 - (info.current_sample_frame & 1);
        const uint64_t t = info.current_sample_frame & ~1;
        info.current_sample_frame = t + offlineRenderSizeQuantum + index;
        info.current_time = info.current_sample_frame / static_cast<double>(info.sampling_rate);
        info.epoch[index] += std::chrono::nanoseconds {static_cast<uint64_t>(
            1.e9 * (double)framesToProcess / (double)offlineRenderSizeQuantum) };

        framesToProcess -= offlineRenderSizeQuantum;
    }
}
