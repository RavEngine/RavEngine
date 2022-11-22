// License: BSD 3 Clause
// Copyright (C) 2010, Google Inc. All rights reserved.
// Copyright (C) 2015+, The LabSound Authors. All rights reserved.

#pragma once

#ifndef lab_audiodevice_h
#define lab_audiodevice_h

#include <chrono>
#include <stddef.h>
#include <stdint.h>
#include <string>
#include <vector>

namespace lab
{
class AudioBus;
class AudioContext;
class AudioNodeInput;
class AudioHardwareInput;
class AudioDevice;

/////////////////////////////////////////////
//   Audio Device Configuration Settings   //
/////////////////////////////////////////////

struct AudioDeviceInfo
{
    int32_t index{-1};
    std::string identifier;
    uint32_t num_output_channels{0};
    uint32_t num_input_channels{0};
    std::vector<float> supported_samplerates;
    float nominal_samplerate{0};
    bool is_default_output{false};
    bool is_default_input{false};
};

// Input and Output
struct AudioStreamConfig
{
    int32_t device_index{-1};
    uint32_t desired_channels{0};
    float desired_samplerate{0};
};

// low bit of current_sample_frame indexes time point 0 or 1
// (so that time and epoch are written atomically, after the alternative epoch has been filled in)
struct SamplingInfo
{
    uint64_t current_sample_frame{0};
    double current_time{0.0};
    float sampling_rate{0.0};
    std::chrono::high_resolution_clock::time_point epoch[2];
};

// This is a free function to consolidate and implement the required functionality to take buffers
// from the hardware (both input and output) and begin pulling the graph until fully rendered per quanta.
// This was formerly a function found on the removed `AudioDestinationNode` class (removed
// to de-complicate some annoying inheritance chains).
//
// `pull_graph(...)` will need to be called by a single AudioNode per-context. For instance,
// the `AudioHardwareDeviceNode` or the `NullDeviceNode`.
void pull_graph(AudioContext * ctx, AudioNodeInput * required_inlet, AudioBus * src, AudioBus * dst, int frames,
                const SamplingInfo & info, AudioHardwareInput * optional_hardware_input = nullptr);

///////////////////////////////////
//   AudioDeviceRenderCallback   //
///////////////////////////////////

// `render()` is called periodically to get the next render quantum of audio into destinationBus.
// Optional audio input is given in src (if not nullptr). This structure also keeps
// track of timing information with respect to the callback.
class AudioDeviceRenderCallback
{
    AudioStreamConfig outputConfig;
    AudioStreamConfig inputConfig;
    friend class AudioDevice;

public:
    virtual ~AudioDeviceRenderCallback() {}
    virtual void render(AudioBus * src, AudioBus * dst, int frames, const SamplingInfo & info) = 0;
    virtual void start() = 0;
    virtual void stop() = 0;
    virtual bool isRunning() const = 0;

    virtual const SamplingInfo & getSamplingInfo() const = 0;
    virtual const AudioStreamConfig & getOutputConfig() const = 0;
    virtual const AudioStreamConfig & getInputConfig() const = 0;
};

/////////////////////
//   AudioDevice   //
/////////////////////

// The audio hardware periodically calls the AudioDeviceRenderCallback `render()` method asking it to
// render/output the next render quantum of audio. It optionally will pass in local/live audio
// input when it calls `render()`.

struct AudioDeviceIndex
{
    uint32_t index;
    bool valid;
};

class AudioDevice
{
public:
    static std::vector<AudioDeviceInfo> MakeAudioDeviceList();
    static AudioDeviceIndex GetDefaultOutputAudioDeviceIndex() noexcept;
    static AudioDeviceIndex GetDefaultInputAudioDeviceIndex() noexcept;
    static AudioDevice * MakePlatformSpecificDevice(AudioDeviceRenderCallback &, 
        const AudioStreamConfig & outputConfig, const AudioStreamConfig & inputConfig);

    virtual ~AudioDevice() {}
    virtual void start() = 0;
    virtual void stop() = 0;
    virtual bool isRunning() const = 0;
    virtual void backendReinitialize() {}
};

}  // lab

#endif  // lab_audiodevice_h
