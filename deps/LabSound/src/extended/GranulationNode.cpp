// SPDX-License-Identifier: BSD-2-Clause
// Copyright (C) 2015+, The LabSound Authors. All rights reserved.

#include "LabSound/extended/GranulationNode.h"
#include "LabSound/extended/Registry.h"

#include "LabSound/core/AudioBus.h"
#include "LabSound/core/AudioNodeOutput.h"
#include "LabSound/core/AudioSetting.h"
#include "LabSound/core/WindowFunctions.h"

#include "LabSound/extended/AudioContextLock.h"
#include "LabSound/extended/Util.h"

#include "internal/Assertions.h"
#include "internal/AudioUtilities.h"

using namespace lab;

GranulationNode::GranulationNode(AudioContext & ac) : AudioScheduledSourceNode(ac)
{
    // Sample that will be granulated
    grainSourceBus = std::make_shared<AudioSetting>("GrainSource", "GSRC", AudioSetting::Type::Bus);

    // Windowing function that will be applied as an evelope to each grain during playback
    windowFunc = std::make_shared<AudioSetting>("WindowFunction", "WINF", s_window_types);
    windowFunc->setEnumeration(static_cast<int>(WindowFunction::bartlett), true);

    // Total number of grains that will be allocated
    numGrains = std::make_shared<AudioParam>("NumGrains", "NGRN", 8.f, 1.f, 256.f, 1);

    // Duration of each grain in seconds (currently fixed but ideally will be configurable per-grain)
    grainDuration = std::make_shared<AudioParam>("GrainDuration", "GDUR", 0.1f, 0.01f, 0.5f);

    // The min/max positional offset (given in a normalized 0-1 range) from which a grain should
    // be sampled from grainSourceBus. These numbers are used to inform the range of a random
    // number generator. 
    grainPositionMin = std::make_shared<AudioParam>("PositionMin", "GMIN", 0.0f, 0.0f, 0.99f);
    grainPositionMax = std::make_shared<AudioParam>("PositionMax", "GMAX", 1.0f, 0.01f, 1.0f);

    // How fast the grain should play, given as a multiplier. Useful for pitch-shifting effects.
    grainPlaybackFreq = std::make_shared<AudioParam>("PlaybackFrequency", "FREQ", 1.0f, 0.01f, 12.0f);

    addOutput(std::unique_ptr<AudioNodeOutput>(new AudioNodeOutput(this, 1)));

    initialize();
}

GranulationNode::~GranulationNode()
{
    uninitialize();
}

void GranulationNode::reset(ContextRenderLock&)
{

}

bool GranulationNode::RenderGranulation(ContextRenderLock & r, AudioBus * out_bus, int destinationFrameOffset, int numberOfFrames)
{
    if (!r.context())
        return false;

    // Sanity check destinationFrameOffset, numberOfFrames.
    const int destinationLength = out_bus->length();

    bool isLengthGood = destinationLength <= 4096 && numberOfFrames <= 4096;
    ASSERT(isLengthGood);
    if (!isLengthGood)
        return false;

    bool isOffsetGood = destinationFrameOffset <= destinationLength && destinationFrameOffset + numberOfFrames <= destinationLength;
    ASSERT(isOffsetGood);
    if (!isOffsetGood)
        return false;

    if (!out_bus) return false;

    float * mono_destination = out_bus->channel(0)->mutableData();

    // @fixme - this vector
    std::vector<float> grain_sum_buffer(numberOfFrames, 0.f);

    {
        for (int i = 0; i < grain_pool.size(); ++i)
        {
            grain_pool[i].tick(grain_sum_buffer.data(), static_cast<int>(grain_sum_buffer.size()));
        }

        for (int f = 0; f < numberOfFrames; ++f)
        {
            grain_sum_buffer[f] /= static_cast<float>(grain_pool.size());
            mono_destination[f] = grain_sum_buffer[f]; // @todo - memcpy
        }

        // Optional grain culling when they're "done"
        // if (!grain_pool.empty())
        // {
        //     auto it = std::remove_if(std::begin(grain_pool), std::end(grain_pool), [this](const grain & g)
        //     {
        //         return g.in_use;
        //     });
        //     grain_pool.erase(it, std::end(grain_pool));
        // }
    }

    out_bus->clearSilentFlag();

    return true;
}

