// SPDX-License-Identifier: BSD-3-Clause
// Copyright (C) 2011, Google Inc. All rights reserved.
// Copyright (C) 2015+, The LabSound Authors. All rights reserved.

#include "LabSound/core/AudioHardwareDeviceNode.h"
#include "LabSound/core/AudioBus.h"
#include "LabSound/core/AudioContext.h"
#include "LabSound/core/AudioDevice.h"
#include "LabSound/core/AudioNodeInput.h"
#include "LabSound/core/AudioNodeOutput.h"
#include "LabSound/core/AudioSourceProvider.h"

#include "LabSound/extended/AudioContextLock.h"
#include "LabSound/extended/Registry.h"

#include "internal/Assertions.h"
#include "internal/AudioUtilities.h"
#include "internal/DenormalDisabler.h"

#include <chrono>

using namespace lab;

/////////////////////////////////
//   AudioHardwareDeviceNode   //
/////////////////////////////////

AudioHardwareDeviceNode::AudioHardwareDeviceNode(AudioContext & context,
                                                 const AudioStreamConfig & outputConfig,
                                                 const AudioStreamConfig & inputConfig)
    : AudioNode(context)
    , m_context(&context)
    , outConfig(outputConfig)
    , inConfig(inputConfig)
{
    // Ensure that input and output sample rates match
    if (inputConfig.device_index != -1 && outputConfig.device_index != -1)
    {
        ASSERT(outputConfig.desired_samplerate == inputConfig.desired_samplerate);
    }

    m_platformAudioDevice = std::unique_ptr<AudioDevice>(AudioDevice::MakePlatformSpecificDevice(*this, outputConfig, inputConfig));

    LOG_INFO("MakePlatformSpecificDevice() \n"
             "\t* Sample Rate:     %f \n"
             "\t* Input Channels:  %i \n"
             "\t* Output Channels: %i   ",
        outputConfig.desired_samplerate, inputConfig.desired_channels, outputConfig.desired_channels);

    if (inputConfig.device_index != -1)
    {
        m_audioHardwareInput = new AudioHardwareInput(inputConfig.desired_channels);
    }

    // This is the "final node" in the chain. It will pull on all others from this input.
    addInput(std::make_unique<AudioNodeInput>(this));

    // Node-specific default mixing rules.
    //m_channelCount = outputConfig.desired_channels;
    m_channelCountMode = ChannelCountMode::Explicit;
    m_channelInterpretation = ChannelInterpretation::Speakers;

    ContextGraphLock glock(&context, "AudioHardwareDeviceNode");
    AudioNode::setChannelCount(glock, outputConfig.desired_channels);

    // Info is provided by the backend every frame, but some nodes need to be constructed
    // with a valid sample rate before the first frame so we make our best guess here
    last_info = {};
    last_info.sampling_rate = outputConfig.desired_samplerate;

    initialize();
}

AudioHardwareDeviceNode::~AudioHardwareDeviceNode()
{
    uninitialize();
}

void AudioHardwareDeviceNode::initialize()
{
    if (!isInitialized())
        AudioNode::initialize();
}

void AudioHardwareDeviceNode::uninitialize()
{
    if (!isInitialized()) return;
    m_platformAudioDevice->stop();
    AudioNode::uninitialize();
    m_platformAudioDevice.reset();
}

void AudioHardwareDeviceNode::start()
{
    initialize();  // presumably called by the context

    if (isInitialized())
    {
        m_platformAudioDevice->start();
    }
}

void AudioHardwareDeviceNode::backendReinitialize()
{
    m_platformAudioDevice->backendReinitialize();
}

void AudioHardwareDeviceNode::stop()
{
    m_platformAudioDevice->stop();
}

bool AudioHardwareDeviceNode::isRunning() const
{
    return m_platformAudioDevice->isRunning();
}

const SamplingInfo & AudioHardwareDeviceNode::getSamplingInfo() const
{
    return last_info;
}

const AudioStreamConfig & AudioHardwareDeviceNode::getOutputConfig() const
{
    return outConfig;
}

const AudioStreamConfig & AudioHardwareDeviceNode::getInputConfig() const
{
    return inConfig;
}

void AudioHardwareDeviceNode::reset(ContextRenderLock &)
{
    m_platformAudioDevice->stop();
    m_platformAudioDevice->start();
};

void AudioHardwareDeviceNode::render(AudioBus * src, AudioBus * dst, int frames, const SamplingInfo & info)
{
    ProfileScope selfProfile(totalTime);
    ProfileScope profile(graphTime);
    pull_graph(m_context, input(0).get(), src, dst, frames, info, m_audioHardwareInput);
    last_info = info;
    profile.finalize(); // ensure profile is not prematurely destructed
    selfProfile.finalize();
}

AudioSourceProvider * AudioHardwareDeviceNode::AudioHardwareInputProvider()
{
    return static_cast<AudioSourceProvider *>(m_audioHardwareInput);
}
