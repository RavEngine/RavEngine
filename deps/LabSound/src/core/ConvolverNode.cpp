
// SPDX-License-Identifier: BSD-2-Clause
// Copyright (C) 2015+, The LabSound Authors. All rights reserved.

#include "LabSound/core/ConvolverNode.h"
#include "LabSound/core/AudioBus.h"
#include "LabSound/core/AudioNodeInput.h"
#include "LabSound/core/AudioNodeOutput.h"
#include "LabSound/extended/AudioContextLock.h"
#include "LabSound/extended/Registry.h"
#include "internal/VectorMath.h"
#include <cmath>

namespace lab
{

using std::isinf;
using std::isnan;

// from _SoundPipe_FFT.cpp
struct sp_conv;
struct sp_data;
struct sp_ftbl;
typedef float SPFLOAT;
int sp_create(sp_data ** spp);
int sp_destroy(sp_data ** spp);
int sp_conv_compute(sp_data * sp, sp_conv * p, SPFLOAT * in, SPFLOAT * out);
int sp_conv_create(sp_conv ** p);
int sp_conv_destroy(sp_conv ** p);
int sp_conv_init(sp_data * sp, sp_conv * p, sp_ftbl * ft, SPFLOAT iPartLen);
int sp_ftbl_destroy(sp_ftbl ** ft);
int sp_ftbl_bind(sp_data * sp, sp_ftbl ** ft, SPFLOAT * tbl, size_t size);

//------------------------------------------------------------------------------
// calculateNormalizationScale is adapted from webkit's Reverb.cpp, and carried the license:
// Empirical gain calibration tested across many impulse responses to ensure
// perceived volume is same as dry (unprocessed) signal
const float GainCalibration = -58;
const float GainCalibrationSampleRate = 44100;

// A minimum power value to when normalizing a silent (or very quiet) impulse response
const float MinPower = 0.000125f;

// calculateNormalizationScale license: BSD 3 Clause, Copyright (C) 2010, Google Inc. All rights reserved.
static float calculateNormalizationScale(AudioBus * response)
{
    // Normalize by RMS power
    size_t numberOfChannels = response->numberOfChannels();
    int length = response->length();

    float power = 0;

    for (int i = 0; i < numberOfChannels; ++i)
    {
        float channelPower = 0;
        VectorMath::vsvesq(response->channel(i)->data(), 1, &channelPower, length);
        power += channelPower;
    }

    power = sqrt(power / (numberOfChannels * length));

    // Protect against accidental overload
    if (isinf(power) || isnan(power) || power < MinPower)
        power = MinPower;

    float scale = 1 / power;

    scale *= powf(10, GainCalibration * 0.05f);  // calibrate to make perceived volume same as unprocessed

    // Scale depends on sample-rate.
    if (response->sampleRate())
        scale *= GainCalibrationSampleRate / response->sampleRate();

    // True-stereo compensation
    if (response->numberOfChannels() == Channels::Quad)
        scale *= 0.5f;

    return scale;
}
//------------------------------------------------------------------------------

ConvolverNode::ReverbKernel::ReverbKernel(ReverbKernel && rh) noexcept
    : conv(rh.conv)
    , ft(rh.ft)
{
    rh.conv = nullptr;
    rh.ft = nullptr;
}

ConvolverNode::ReverbKernel::~ReverbKernel()
{
    if (ft)
        lab::sp_ftbl_destroy(&ft);

    if (conv)
        lab::sp_conv_destroy(&conv);
}

ConvolverNode::ConvolverNode(AudioContext& ac)
    : AudioScheduledSourceNode(ac)
    , _normalize(std::make_shared<AudioSetting>("normalize", "NRML", AudioSetting::Type::Bool))
    , _impulseResponseClip(std::make_shared<AudioSetting>("impulseResponse", "IMPL", AudioSetting::Type::Bus))
{
    _swap_ready = false;

    m_settings.push_back(_impulseResponseClip);
    m_settings.push_back(_normalize);
    _normalize->setBool(true);

    addInput(std::unique_ptr<AudioNodeInput>(new AudioNodeInput(this)));
    addOutput(std::unique_ptr<AudioNodeOutput>(new AudioNodeOutput(this, 0)));

    lab::sp_create(&_sp);

    _impulseResponseClip->setValueChanged([this]() {
        this->_activateNewImpulse();
    });

    initialize();
}

ConvolverNode::~ConvolverNode()
{
    _kernels.clear();
    lab::sp_destroy(&_sp);
    uninitialize();
}

bool ConvolverNode::normalize() const
{
    return _normalize->valueBool();
}
void ConvolverNode::setNormalize(bool new_n)
{
    bool n = normalize();
    if (new_n == n)
        return;

    auto clip = _impulseResponseClip->valueBus();
    if (clip)
    {
        size_t len = clip->length();
        if (n && _scale != 1.f)
        {
            // undo previous normalization
            float s = 1.f / _scale;
            for (int i = 0; i < clip->numberOfChannels(); ++i)
            {
                float * data = clip->channel(i)->mutableData();
                for (int j = 0; j < len; ++j)
                    data[j] *= s;
            }
        }
        if (!n)
        {
            // wasn't previously normalized, so normalize
            _scale = calculateNormalizationScale(clip.get());
            for (int i = 0; i < clip->numberOfChannels(); ++i)
            {
                float * data = clip->channel(i)->mutableData();
                for (int j = 0; j < len; ++j)
                    data[j] *= _scale;
            }
        }
    }

    _normalize->setBool(new_n);
}

void ConvolverNode::setImpulse(std::shared_ptr<AudioBus> bus)
{
    if (!bus)
        return;

    auto new_bus = AudioBus::createByCloning(bus.get());
    _impulseResponseClip->setBus(new_bus.get());    // setBus will invoke _activatNewImpulse()
}

void ConvolverNode::_activateNewImpulse()
{
    auto clip = _impulseResponseClip->valueBus();
    size_t len = clip->length();
    _scale = 1;
    if (normalize())
    {
        _scale = calculateNormalizationScale(clip.get());
        for (int i = 0; i < clip->numberOfChannels(); ++i)
        {
            float * data = clip->channel(i)->mutableData();
            for (int j = 0; j < len; ++j)
                data[j] *= _scale;
        }
    }

    {
        std::unique_lock<std::mutex> kernel_guard(_kernel_mutex);
        int c = static_cast<int>(clip->numberOfChannels());
        for (int i = 0; i < c; ++i)
        {
            // create one kernel per IR channel
            ReverbKernel kernel;

            // ft doesn't own the data; it does retain a pointer to it.
            sp_ftbl_bind(_sp, &kernel.ft,
                         clip->channel(0)->mutableData(), clip->channel(0)->length());

            sp_conv_create(&kernel.conv);
            sp_conv_init(_sp, kernel.conv, kernel.ft, 8192);

            _pending_kernels.emplace_back(std::move(kernel));
        }
        _swap_ready = true;
    }

    start(0);
}

std::shared_ptr<AudioBus> ConvolverNode::getImpulse() const
{
    return _impulseResponseClip->valueBus();
}

void ConvolverNode::process(ContextRenderLock & r, int bufferSize)
{
    if (_swap_ready) {
        // this could cause an audio hiccough when swapping, but it's necessary to avoid a race
        std::unique_lock<std::mutex> kernel_guard(_kernel_mutex);
        _swap_ready = false;
        std::swap(_kernels, _pending_kernels);
        _pending_kernels.clear();
    }

    AudioBus * outputBus = output(0)->bus(r);
    AudioBus * inputBus = input(0)->bus(r);

    if (!isInitialized() || !outputBus || !inputBus || !inputBus->numberOfChannels() || !_kernels.size())
    {
        outputBus->zero();
        return;
    }

    // if the user never specified the number of output channels, make it match the input
    if (!outputBus->numberOfChannels())
    {
        output(0)->setNumberOfChannels(r, inputBus->numberOfChannels());
        outputBus = output(0)->bus(r);  // set number of channels invalidates the pointer
    }

    int quantumFrameOffset = _scheduler._renderOffset;
    int nonSilentFramesToProcess = _scheduler._renderLength;

    int numInputChannels = static_cast<int>(inputBus->numberOfChannels());
    int numOutputChannels = static_cast<int>(outputBus->numberOfChannels());
    int numReverbChannels = static_cast<int>(_kernels.size());

    if (!nonSilentFramesToProcess)
    {
        outputBus->zero();
        return;
    }

    /// @todo should a situation such as 1:2:1 be invalid, or should it be powersum(1:1:1, 1:2:1)?
    /// at the moment, this routine trivially does 1:1:1 only.

    for (int i = 0; i < numOutputChannels; ++i)
    {
        int kernel = i < numReverbChannels ? i : numReverbChannels - 1;
        lab::sp_conv * conv = _kernels[kernel].conv;
        float* destP = outputBus->channel(i)->mutableData() + _scheduler._renderOffset;

        // Start rendering at the correct offset.
        destP += quantumFrameOffset;
        {
            AudioBus * input_bus = input(0)->bus(r);
            int in_channel = i < numInputChannels ? i : numInputChannels - 1;
            float const* data = input_bus->channel(in_channel)->data() + quantumFrameOffset;
            size_t c = input_bus->channel(in_channel)->length();
            for (int j = 0; j < _scheduler._renderLength; ++j)
            {
                lab::SPFLOAT in = j < c ? data[j] : 0.f;  // don't read off the end of the input buffer
                lab::SPFLOAT out = 0.f;
                lab::sp_conv_compute(_sp, conv, &in, &out);
                *destP++ = out;
            }
        }
    }

    _now += double(_scheduler._renderLength) / r.context()->sampleRate();
    outputBus->clearSilentFlag();
}

void ConvolverNode::reset(ContextRenderLock &)
{
    _kernels.clear();
}

bool ConvolverNode::propagatesSilence(ContextRenderLock & r) const
{
    return !isPlayingOrScheduled() || hasFinished();
}

}  // lab::Sound