bool GranulationNode::setGrainSource(ContextRenderLock & r, std::shared_ptr<AudioBus> buffer)
{
    ASSERT(grainSourceBus);

    // Granulation Settings Sanity Check
    /// @fixme these values should be per sample, not per quantum
    /// -or- they should be settings if they don't vary per sample
    std::cout << "GranulationNode::WindowFunction    " << s_window_types[windowFunc->valueUint32()] << std::endl;
    std::cout << "GranulationNode::numGrains         " << numGrains->value() << std::endl;
    std::cout << "GranulationNode::grainDuration     " << grainDuration->value() << std::endl;
    std::cout << "GranulationNode::grainPositionMin  " << grainPositionMin->value() << std::endl;
    std::cout << "GranulationNode::grainPositionMax  " << grainPositionMax->value() << std::endl;
    std::cout << "GranulationNode::grainPlaybackFreq " << grainPlaybackFreq->value() << std::endl;

    grainSourceBus->setBus(buffer.get());
    output(0)->setNumberOfChannels(r, buffer ? buffer->numberOfChannels() : 0);

    // Compute useful values
    /// @fixme these values should be per sample, not per quantum
    /// -or- they should be settings if they don't vary per sample
    const float grain_duration_seconds = grainDuration->value();
    const uint64_t grain_duration_samples = static_cast<uint64_t>(grain_duration_seconds * r.context()->sampleRate());

    // Setup window/envelope
    std::vector<float> grain_window(grain_duration_samples, 1.f);
    lab::ApplyWindowFunctionInplace(static_cast<WindowFunction>(windowFunc->valueUint32()), grain_window.data(), static_cast<int>(grain_window.size()));

    window_bus.reset(new lab::AudioBus(1, static_cast<int>(grain_duration_samples)));
    window_bus->setSampleRate(r.context()->sampleRate());

    float * windowData = window_bus->channel(0)->mutableData();
    for (uint32_t sample = 0; sample < grain_window.size(); ++sample)
    {
        windowData[sample] = grain_window[sample];
    }

    // Setup grains
    UniformRandomGenerator rnd;
    auto sample_to_granulate = grainSourceBus->valueBus();
    /// @fixme should be setting
    for (int i = 0; i < numGrains->value(); ++i)
    {
        /// @fixme these values should be per sample, not per quantum
        /// -or- they should be settings if they don't vary per sample
        const float random_pos_offset = rnd.random_float(grainPositionMin->value(), grainPositionMax->value());
        grain_pool.emplace_back(grain(sample_to_granulate, window_bus, r.context()->sampleRate(), random_pos_offset, grain_duration_seconds, grainPlaybackFreq->value()));
    }

    return true;
}

void GranulationNode::process(ContextRenderLock& r, int bufferSize)
{
    AudioBus * outputBus = output(0)->bus(r);

    if (!isInitialized() || !outputBus->numberOfChannels()) 
    {
        outputBus->zero();
        return;
    }

    int quantumFrameOffset = _scheduler._renderOffset;
    int bufferFramesToProcess = _scheduler._renderLength;

    if (!bufferFramesToProcess)
    {
        outputBus->zero();
        return;
    }

    if (!RenderGranulation(r, outputBus, quantumFrameOffset, bufferFramesToProcess))
    {
        outputBus->zero();
        return;
    }
    outputBus->clearSilentFlag();
}
    
bool GranulationNode::propagatesSilence(ContextRenderLock & r) const
{
    return !isPlayingOrScheduled() || hasFinished();
}
