// SPDX-License-Identifier: BSD-3-Clause
// Copyright (C) 2020, The LabSound Authors. All rights reserved.

#include "AudioDevice_RtAudio.h"

#include "internal/Assertions.h"
#include "internal/VectorMath.h"

#include "LabSound/core/AudioDevice.h"
#include "LabSound/core/AudioHardwareDeviceNode.h"
#include "LabSound/core/AudioNode.h"

#include "LabSound/extended/Logging.h"

#include "RtAudio.h"

namespace lab
{

////////////////////////////////////////////////////
//   Platform/backend specific static functions   //
////////////////////////////////////////////////////

std::vector<AudioDeviceInfo> AudioDevice::MakeAudioDeviceList()
{
    std::vector<std::string> rt_audio_apis{
        "unspecified",
        "linux_alsa",
        "linux_pulse",
        "linux_oss",
        "unix_jack",
        "macos_coreaudio",
        "windows_wasapi",
        "windows_asio",
        "windows_directsound",
        "rtaudio_dummy"};

    RtAudio rt;
    if (rt.getDeviceCount() <= 0) throw std::runtime_error("no rtaudio devices available!");

    const auto api_enum = rt.getCurrentApi();
    LOG_INFO("using rtaudio api %s", rt_audio_apis[static_cast<int>(api_enum)].c_str());

    auto to_flt_vec = [](const std::vector<unsigned int> & vec) {
        std::vector<float> result;
        for (auto & i : vec) result.push_back(static_cast<float>(i));
        return result;
    };

    std::vector<AudioDeviceInfo> devices;
    for (uint32_t i = 0; i < rt.getDeviceCount(); ++i)
    {
        RtAudio::DeviceInfo info = rt.getDeviceInfo(i);

        if (info.probed)
        {
            AudioDeviceInfo lab_device_info;
            lab_device_info.index = i;
            lab_device_info.identifier = info.name;
            lab_device_info.num_output_channels = info.outputChannels;
            lab_device_info.num_input_channels = info.inputChannels;
            lab_device_info.supported_samplerates = to_flt_vec(info.sampleRates);
            lab_device_info.nominal_samplerate = static_cast<float>(info.preferredSampleRate);
            lab_device_info.is_default_output = info.isDefaultOutput;
            lab_device_info.is_default_input = info.isDefaultInput;

            devices.push_back(lab_device_info);
        }
        else
        {
            LOG_ERROR("probing failed %s", info.name.c_str());
        }
    }

    return devices;
}

AudioDeviceIndex AudioDevice::GetDefaultOutputAudioDeviceIndex() noexcept
{
    RtAudio rt;
    if (rt.getDeviceCount() <= 0) return {0, false};
    return {rt.getDefaultOutputDevice(), true};
}

AudioDeviceIndex AudioDevice::GetDefaultInputAudioDeviceIndex() noexcept
{
    RtAudio rt;
    if (rt.getDeviceCount() <= 0) return {0, false};
    return {rt.getDefaultInputDevice(), true};
}

AudioDevice * AudioDevice::MakePlatformSpecificDevice(
    AudioDeviceRenderCallback & callback,
    const AudioStreamConfig & outputConfig, 
    const AudioStreamConfig & inputConfig)
{
    return new AudioDevice_RtAudio(callback, outputConfig, inputConfig);
}


/////////////////////////////
//   AudioDevice_RtAudio   //
/////////////////////////////

const float kLowThreshold = -1.0f;
const float kHighThreshold = 1.0f;
const bool kInterleaved = false;

AudioDevice_RtAudio::AudioDevice_RtAudio(
    AudioDeviceRenderCallback & callback,
    const AudioStreamConfig & _outputConfig, const AudioStreamConfig & _inputConfig)
    : _callback(callback)
    , outputConfig(_outputConfig)
    , inputConfig(_inputConfig)
{
    if (rtaudio_ctx.getDeviceCount() < 1)
    {
        LOG_ERROR("no audio devices available");
    }

    rtaudio_ctx.showWarnings(true);

    // Translate AudioStreamConfig into RTAudio-native data structures

    RtAudio::StreamParameters outputParams;
    outputParams.deviceId = outputConfig.device_index;
    outputParams.nChannels = outputConfig.desired_channels;
    LOG_INFO("using output device idx: %i", outputConfig.device_index);
    if (outputConfig.device_index >= 0) LOG_INFO("using output device name: %s", rtaudio_ctx.getDeviceInfo(outputParams.deviceId).name.c_str());

    RtAudio::StreamParameters inputParams;
    inputParams.deviceId = inputConfig.device_index;
    inputParams.nChannels = inputConfig.desired_channels;
    LOG_INFO("using input device idx: %i", inputConfig.device_index);
    if (inputConfig.device_index >= 0) LOG_INFO("using input device name: %s", rtaudio_ctx.getDeviceInfo(inputParams.deviceId).name.c_str());

    authoritativeDeviceSampleRateAtRuntime = outputConfig.desired_samplerate;

    if (inputConfig.desired_channels > 0)
    {
        auto inDeviceInfo = rtaudio_ctx.getDeviceInfo(inputParams.deviceId);
        if (inDeviceInfo.probed && inDeviceInfo.inputChannels > 0)
        {
            // ensure that the number of input channels buffered does not exceed the number available.
            inputParams.nChannels = (inDeviceInfo.inputChannels < inputConfig.desired_channels) ? inDeviceInfo.inputChannels : inputConfig.desired_channels;
            inputConfig.desired_channels = inputParams.nChannels;
            LOG_INFO("[AudioDevice_RtAudio] adjusting number of input channels: %i ", inputParams.nChannels);
        }
    }

    RtAudio::StreamOptions options;
    // RTAUDIO_MINIMIZE_LATENCY tells RtAudio to use the hardware's minimum buffer size
    // which is not desirable as the minimum way be too small, and a non-power of 2.
    //options.flags = RTAUDIO_MINIMIZE_LATENCY;
    if (!kInterleaved) options.flags |= RTAUDIO_NONINTERLEAVED;

    // Note! RtAudio has a hard limit on a power of two buffer size, non-power of two sizes will result in
    // heap corruption, for example, when dac.stopStream() is invoked.
    uint32_t bufferFrames = AudioNode::ProcessingSizeInFrames;

    samplingInfo.epoch[0] = samplingInfo.epoch[1] = std::chrono::high_resolution_clock::now();

    try
    {
        rtaudio_ctx.openStream(&outputParams, (inputParams.nChannels > 0) ? &inputParams : nullptr, RTAUDIO_FLOAT32,
                               static_cast<unsigned int>(authoritativeDeviceSampleRateAtRuntime), &bufferFrames, &rt_audio_callback, this, &options);
    }
    catch (const RtAudioError & e)
    {
        LOG_ERROR(e.getMessage().c_str());
    }
}

AudioDevice_RtAudio::~AudioDevice_RtAudio()
{
    if (rtaudio_ctx.isStreamOpen())
    {
        rtaudio_ctx.closeStream();
    }
}

void AudioDevice_RtAudio::start()
{
    ASSERT(authoritativeDeviceSampleRateAtRuntime != 0.f);  // something went very wrong
    try
    {
        rtaudio_ctx.startStream();
    }
    catch (const RtAudioError & e)
    {
        LOG_ERROR(e.getMessage().c_str());
    }
}

void AudioDevice_RtAudio::stop()
{
    try
    {
        rtaudio_ctx.stopStream();
    }
    catch (const RtAudioError & e)
    {
        LOG_ERROR(e.getMessage().c_str());
    }
}

bool AudioDevice_RtAudio::isRunning() const
{
    try
    {
        return rtaudio_ctx.isStreamRunning();
    }
    catch (const RtAudioError & e)
    {
        LOG_ERROR(e.getMessage().c_str());
        return false;
    }
}


// Pulls on our provider to get rendered audio stream.
void AudioDevice_RtAudio::render(int numberOfFrames, void * outputBuffer, void * inputBuffer)
{
    float * fltOutputBuffer = reinterpret_cast<float *>(outputBuffer);
    float * fltInputBuffer = reinterpret_cast<float *>(inputBuffer);

    if (outputConfig.desired_channels)
    {
        if (!_renderBus || _renderBus->length() < numberOfFrames)
        {
            _renderBus.reset(new AudioBus(outputConfig.desired_channels, numberOfFrames, true));
            _renderBus->setSampleRate(authoritativeDeviceSampleRateAtRuntime);
        }
    }

    if (inputConfig.desired_channels)
    {
        if (!_inputBus || _inputBus->length() < numberOfFrames)
        {
            _inputBus.reset(new AudioBus(inputConfig.desired_channels, numberOfFrames, true));
            _inputBus->setSampleRate(authoritativeDeviceSampleRateAtRuntime);
        }
    }

    // copy the input buffer
    if (inputConfig.desired_channels)
    {
        if (kInterleaved)
        {
            for (uint32_t i = 0; i < inputConfig.desired_channels; ++i)
            {
                AudioChannel * channel = _inputBus->channel(i);
                float * src = &fltInputBuffer[i];
                VectorMath::vclip(src, 1, &kLowThreshold, &kHighThreshold, channel->mutableData(), inputConfig.desired_channels, numberOfFrames);
            }
        }
        else
        {
            for (uint32_t i = 0; i < inputConfig.desired_channels; ++i)
            {
                AudioChannel * channel = _inputBus->channel(i);
                float * src = &fltInputBuffer[i * numberOfFrames];
                VectorMath::vclip(src, 1, &kLowThreshold, &kHighThreshold, channel->mutableData(), 1, numberOfFrames);
            }
        }
    }

    // Update sampling info
    const int32_t index = 1 - (samplingInfo.current_sample_frame & 1);
    const uint64_t t = samplingInfo.current_sample_frame & ~1;
    samplingInfo.sampling_rate = authoritativeDeviceSampleRateAtRuntime;
    samplingInfo.current_sample_frame = t + numberOfFrames + index;
    samplingInfo.current_time = samplingInfo.current_sample_frame / static_cast<double>(samplingInfo.sampling_rate);
    samplingInfo.epoch[index] = std::chrono::high_resolution_clock::now();

    // Pull on the graph
    _callback.render(_inputBus.get(), _renderBus.get(), numberOfFrames, samplingInfo);

    // Then deliver the rendered audio back to rtaudio, ready for the next callback
    if (outputConfig.desired_channels)
    {
        // Clamp values at 0db (i.e., [-1.0, 1.0]) and also copy result to the DAC output buffer
        if (kInterleaved)
        {
            for (uint32_t i = 0; i < outputConfig.desired_channels; ++i)
            {
                AudioChannel * channel = _renderBus->channel(i);
                float * dst = &fltOutputBuffer[i];
                VectorMath::vclip(channel->data(), 1, &kLowThreshold, &kHighThreshold, dst, outputConfig.desired_channels, numberOfFrames);
            }
        }
        else
        {
            for (uint32_t i = 0; i < outputConfig.desired_channels; ++i)
            {
                AudioChannel * channel = _renderBus->channel(i);
                float * dst = &fltOutputBuffer[i * numberOfFrames];
                VectorMath::vclip(channel->data(), 1, &kLowThreshold, &kHighThreshold, dst, 1, numberOfFrames);
            }
        }
    }
}

int rt_audio_callback(void * outputBuffer, void * inputBuffer, unsigned int nBufferFrames, double streamTime, RtAudioStreamStatus status, void * userData)
{
    AudioDevice_RtAudio * self = reinterpret_cast<AudioDevice_RtAudio *>(userData);
    float * fltOutputBuffer = reinterpret_cast<float *>(outputBuffer);
    memset(fltOutputBuffer, 0, nBufferFrames * self->outputConfig.desired_channels * sizeof(float));
    self->render(nBufferFrames, fltOutputBuffer, inputBuffer);
    return 0;
}

}  // namespace lab
