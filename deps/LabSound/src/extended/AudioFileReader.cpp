// SPDX-License-Identifier: BSD-2-Clause
// Copyright (C) 2015+, The LabSound Authors. All rights reserved.

#include "internal/Assertions.h"

#include "LabSound/core/AudioBus.h"
#include "LabSound/core/Macros.h"
#include "LabSound/core/Mixing.h"

#include "LabSound/extended/AudioFileReader.h"

#include "libnyquist/Decoders.h"

#include <cstring>
#include <mutex>

namespace detail
{
std::shared_ptr<lab::AudioBus> LoadInternal(nqr::AudioData * audioData, bool mixToMono)
{
    int numSamples = static_cast<int>(audioData->samples.size());
    if (!numSamples) return nullptr;

    int length = int(numSamples / audioData->channelCount);
    const int busChannelCount = mixToMono ? 1 : (audioData->channelCount);

    std::vector<float> planarSamples(numSamples);

    // Create AudioBus where we'll put the PCM audio data
    std::shared_ptr<lab::AudioBus> audioBus(new lab::AudioBus(busChannelCount, length));
    audioBus->setSampleRate((float) audioData->sampleRate);

    // Deinterleave stereo into LabSound/WebAudio planar channel layout
    nqr::DeinterleaveChannels(audioData->samples.data(), planarSamples.data(), length, audioData->channelCount, length);

    // Mix to mono if stereo -- easier to do in place instead of using libnyquist helper functions
    // because we've already deinterleaved
    if (audioData->channelCount == lab::Channels::Stereo && mixToMono)
    {
        float * destinationMono = audioBus->channel(0)->mutableData();
        float * leftSamples = planarSamples.data();
        float * rightSamples = planarSamples.data() + length;

        for (int i = 0; i < length; i++)
        {
            destinationMono[i] = 0.5f * (leftSamples[i] + rightSamples[i]);
        }
    }
    else
    {
        for (int i = 0; i < busChannelCount; ++i)
        {
            std::memcpy(audioBus->channel(i)->mutableData(), planarSamples.data() + (i * length), length * sizeof(float));
        }
    }

    delete audioData;

    return audioBus;
}
}

namespace lab
{

nqr::NyquistIO nyquist_io;
std::mutex g_fileIOMutex;

std::shared_ptr<AudioBus> MakeBusFromFile(const char * filePath, bool mixToMono)
{
    std::lock_guard<std::mutex> lock(g_fileIOMutex);
    nqr::AudioData * audioData = new nqr::AudioData();
    try
    {
        nyquist_io.Load(audioData, std::string(filePath));
    }
    catch (...)
    {
        // use empty pointer as load failure sentinel
        /// @TODO report loading error
        return {};
    }

    return detail::LoadInternal(audioData, mixToMono);
}

std::shared_ptr<AudioBus> MakeBusFromFile(const std::string & path, bool mixToMono)
{
    return MakeBusFromFile(path.c_str(), mixToMono);
}

std::shared_ptr<AudioBus> MakeBusFromFile(const char * filePath, bool mixToMono, float targetSampleRate)
{
    auto bus = MakeBusFromFile(filePath, false);
    if (bus)
    {
        return AudioBus::createBySampleRateConverting(bus.get(), mixToMono, targetSampleRate);
    }
    return bus;
}

std::shared_ptr<AudioBus> MakeBusFromFile(const std::string & path, bool mixToMono, float targetSampleRate)
{
    return MakeBusFromFile(path.c_str(), mixToMono, targetSampleRate);
}


std::shared_ptr<AudioBus> MakeBusFromMemory(const std::vector<uint8_t> & buffer, bool mixToMono)
{
    std::lock_guard<std::mutex> lock(g_fileIOMutex);
    nqr::AudioData * audioData = new nqr::AudioData();
    nyquist_io.Load(audioData, buffer);
    return detail::LoadInternal(audioData, mixToMono);
}

std::shared_ptr<AudioBus> MakeBusFromMemory(const std::vector<uint8_t> & buffer, const std::string & extension, bool mixToMono)
{
    std::lock_guard<std::mutex> lock(g_fileIOMutex);
    nqr::AudioData * audioData = new nqr::AudioData();
    nyquist_io.Load(audioData, extension, buffer);
    return detail::LoadInternal(audioData, mixToMono);
}

}  // end namespace lab
