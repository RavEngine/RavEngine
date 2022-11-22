// SPDX-License-Identifier: BSD-2-Clause
// Copyright (C) 2015, The LabSound Authors. All rights reserved.

#ifndef RECORDER_NODE_H
#define RECORDER_NODE_H

#include "LabSound/core/AudioNode.h"
#include "LabSound/core/AudioContext.h"
#include <mutex>
#include <vector>

namespace lab
{

class RecorderNode : public AudioNode
{
    virtual double tailTime(ContextRenderLock & r) const override { return 0; }
    virtual double latencyTime(ContextRenderLock & r) const override { return 0; }

    bool m_recording{false};

    std::vector<std::vector<float>> m_data;  // non-interleaved
    mutable std::recursive_mutex m_mutex;

    float m_sampleRate;

public:

    // create a recorder
    RecorderNode(AudioContext & r, int channelCount = 2);

    // create a recorder with a specific configuration
    RecorderNode(AudioContext & r, const AudioStreamConfig & outConfig);
    virtual ~RecorderNode();

    static  const char* static_name() { return "Recorder"; }
    virtual const char* name() const override { return static_name(); }

    // AudioNode
    virtual void process(ContextRenderLock &, int bufferSize) override;
    virtual void reset(ContextRenderLock &) override;

    void startRecording() { m_recording = true; }
    void stopRecording() { m_recording = false; }

    float recordedLengthInSeconds() const;

    // create a bus from the recording; it can be used by other nodes
    // such as a SampledAudioNode.
    std::unique_ptr<AudioBus> createBusFromRecording(bool mixToMono);

    // returns true for success
    bool writeRecordingToWav(const std::string & filenameWithWavExtension, bool mixToMono);
};

}  // end namespace lab

#endif
