// License: BSD 3 Clause
// Copyright (C) 2010, Google Inc. All rights reserved.
// Copyright (C) 2015+, The LabSound Authors. All rights reserved.

#include "LabSound/core/AudioBus.h"
#include "internal/Assertions.h"
#include "internal/DenormalDisabler.h"
#include "internal/VectorMath.h"
#include "libsamplerate/include/samplerate.h"

#include <algorithm>
#include <assert.h>
#include <math.h>

namespace lab
{

using namespace VectorMath;

const unsigned MaxBusChannels = 32;

AudioBus::AudioBus(int numberOfChannels, int length, bool allocate)
    : m_length(length)
{
    ASSERT(numberOfChannels <= MaxBusChannels);
    if (numberOfChannels > MaxBusChannels)
        return;

    for (int i = 0; i < numberOfChannels; ++i)
    {
        AudioChannel * newChannel;
        if (allocate)
            newChannel = new AudioChannel(length);
        else
            newChannel = new AudioChannel(nullptr, length);

        m_channels.emplace_back(std::unique_ptr<AudioChannel>(newChannel));
    }
}

void AudioBus::setChannelMemory(int channelIndex, float * storage, int length)
{
    if (channelIndex < m_channels.size())
    {
        channel(channelIndex)->set(storage, length);
        m_length = length;  // @fixme - verify that this length matches all the other channel lengths
    }
}

void AudioBus::resizeSmaller(int newLength)
{
    ASSERT(newLength <= m_length);
    if (newLength <= m_length)
    {
        m_length = newLength;
    }

    for (int i = 0; i < m_channels.size(); ++i)
    {
        m_channels[i]->resizeSmaller(newLength);
    }
}

void AudioBus::zero()
{
    for (int i = 0; i < m_channels.size(); ++i)
    {
        m_channels[i]->zero();
    }
}

AudioChannel * AudioBus::channelByType(Channel channelType)
{
    // For now we only support canonical channel layouts...
    if (m_layout != LayoutCanonical) return 0;

    switch (numberOfChannels())
    {
        case Channels::Mono:
            if (channelType == Channel::Mono || channelType == Channel::Left || channelType == Channel::First)
            {
                return channel(static_cast<int>(Channel::Left));
            }
            return 0;

        case Channels::Stereo:
            switch (channelType)
            {
                case Channel::Left:
                case Channel::Right:
                    return channel(static_cast<int>(channelType));
                default:
                    return 0;
            }

        case Channels::Quad:
            switch (channelType)
            {
                case Channel::Left:
                case Channel::Right:
                case Channel::SurroundLeft:
                case Channel::SurroundRight:
                    return channel(static_cast<int>(channelType));
                default:
                    return 0;
            }

        case Channels::Surround_5_0:
            switch (channelType)
            {
                case Channel::Left:
                case Channel::Right:
                case Channel::Center:
                case Channel::SurroundLeft:
                case Channel::SurroundRight:
                    return channel(static_cast<int>(channelType));
                default:
                    return 0;
            }

        case Channels::Surround_5_1:
            switch (channelType)
            {
                case Channel::Left:
                case Channel::Right:
                case Channel::Center:
                case Channel::LFE:
                case Channel::SurroundLeft:
                case Channel::SurroundRight:
                    return channel(static_cast<int>(channelType));
                default:
                    return 0;
            }

        case Channels::Surround_7_1:
            switch (channelType)
            {
                case Channel::Left:
                case Channel::Right:
                case Channel::Center:
                case Channel::LFE:
                case Channel::SurroundLeft:
                case Channel::SurroundRight:
                case Channel::BackLeft:
                case Channel::BackRight:
                    return channel(static_cast<int>(channelType));
                default:
                    return 0;
            }
    }

    ASSERT_NOT_REACHED();
    return 0;
}

const AudioChannel * AudioBus::channelByType(Channel type) const
{
    return const_cast<AudioBus *>(this)->channelByType(type);
}

// Returns true if the channel count and frame-size match.
bool AudioBus::topologyMatches(const AudioBus & bus) const
{
    if (numberOfChannels() != bus.numberOfChannels())
        return false;  // channel mismatch

    // Make sure source bus has enough frames.
    if (length() > bus.length())
        return false;  // frame-size mismatch

    return true;
}

std::unique_ptr<AudioBus> AudioBus::createBufferFromRange(const AudioBus * sourceBuffer, int startFrame, int endFrame)
{
    int numberOfSourceFrames = sourceBuffer->length();
    int numberOfChannels = sourceBuffer->numberOfChannels();

    // Sanity checking
    bool isRangeSafe = startFrame < endFrame && endFrame <= numberOfSourceFrames;
    ASSERT(isRangeSafe);

    if (!isRangeSafe)
    {
        return nullptr;
    }

    int rangeLength = endFrame - startFrame;

    std::unique_ptr<AudioBus> audioBus(new AudioBus(numberOfChannels, rangeLength));
    audioBus->setSampleRate(sourceBuffer->sampleRate());

    for (int i = 0; i < numberOfChannels; ++i)
    {
        audioBus->channel(i)->copyFromRange(sourceBuffer->channel(i), startFrame, endFrame);
    }

    return audioBus;
}

std::unique_ptr<AudioBus> AudioBus::createByCloning(const AudioBus * sourceBus)
{
    const int numberOfSourceFrames = sourceBus->length();
    const int numberOfChannels = sourceBus->numberOfChannels();

    std::unique_ptr<AudioBus> clonedBus(new AudioBus(numberOfChannels, numberOfSourceFrames));
    clonedBus->setSampleRate(sourceBus->sampleRate());

    for (int i = 0; i < numberOfChannels; ++i)
    {
        clonedBus->channel(i)->copyFromRange(sourceBus->channel(i), 0, numberOfSourceFrames);
    }

    return clonedBus;
}

float AudioBus::maxAbsValue() const
{
    float max = 0.0f;
    for (int i = 0; i < numberOfChannels(); ++i)
    {
        max = std::max(max, channel(i)->maxAbsValue());
    }

    return max;
}

void AudioBus::normalize()
{
    float max = maxAbsValue();
    if (max) scale(1.0f / max);
}

void AudioBus::scale(float scale)
{
    for (int i = 0; i < numberOfChannels(); ++i)
    {
        channel(i)->scale(scale);
    }
}

// Just copies the samples from the source bus to this one.
// This is just a simple copy if the number of channels match, otherwise a mixup or mixdown is done.
// For now, we just support mixup from mono -> stereo and mixdown from stereo -> mono.
void AudioBus::copyFrom(const AudioBus & sourceBus, ChannelInterpretation channelInterpretation)
{
    if (&sourceBus == this) return;

    int numberOfSourceChannels = sourceBus.numberOfChannels();
    int numberOfDestinationChannels = numberOfChannels();

    if (numberOfDestinationChannels == numberOfSourceChannels)
    {
        for (int i = 0; i < numberOfSourceChannels; ++i)
        {
            channel(i)->copyFrom(sourceBus.channel(i));
        }
    }
    else
    {
        switch (channelInterpretation)
        {
            case ChannelInterpretation::Speakers: speakersCopyFrom(sourceBus); break;
            case ChannelInterpretation::Discrete: discreteCopyFrom(sourceBus); break;
            default:
                ASSERT_NOT_REACHED();
        }
    }
}

void AudioBus::sumFrom(const AudioBus & sourceBus, ChannelInterpretation channelInterpretation)
{
    if (&sourceBus == this) return;

    int numberOfSourceChannels = sourceBus.numberOfChannels();
    int numberOfDestinationChannels = numberOfChannels();

    if (numberOfDestinationChannels == numberOfSourceChannels)
    {
        for (int i = 0; i < numberOfSourceChannels; ++i)
        {
            channel(i)->sumFrom(sourceBus.channel(i));
        }
    }
    else
    {
        switch (channelInterpretation)
        {
            case ChannelInterpretation::Speakers: speakersSumFrom(sourceBus); break;
            case ChannelInterpretation::Discrete: discreteSumFrom(sourceBus);  break;
            default:
                ASSERT_NOT_REACHED();
        }
    }
}

void AudioBus::speakersCopyFrom(const AudioBus & sourceBus)
{
    // FIXME: Implement down mixing 5.1 to stereo.
    // https://bugs.webkit.org/show_bug.cgi?id=79192

    const int numberOfSourceChannels = sourceBus.numberOfChannels();
    const int numberOfDestinationChannels = numberOfChannels();

    if (numberOfDestinationChannels == Channels::Stereo && numberOfSourceChannels == Channels::Mono)
    {
        // Handle mono -> stereo case (for now simply copy mono channel into both left and right)
        // FIXME: Really we should apply an equal-power scaling factor here, since we're effectively panning center...
        const AudioChannel * sourceChannel = sourceBus.channel(0);
        channelByType(Channel::Left)->copyFrom(sourceChannel);
        channelByType(Channel::Right)->copyFrom(sourceChannel);
    }
    else if (numberOfDestinationChannels == Channels::Mono && numberOfSourceChannels == Channels::Stereo)
    {
        // Handle stereo -> mono case. output = 0.5 * (input.L + input.R).
        AudioBus & sourceBusSafe = const_cast<AudioBus &>(sourceBus);

        const float * sourceL = sourceBusSafe.channelByType(Channel::Left)->data();
        const float * sourceR = sourceBusSafe.channelByType(Channel::Right)->data();

        float * destination = channelByType(Channel::Left)->mutableData();
        vadd(sourceL, 1, sourceR, 1, destination, 1, length());
        float scale = 0.5;
        vsmul(destination, 1, &scale, destination, 1, length());
    }
    else if (numberOfDestinationChannels == Channels::Surround_5_1 && numberOfSourceChannels == Channels::Mono)
    {
        // Handle mono -> 5.1 case, copy mono channel to center.
        channelByType(Channel::Center)->copyFrom(sourceBus.channel(0));
        channelByType(Channel::Left)->zero();
        channelByType(Channel::Right)->zero();
        channelByType(Channel::LFE)->zero();
        channelByType(Channel::SurroundLeft)->zero();
        channelByType(Channel::SurroundRight)->zero();
    }
    else if (numberOfDestinationChannels == Channels::Mono && numberOfSourceChannels == Channels::Surround_5_1)
    {
        // Handle 5.1 -> mono case.
        zero();
        speakersSumFrom5_1_ToMono(sourceBus);
    }
    else if (numberOfDestinationChannels == Channels::Surround_7_1 && numberOfSourceChannels == Channels::Mono)
    {
        // Handle mono -> 7.1 case, copy mono channel to center.
        channelByType(Channel::Center)->copyFrom(sourceBus.channel(0));
        channelByType(Channel::Left)->zero();
        channelByType(Channel::Right)->zero();
        channelByType(Channel::LFE)->zero();
        channelByType(Channel::SurroundLeft)->zero();
        channelByType(Channel::SurroundRight)->zero();
        channelByType(Channel::BackLeft)->zero();
        channelByType(Channel::BackRight)->zero();
    }
    else if (numberOfDestinationChannels == Channels::Mono && numberOfSourceChannels == Channels::Surround_7_1)
    {
        // Handle 7.1 -> mono case.
        zero();
        speakersSumFrom7_1_ToMono(sourceBus);
    }
    else
    {
        // Fallback for unknown combinations.
        discreteCopyFrom(sourceBus);
    }
}

void AudioBus::speakersSumFrom(const AudioBus & sourceBus)
{
    // FIXME: Implement down mixing 5.1 && 7.1 to stereo.
    // https://bugs.webkit.org/show_bug.cgi?id=79192

    const int numberOfSourceChannels = sourceBus.numberOfChannels();
    const int numberOfDestinationChannels = numberOfChannels();

    if (numberOfDestinationChannels == Channels::Stereo && numberOfSourceChannels == Channels::Mono)
    {
        // Handle mono -> stereo case (summing mono channel into both left and right).
        const AudioChannel * sourceChannel = sourceBus.channel(0);
        channelByType(Channel::Left)->sumFrom(sourceChannel);
        channelByType(Channel::Right)->sumFrom(sourceChannel);
    }
    else if (numberOfDestinationChannels == Channels::Mono && numberOfSourceChannels == Channels::Stereo)
    {
        // Handle stereo -> mono case. output += 0.5 * (input.L + input.R).
        AudioBus & sourceBusSafe = const_cast<AudioBus &>(sourceBus);

        const float * sourceL = sourceBusSafe.channelByType(Channel::Left)->data();
        const float * sourceR = sourceBusSafe.channelByType(Channel::Right)->data();

        float * destination = channelByType(Channel::Left)->mutableData();
        float scale = 0.5;
        vsma(sourceL, 1, &scale, destination, 1, length());
        vsma(sourceR, 1, &scale, destination, 1, length());
    }
    else if (numberOfDestinationChannels == Channels::Surround_5_1 && numberOfSourceChannels == Channels::Mono)
    {
        // Handle mono -> 5.1 case, sum mono channel into center.
        channelByType(Channel::Center)->sumFrom(sourceBus.channel(0));
    }
    else if (numberOfDestinationChannels == Channels::Mono && numberOfSourceChannels == Channels::Surround_5_1)
    {
        // Handle 5.1 -> mono case.
        speakersSumFrom5_1_ToMono(sourceBus);
    }
    else if (numberOfDestinationChannels == Channels::Surround_7_1 && numberOfSourceChannels == Channels::Mono)
    {
        // Handle mono -> 7.1 case, sum mono channel into center.
        channelByType(Channel::Center)->sumFrom(sourceBus.channel(0));
    }
    else if (numberOfDestinationChannels == Channels::Mono && numberOfSourceChannels == Channels::Surround_7_1)
    {
        // Handle 7.1 -> mono case.
        speakersSumFrom7_1_ToMono(sourceBus);
    }
    else
    {
        // Fallback for unknown combinations.
        discreteSumFrom(sourceBus);
    }
}

void AudioBus::speakersSumFrom5_1_ToMono(const AudioBus & sourceBus)
{
    AudioBus & sourceBusSafe = const_cast<AudioBus &>(sourceBus);

    const float * sourceL = sourceBusSafe.channelByType(Channel::Left)->data();
    const float * sourceR = sourceBusSafe.channelByType(Channel::Right)->data();
    const float * sourceC = sourceBusSafe.channelByType(Channel::Center)->data();
    const float * sourceSL = sourceBusSafe.channelByType(Channel::SurroundLeft)->data();
    const float * sourceSR = sourceBusSafe.channelByType(Channel::SurroundRight)->data();

    float * destination = channelByType(Channel::Left)->mutableData();

    AudioFloatArray temp(length());
    float * tempData = temp.data();

    // Sum in L and R.
    vadd(sourceL, 1, sourceR, 1, tempData, 1, length());
    float scale = 0.7071f;
    vsmul(tempData, 1, &scale, tempData, 1, length());
    vadd(tempData, 1, destination, 1, destination, 1, length());

    // Sum in SL and SR.
    vadd(sourceSL, 1, sourceSR, 1, tempData, 1, length());
    scale = 0.5;
    vsmul(tempData, 1, &scale, tempData, 1, length());
    vadd(tempData, 1, destination, 1, destination, 1, length());

    // Sum in center.
    vadd(sourceC, 1, destination, 1, destination, 1, length());
}

void AudioBus::speakersSumFrom7_1_ToMono(const AudioBus & sourceBus)
{
    AudioBus & sourceBusSafe = const_cast<AudioBus &>(sourceBus);

    const float * sourceL = sourceBusSafe.channelByType(Channel::Left)->data();
    const float * sourceR = sourceBusSafe.channelByType(Channel::Right)->data();
    const float * sourceC = sourceBusSafe.channelByType(Channel::Center)->data();
    const float * sourceSL = sourceBusSafe.channelByType(Channel::SurroundLeft)->data();
    const float * sourceSR = sourceBusSafe.channelByType(Channel::SurroundRight)->data();
    const float * sourceBL = sourceBusSafe.channelByType(Channel::BackLeft)->data();
    const float * sourceBR = sourceBusSafe.channelByType(Channel::BackRight)->data();

    float * destination = channelByType(Channel::Left)->mutableData();

    AudioFloatArray temp(length());
    float * tempData = temp.data();

    // Sum in L and R.
    vadd(sourceL, 1, sourceR, 1, tempData, 1, length());
    float scale = 0.7071f;
    vsmul(tempData, 1, &scale, tempData, 1, length());
    vadd(tempData, 1, destination, 1, destination, 1, length());

    // Sum in SL and SR.
    vadd(sourceSL, 1, sourceSR, 1, tempData, 1, length());
    scale = 0.5;
    vsmul(tempData, 1, &scale, tempData, 1, length());
    vadd(tempData, 1, destination, 1, destination, 1, length());

    // Sum in BL and BR.
    vadd(sourceBL, 1, sourceBR, 1, tempData, 1, length());
    scale = 0.5;
    vsmul(tempData, 1, &scale, tempData, 1, length());
    vadd(tempData, 1, destination, 1, destination, 1, length());

    // Sum in center.
    vadd(sourceC, 1, destination, 1, destination, 1, length());
}

void AudioBus::discreteCopyFrom(const AudioBus & sourceBus)
{
    const int numberOfSourceChannels = sourceBus.numberOfChannels();
    const int numberOfDestinationChannels = numberOfChannels();

    if (numberOfDestinationChannels < numberOfSourceChannels)
    {
        // Down-mix by copying channels and dropping the remaining.
        for (int i = 0; i < numberOfDestinationChannels; ++i)
        {
            channel(i)->copyFrom(sourceBus.channel(i));
        }
    }
    else if (numberOfDestinationChannels > numberOfSourceChannels)
    {
        // Up-mix by copying as many channels as we have, then zeroing remaining channels.
        for (int i = 0; i < numberOfSourceChannels; ++i)
        {
            channel(i)->copyFrom(sourceBus.channel(i));
        }

        for (int i = numberOfSourceChannels; i < numberOfDestinationChannels; ++i)
        {
            channel(i)->zero();
        }
    }
}

void AudioBus::discreteSumFrom(const AudioBus & sourceBus)
{
    const int numberOfSourceChannels = sourceBus.numberOfChannels();
    const int numberOfDestinationChannels = numberOfChannels();

    if (numberOfDestinationChannels < numberOfSourceChannels)
    {
        // Down-mix by summing channels and dropping the remaining.
        for (int i = 0; i < numberOfDestinationChannels; ++i)
        {
            channel(i)->sumFrom(sourceBus.channel(i));
        }
    }
    else if (numberOfDestinationChannels > numberOfSourceChannels)
    {
        // Up-mix by summing as many channels as we have.
        for (int i = 0; i < numberOfSourceChannels; ++i)
        {
            channel(i)->sumFrom(sourceBus.channel(i));
        }
    }
}

void AudioBus::copyWithGainFrom(const AudioBus & sourceBus, float * lastMixGain, float targetGain)
{
    if (!topologyMatches(sourceBus))
    {
        // happens if a connection has been made, but the channel count has yet to be propagated.
        zero();
        return;
    }

    if (sourceBus.isSilent())
    {
        zero();
        return;
    }

    const int numberOfChannels = static_cast<int>(m_channels.size());
    ASSERT(numberOfChannels <= MaxBusChannels);
    if (numberOfChannels > MaxBusChannels) return;

    // If it is copying from the same bus and no need to change gain, just return.
    if ((this == &sourceBus) && (*lastMixGain == targetGain) && (targetGain == 1))
    {
        return;
    }

    AudioBus & sourceBusSafe = const_cast<AudioBus &>(sourceBus);
    const float * sources[MaxBusChannels];
    float * destinations[MaxBusChannels];

    for (int i = 0; i < numberOfChannels; ++i)
    {
        sources[i] = sourceBusSafe.channel(i)->data();
        destinations[i] = channel(i)->mutableData();
    }

    // We don't want to suddenly change the gain from mixing one time slice to the next,
    // so we "de-zipper" by slowly changing the gain each sample-frame until we've achieved the target gain.

    // Take master bus gain into account as well as the targetGain.
    float totalDesiredGain = static_cast<float>(m_busGain * targetGain);

    // First time, snap directly to totalDesiredGain.
    float gain = static_cast<float>(m_isFirstTime ? totalDesiredGain : *lastMixGain);
    m_isFirstTime = false;

    const float DezipperRate = 0.005f;
    int framesToProcess = length();

    // If the gain is within epsilon of totalDesiredGain, we can skip dezippering.
    // FIXME: this value may need tweaking.
    const float epsilon = 0.001f;
    float gainDiff = fabs(totalDesiredGain - gain);

    // Number of frames to de-zipper before we are close enough to the target gain.
    // FIXME: framesToDezipper could be smaller when target gain is close enough within this process loop.
    int framesToDezipper = (gainDiff < epsilon) ? 0 : framesToProcess;

    if (framesToDezipper)
    {
        if (!m_dezipperGainValues.get() || m_dezipperGainValues->size() < framesToDezipper)
        {
            m_dezipperGainValues = std::unique_ptr<AudioFloatArray>(new AudioFloatArray(framesToDezipper));
        }

        float * gainValues = m_dezipperGainValues->data();

        for (int i = 0; i < framesToDezipper; ++i)
        {
            gain += (totalDesiredGain - gain) * DezipperRate;

            // FIXME: If we are clever enough in calculating the framesToDezipper value, we can probably get
            // rid of this DenormalDisabler::flushDenormalFloatToZero() call.
            gain = DenormalDisabler::flushDenormalFloatToZero(gain);
            *gainValues++ = gain;
        }

        for (int channelIndex = 0; channelIndex < numberOfChannels; ++channelIndex)
        {
            vmul(sources[channelIndex], 1, m_dezipperGainValues->data(), 1, destinations[channelIndex], 1, framesToDezipper);
            sources[channelIndex] += framesToDezipper;
            destinations[channelIndex] += framesToDezipper;
        }
    }
    else
    {
        gain = totalDesiredGain;
    }

    // Apply constant gain after de-zippering has converged on target gain.
    if (framesToDezipper < framesToProcess)
    {
        for (int channelIndex = 0; channelIndex < numberOfChannels; ++channelIndex)
        {
            vsmul(sources[channelIndex], 1, &gain, destinations[channelIndex], 1, framesToProcess - framesToDezipper);
        }
    }

    // Save the target gain as the starting point for next time around.
    *lastMixGain = gain;
}

void AudioBus::copyWithSampleAccurateGainValuesFrom(const AudioBus & sourceBus, float * gainValues, int numberOfGainValues)
{
    // Make sure we're processing from the same type of bus.
    // We *are* able to process from mono -> stereo
    if (sourceBus.numberOfChannels() != Channels::Mono && !topologyMatches(sourceBus))
    {
        ASSERT_NOT_REACHED();
        return;
    }

    if (!gainValues || numberOfGainValues > sourceBus.length())
    {
        ASSERT_NOT_REACHED();
        return;
    }

    if (sourceBus.length() == numberOfGainValues && sourceBus.length() == length() && sourceBus.isSilent())
    {
        zero();
        return;
    }

    // We handle both the 1 -> N and N -> N case here.
    const float * source = sourceBus.channel(0)->data();
    for (int channelIndex = 0; channelIndex < numberOfChannels(); ++channelIndex)
    {
        if (sourceBus.numberOfChannels() == numberOfChannels())
        {
            source = sourceBus.channel(channelIndex)->data();
        }
        vmul(source, 1, gainValues, 1, channel(channelIndex)->mutableData(), 1, numberOfGainValues);
    }
}

std::unique_ptr<AudioBus> AudioBus::createBySampleRateConverting(const AudioBus * sourceBus, bool mixToMono, float newSampleRate)
{
    // sourceBus's sample-rate must be known.
    ASSERT(sourceBus && sourceBus->sampleRate());

    if (!sourceBus || !sourceBus->sampleRate())
    {
        return nullptr;
    }

    double sourceSampleRate = sourceBus->sampleRate();
    float destinationSampleRate = static_cast<float>(newSampleRate);
    double sampleRateRatio = sourceSampleRate / destinationSampleRate;
    int numberOfSourceChannels = sourceBus->numberOfChannels();

    if (numberOfSourceChannels == Channels::Mono)
    {
        mixToMono = false;  // already mono
    }

    if (sourceSampleRate == destinationSampleRate)
    {
        // No sample-rate conversion is necessary.
        if (mixToMono) return AudioBus::createByMixingToMono(sourceBus);

        // Return exact copy.
        return AudioBus::createBufferFromRange(sourceBus, 0, sourceBus->length());
    }

    if (sourceBus->isSilent())
    {
        std::unique_ptr<AudioBus> silentBus(new AudioBus(numberOfSourceChannels, static_cast<int>(sourceBus->length() / sampleRateRatio)));
        silentBus->setSampleRate(newSampleRate);
        return silentBus;
    }

    // First, mix to mono (if necessary) then sample-rate convert.
    const AudioBus * resamplerSourceBus;
    std::unique_ptr<AudioBus> mixedMonoBus;
    if (mixToMono)
    {
        mixedMonoBus = AudioBus::createByMixingToMono(sourceBus);
        resamplerSourceBus = mixedMonoBus.get();
    }
    else
    {
        // Directly resample without down-mixing.
        resamplerSourceBus = sourceBus;
    }

    // Calculate destination length based on the sample-rates.
    int sourceLength = resamplerSourceBus->length();
    int destinationLength = static_cast<int>(sourceLength / sampleRateRatio);

    // Create destination bus with same number of channels.
    int numberOfDestinationChannels = resamplerSourceBus->numberOfChannels();
    std::unique_ptr<AudioBus> destinationBus(new AudioBus(numberOfDestinationChannels, destinationLength));

    // Sample-rate convert each channel.
    for (int i = 0; i < numberOfDestinationChannels; ++i)
    {
        SRC_DATA convert;
        convert.data_in = resamplerSourceBus->channel(i)->data();
        convert.data_out = destinationBus->channel(i)->mutableData();
        convert.input_frames = sourceLength;
        convert.output_frames = destinationLength;
        convert.input_frames_used = 0;
        convert.output_frames_gen = 0;
        convert.end_of_input = 0;
        convert.src_ratio = 1. / sampleRateRatio;
        int result = src_simple(&convert, SRC_SINC_FASTEST, 1);
        if (result != 0)
        {
            std::unique_ptr<AudioBus> silentBus(new AudioBus(numberOfSourceChannels, static_cast<int>(sourceBus->length() / sampleRateRatio)));
            silentBus->setSampleRate(newSampleRate);
            return silentBus;
        }
    }

    destinationBus->clearSilentFlag();
    destinationBus->setSampleRate(newSampleRate);
    return destinationBus;
}

std::unique_ptr<AudioBus> AudioBus::createByMixingToMono(const AudioBus * sourceBus)
{
    if (sourceBus->isSilent())
    {
        return std::unique_ptr<AudioBus>(new AudioBus(Channels::Mono, sourceBus->length()));
    }

    switch (sourceBus->numberOfChannels())
    {
        case Channels::Mono:
            // Simply create an exact copy.
            return AudioBus::createBufferFromRange(sourceBus, 0, sourceBus->length());
        default:
        {
            const int n = sourceBus->length();
            const int m = sourceBus->numberOfChannels();
            std::unique_ptr<AudioBus> destinationBus(new AudioBus(Channels::Mono, n));
            float * destination = destinationBus->channel(0)->mutableData();

            // Do the mono mixdown.
            for (int i = 0; i < n; ++i)
            {
                destination[i] = 0.0;

                for (int j = 0; j < m; ++j)
                {
                    const float * source = sourceBus->channel(j)->data();
                    destination[j] += source[j];
                }

                destination[i] /= m;
            }

            destinationBus->clearSilentFlag();
            destinationBus->setSampleRate(sourceBus->sampleRate());

            return destinationBus;
        }
    }

    ASSERT_NOT_REACHED();

    return nullptr;
}

bool AudioBus::isSilent() const
{
    for (int i = 0; i < m_channels.size(); ++i)
    {
        if (!m_channels[i]->isSilent())
        {
            return false;
        }
    }
    return true;
}

void AudioBus::clearSilentFlag()
{
    for (int i = 0; i < m_channels.size(); ++i)
    {
        m_channels[i]->clearSilentFlag();
    }
}

}  // end namespace lab
