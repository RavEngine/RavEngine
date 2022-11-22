// License: BSD 3 Clause
// Copyright (C) 2010, Google Inc. All rights reserved.
// Copyright (C) 2015+, The LabSound Authors. All rights reserved.

#include "internal/AudioDSPKernelProcessor.h"
#include "internal/Assertions.h"
#include "internal/AudioDSPKernel.h"
#include <algorithm>

namespace lab
{

// setNumberOfChannels() may later be called if the object is not yet in an "initialized" state.
AudioDSPKernelProcessor::AudioDSPKernelProcessor()
    : AudioProcessor()
    , m_hasJustReset(true)
{
}

void AudioDSPKernelProcessor::initialize()
{
    if (isInitialized())
        return;

    m_initialized = true;
    m_hasJustReset = true;
}

void AudioDSPKernelProcessor::uninitialize()
{
    if (!isInitialized())
        return;

    m_kernels.clear();
    m_initialized = false;
}

void AudioDSPKernelProcessor::process(ContextRenderLock & r, const AudioBus * source, AudioBus * destination, int framesToProcess)
{
    ASSERT(source && destination);
    if (!source || !destination)
        return;

    if (!isInitialized())
    {
        destination->zero();
        return;
    }

    size_t src_channels = source->numberOfChannels();
    size_t dst_channels = destination->numberOfChannels();
    size_t kernel_count = std::min(src_channels, dst_channels);

    if (m_kernels.size() != dst_channels)
    {
        m_kernels.clear();
        for (int i = 0; i < dst_channels; ++i)
            m_kernels.push_back(std::unique_ptr<AudioDSPKernel>(createKernel()));
    }

    bool channelCountMatches = source->numberOfChannels() == destination->numberOfChannels() && source->numberOfChannels() == m_kernels.size();
    ASSERT(channelCountMatches);
    if (!channelCountMatches)
        return;

    for (unsigned i = 0; i < m_kernels.size(); ++i)
        m_kernels[i]->process(r, source->channel(i)->data(),
                              destination->channel(i)->mutableData(), framesToProcess);
}

// Resets filter state
void AudioDSPKernelProcessor::reset()
{
    if (!isInitialized())
        return;

    // Forces snap to parameter values - first time.
    // Any processing depending on this value must set it to false at the appropriate time.
    m_hasJustReset = true;

    for (unsigned i = 0; i < m_kernels.size(); ++i)
        m_kernels[i]->reset();
}

double AudioDSPKernelProcessor::tailTime(ContextRenderLock & r) const
{
    // It is expected that all the kernels have the same tailTime.
    return !m_kernels.empty() ? (*m_kernels.begin())->tailTime(r) : 0;
}

double AudioDSPKernelProcessor::latencyTime(ContextRenderLock & r) const
{
    // It is expected that all the kernels have the same latencyTime.
    return !m_kernels.empty() ? (*m_kernels.begin())->latencyTime(r) : 0;
}

}  // namespace lab
