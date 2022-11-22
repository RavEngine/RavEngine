// License: BSD 3 Clause
// Copyright (C) 2010, Google Inc. All rights reserved.
// Copyright (C) 2015+, The LabSound Authors. All rights reserved.

#ifndef AudioBus_h
#define AudioBus_h

#include "LabSound/core/AudioChannel.h"
#include "LabSound/core/Mixing.h"
#include <iostream>
#include <vector>

namespace lab
{

using lab::Channel;
using lab::ChannelInterpretation;

// An AudioBus represents a collection of one or more AudioChannels.
// The data layout is "planar" as opposed to "interleaved".
// An AudioBus with one channel is mono, an AudioBus with two channels is stereo, etc.
class AudioBus
{
    AudioBus(const AudioBus &);  // noncopyable

public:
    // Can define non-standard layouts here:
    enum
    {
        LayoutCanonical = 0
    };

    // allocate indicates whether or not to initially have the AudioChannels created with managed storage.
    // Normal usage is to pass true here, in which case the AudioChannels will memory-manage their own storage.
    // If allocate is false then setChannelMemory() has to be called later on for each channel before the AudioBus is useable...
    AudioBus(int numberOfChannels, int length, bool allocate = true);

    // Tells the given channel to use an externally allocated buffer.
    void setChannelMemory(int channelIndex, float * storage, int length);

    // Channels
    int numberOfChannels() const { return static_cast<int>(m_channels.size()); }

    // Use this when looping over channels
    AudioChannel * channel(int channel) { return m_channels[channel].get(); }

    const AudioChannel * channel(int channel) const
    {
        return const_cast<AudioBus *>(this)->m_channels[channel].get();
    }

    // use this when accessing channels semantically
    AudioChannel * channelByType(Channel type);
    const AudioChannel * channelByType(Channel type) const;

    // Number of sample-frames
    int length() const { return m_length; }

    // resizeSmaller() can only be called with a new length <= the current length.
    // The data stored in the bus will remain undisturbed.
    void resizeSmaller(int newLength);

    // Sample-rate : 0.0 if unknown or "don't care"
    float sampleRate() const { return m_sampleRate; }
    void setSampleRate(float sampleRate) { m_sampleRate = sampleRate; }

    // Zeroes all channels.
    void zero();

    // Clears the silent flag on all channels.
    void clearSilentFlag();

    // Returns true if the silent bit is set on all channels.
    bool isSilent() const;

    // Returns true if the channel count and frame-size match.
    bool topologyMatches(const AudioBus & sourceBus) const;

    // Scales all samples by the same amount.
    void scale(float scale);

    void reset() { m_isFirstTime = true; }  // for de-zippering

    // Assuming sourceBus has the same topology, copies sample data from each channel of sourceBus to our corresponding channel.
    void copyFrom(const AudioBus & sourceBus, ChannelInterpretation = ChannelInterpretation::Speakers);

    // Sums the sourceBus into our bus with unity gain.
    // Our own internal gain m_busGain is ignored.
    void sumFrom(const AudioBus & sourceBus, ChannelInterpretation = ChannelInterpretation::Speakers);

    // Copy each channel from sourceBus into our corresponding channel.
    // We scale by targetGain (and our own internal gain m_busGain), performing "de-zippering" to smoothly change from *lastMixGain to (targetGain*m_busGain).
    // The caller is responsible for setting up lastMixGain to point to storage which is unique for every "stream" which will be applied to this bus.
    // This represents the dezippering memory.
    void copyWithGainFrom(const AudioBus & sourceBus, float * lastMixGain, float targetGain);

    // Copies the sourceBus by scaling with sample-accurate gain values.
    void copyWithSampleAccurateGainValuesFrom(const AudioBus & sourceBus, float * gainValues, int numberOfGainValues);

    // Returns maximum absolute value across all channels (useful for normalization).
    float maxAbsValue() const;

    // Makes maximum absolute value == 1.0 (if possible).
    void normalize();

    bool isFirstTime() { return m_isFirstTime; }

    // Static Functions

    // Creates a new buffer from a range in the source buffer.
    // 0 may be returned if the range does not fit in the sourceBuffer
    static std::unique_ptr<AudioBus> createBufferFromRange(const AudioBus * sourceBus, int startFrame, int endFrame);

    // Creates a new AudioBus by sample-rate converting sourceBus to the newSampleRate.
    // setSampleRate() must have been previously called on sourceBus.
    static std::unique_ptr<AudioBus> createBySampleRateConverting(const AudioBus * sourceBus, bool mixToMono, float newSampleRate);

    // Creates a new AudioBus by mixing all the channels down to mono.
    // If sourceBus is already mono, then the returned AudioBus will simply be a copy.
    static std::unique_ptr<AudioBus> createByMixingToMono(const AudioBus * sourceBus);

    // Creates a new AudioBus by cloning an existing one
    static std::unique_ptr<AudioBus> createByCloning(const AudioBus * sourceBus);

protected:

    AudioBus() = default;

    void speakersCopyFrom(const AudioBus &);
    void discreteCopyFrom(const AudioBus &);
    void speakersSumFrom(const AudioBus &);
    void discreteSumFrom(const AudioBus &);
    void speakersSumFrom5_1_ToMono(const AudioBus &);
    void speakersSumFrom7_1_ToMono(const AudioBus &);

    std::unique_ptr<AudioFloatArray> m_dezipperGainValues;
    std::vector<std::unique_ptr<AudioChannel>> m_channels;

    bool m_isFirstTime = true;
    float m_sampleRate = 0.0f;
    float m_busGain = 1.0f;
    int m_layout = LayoutCanonical;
    int m_length = 0;
};

}  // lab

#endif  // AudioBus_h
