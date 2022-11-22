// License: BSD 2 Clause
// Copyright (C) 2010, Google Inc. All rights reserved.
// Copyright (C) 2015+, The LabSound Authors. All rights reserved.

#include "LabSound/core/AudioNode.h"

#include "LabSound/core/AudioBus.h"
#include "LabSound/core/AudioContext.h"
#include "LabSound/core/AudioNodeInput.h"
#include "LabSound/core/AudioNodeOutput.h"
#include "LabSound/core/AudioParam.h"
#include "LabSound/core/AudioSetting.h"
#include "LabSound/extended/AudioContextLock.h"

#include "internal/Assertions.h"

using namespace std;

//#define ASN_PRINT(...)
#define ASN_PRINT(a) printf(a)

namespace lab
{


const char* schedulingStateName(SchedulingState s)
{
    switch (s)
    {
    case SchedulingState::UNSCHEDULED: return "UNSCHEDULED";
    case SchedulingState::SCHEDULED: return "SCHEDULED";
    case SchedulingState::FADE_IN: return "FADE_IN";
    case SchedulingState::PLAYING: return "PLAYING";
    case SchedulingState::STOPPING: return "STOPPING";
    case SchedulingState::RESETTING: return "RESETTING";
    case SchedulingState::FINISHING: return "FINISHING";
    case SchedulingState::FINISHED: return "FINISHED";
    }
    return "Unknown";
}


const int start_envelope = 64;
const int end_envelope = 64;

AudioNodeScheduler::AudioNodeScheduler(float sampleRate) 
    : _epoch(0)
    , _startWhen(std::numeric_limits<uint64_t>::max())
    , _stopWhen(std::numeric_limits<uint64_t>::max())
    , _sampleRate(sampleRate)
{
}

bool AudioNodeScheduler::update(ContextRenderLock & r, int epoch_length)
{
    uint64_t proposed_epoch = r.context()->currentSampleFrame();
    if (_epoch >= proposed_epoch)
        return false;

    _epoch = proposed_epoch;
    _renderOffset = 0;
    _renderLength = epoch_length;

    switch (_playbackState)
    {
        case SchedulingState::UNSCHEDULED:
            break;
            
        case SchedulingState::SCHEDULED:
            // start has been called, start looking for the start time
            if (_startWhen <= _epoch)
            {
                // exactly on start, or late, get going straight away
                _renderOffset = 0;
                _renderLength = epoch_length;
                _playbackState = SchedulingState::FADE_IN;
                ASN_PRINT("fade in\n");
            }
            else if (_startWhen < _epoch + epoch_length)
            {
                // start falls within the frame
                _renderOffset = static_cast<int>(_startWhen - _epoch);
                _renderLength = epoch_length - _renderOffset;
                _playbackState = SchedulingState::FADE_IN;
                ASN_PRINT("fade in\n");
            }
            
            /// @TODO the case of a start and stop within one epoch needs to be special
            /// cased, to fit this current architecture, there'd be a FADE_IN_OUT scheduling
            /// state so that the envelope can be correctly applied.
            // FADE_IN_OUT would transition to UNSCHEDULED.
            break;
            
        case SchedulingState::FADE_IN:
            // start time has been achieved, there'll be one quantum with fade in applied.
            _renderOffset = 0;
            _playbackState = SchedulingState::PLAYING;
            ASN_PRINT("playing\n");
            // fall through to PLAYING to allow render length to be adjusted if stop-start is less than one quantum length
            
        case SchedulingState::PLAYING:
            /// @TODO include the end envelope in the stop check so that a scheduled stop that
            /// spans this quantum and the next ends in the current quantum
            
            if (_stopWhen <= _epoch)
            {
                // exactly on start, or late, stop straight away, render a whole frame of fade out
                _renderLength = epoch_length - _renderOffset;
                _playbackState = SchedulingState::STOPPING;
                ASN_PRINT("stopping\n");
            }
            else if (_stopWhen < _epoch + epoch_length)
            {
                // stop falls within the frame
                _renderOffset = 0;
                _renderLength = static_cast<int>(_stopWhen - _epoch);
                _playbackState = SchedulingState::STOPPING;
                ASN_PRINT("stopping\n");
            }
            
            // do not fall through to STOPPING because one quantum must render the fade out effect
            break;
            
        case SchedulingState::STOPPING:
            if (_epoch + epoch_length >= _stopWhen)
            {
                // scheduled stop has occured, so make sure stop doesn't immediately trigger again
                _stopWhen = std::numeric_limits<uint64_t>::max();
                _playbackState = SchedulingState::UNSCHEDULED;
                if(_onEnded)
                    r.context()->enqueueEvent(_onEnded);
                ASN_PRINT("unscheduled\n");
            }
            break;
            
        case SchedulingState::RESETTING:
            break;
        case SchedulingState::FINISHING:
            break;
        case SchedulingState::FINISHED:
            break;
    }

    ASSERT(_renderOffset < epoch_length);
    return true;
}

void AudioNodeScheduler::start(double when)
{
    // irrespective of start state, onStart will be called if possible
    // to allow a node to use subsequent starts as a hint
    if (_onStart)
        _onStart(when);

    // if already scheduled or playing, nothing to do
    if (_playbackState == SchedulingState::SCHEDULED ||
        _playbackState == SchedulingState::PLAYING)
        return;

    // start cancels stop
    _stopWhen = std::numeric_limits<uint64_t>::max();

    // treat non finite, or max values as a cancellation of stopping or resetting
    if (!std::isfinite(when) || when == std::numeric_limits<double>::max())
    {
        if (_playbackState == SchedulingState::STOPPING ||
            _playbackState == SchedulingState::RESETTING)
        {
            _playbackState = SchedulingState::PLAYING;
        }
        return;
    }

    _startWhen = _epoch + static_cast<uint64_t>(when * _sampleRate);
    _playbackState = SchedulingState::SCHEDULED;
}

void AudioNodeScheduler::stop(double when)
{
    // if the node is on the way to a terminal state do nothing
    if (_playbackState >= SchedulingState::STOPPING)
        return;

    // treat non-finite, and FLT_MAX as stop cancellation, if already playing
    if (!std::isfinite(when) || when == std::numeric_limits<double>::max())
    {
        // stop at a non-finite time means don't stop.
        _stopWhen = std::numeric_limits<uint64_t>::max(); // cancel stop
        if (_playbackState == SchedulingState::STOPPING)
        {
            // if in the process of stopping, set it back to scheduling to start immediately
            _playbackState = SchedulingState::SCHEDULED;
            _startWhen = 0;
        }

        return;
    }

    // clamp timing to now or the future
    if (when < 0)
        when = 0;

    // let the scheduler know when to activate the STOPPING state
    _stopWhen = _epoch + static_cast<uint64_t>(when * _sampleRate);
}

void AudioNodeScheduler::reset()
{
    _startWhen = std::numeric_limits<uint64_t>::max();
    _stopWhen = std::numeric_limits<uint64_t>::max();
    if (_playbackState != SchedulingState::UNSCHEDULED)
        _playbackState = SchedulingState::RESETTING;
}

void AudioNodeScheduler::finish(ContextRenderLock & r)
{
    if (_playbackState < SchedulingState::PLAYING)
        _playbackState = SchedulingState::FINISHING;
    else if (_playbackState >= SchedulingState::PLAYING &&
             _playbackState < SchedulingState::FINISHED)
        _playbackState = SchedulingState::FINISHING;

    r.context()->enqueueEvent(_onEnded);
}



AudioNode::AudioNode(AudioContext & ac)
    : _scheduler(ac.sampleRate())
{ }



AudioNode::~AudioNode()
{
    uninitialize();
}

void AudioNode::initialize()
{
    m_isInitialized = true;
}

void AudioNode::uninitialize()
{
    m_isInitialized = false;
}

void AudioNode::reset(ContextRenderLock& r)
{
    _scheduler.reset();
}

void AudioNode::addInput(ContextGraphLock&, std::unique_ptr<AudioNodeInput> input)
{
    m_inputs.emplace_back(std::move(input));
}

void AudioNode::addOutput(ContextGraphLock&, std::unique_ptr<AudioNodeOutput> output)
{
    m_outputs.emplace_back(std::move(output));
}

void AudioNode::addInput(std::unique_ptr<AudioNodeInput> input)
{
    m_inputs.emplace_back(std::move(input));
}

void AudioNode::addOutput(std::unique_ptr<AudioNodeOutput> output)
{
    m_outputs.emplace_back(std::move(output));
}

std::shared_ptr<AudioNodeOutput> AudioNode::output(char const* const str)
{
    for (auto & i : m_outputs)
    {
        if (i->name() == str)
            return i;
    }
    return {};
}


// safe without a Render lock because vector is immutable
std::shared_ptr<AudioNodeInput> AudioNode::input(int i)
{
    if (i < m_inputs.size()) return m_inputs[i];
    return nullptr;
}

// safe without a Render lock because vector is immutable
std::shared_ptr<AudioNodeOutput> AudioNode::output(int i)
{
    if (i < m_outputs.size()) return m_outputs[i];
    return nullptr;
}

int AudioNode::channelCount()
{
    ASSERT(m_channelCount != 0);
    return m_channelCount;
}

void AudioNode::setChannelCount(ContextGraphLock & g, int channelCount)
{
    if (!g.context())
    {
        throw std::invalid_argument("No context specified");
    }

    if (m_channelCount != channelCount)
    {
        m_channelCount = channelCount;
        if (m_channelCountMode != ChannelCountMode::Max)
        {
            for (auto& input : m_inputs)
                input->changedOutputs(g);
        }
    }
}

void AudioNode::setChannelCountMode(ContextGraphLock & g, ChannelCountMode mode)
{
    if (mode >= ChannelCountMode::End || !g.context())
    {
        throw std::invalid_argument("No context specified");
    }

    if (m_channelCountMode != mode)
    {
        m_channelCountMode = mode;
        for (auto& input : m_inputs)
            input->changedOutputs(g);
    }
}

void AudioNode::processIfNecessary(ContextRenderLock & r, int bufferSize)
{
    if (!isInitialized())
        return;

    auto ac = r.context();
    if (!ac)
        return;

    // outputs cache results in their busses.
    // if the scheduler's recorded epoch is the same as the context's, the node
    // shall bail out as it has been processed once already this epoch.

    if (!_scheduler.update(r, bufferSize))
        return;

    ProfileScope selfScope(totalTime);
    graphTime.zero();

    if (isScheduledNode() && 
        (_scheduler._playbackState < SchedulingState::FADE_IN ||
         _scheduler._playbackState == SchedulingState::FINISHED))
    {
        silenceOutputs(r);
        return;
    }


    // there may need to be silence at the beginning or end of the current quantum.

    int start_zero_count = _scheduler._renderOffset;
    int final_zero_start = _scheduler._renderOffset + _scheduler._renderLength;
    int final_zero_count = bufferSize - final_zero_start;

    // if the input counts need to match the output counts,
    // do it here before pulling inputs
    conformChannelCounts();

    // get inputs in preparation for processing
    {
        ProfileScope scope(graphTime);
        pullInputs(r, bufferSize);
        scope.finalize();   // ensure the scope is not prematurely destructed
    }

    // ensure all requested channel count updates have been resolved
    for (auto& out : m_outputs)
        out->updateRenderingState(r);

    //  initialize the busses with start and final zeroes.
    if (start_zero_count)
    {
        for (auto & out : m_outputs)
            for (int i = 0; i < out->numberOfChannels(); ++i)
                memset(out->bus(r)->channel(i)->mutableData(), 0, sizeof(float) * start_zero_count);
    }

    if (final_zero_count)
    {
        for (auto & out : m_outputs)
            for (int i = 0; i < out->numberOfChannels(); ++i)
                memset(out->bus(r)->channel(i)->mutableData() + final_zero_start, 0, sizeof(float) * final_zero_count);
    }

    // do the signal processing

    process(r, bufferSize);

    // clean pops resulting from starting or stopping

#define OOS(x) (float(x) / float(steps))
    if (start_zero_count > 0 || _scheduler._playbackState == SchedulingState::FADE_IN)
    {
        int steps = start_envelope;
        int damp_start = start_zero_count;
        int damp_end = start_zero_count + steps;
        if (damp_end > bufferSize)
        {
            damp_end = bufferSize;
            steps = bufferSize - start_zero_count;
        }

        // damp out the first samples
        if (damp_end > 0)
            for (auto & out : m_outputs)
                for (int i = 0; i < out->bus(r)->numberOfChannels(); ++i)
                {
                    float* data = out->bus(r)->channel(i)->mutableData();
                    for (int j = damp_start; j < damp_end; ++j)
                        data[j] *= OOS(j - damp_start);
                }
    }

    if (final_zero_count > 0 || _scheduler._playbackState == SchedulingState::STOPPING)
    {
        int steps = end_envelope;
        int damp_end, damp_start;
        if (!final_zero_count)
        {
            damp_end = bufferSize - final_zero_count;
            damp_start = damp_end - steps;
        }
        else
        {
            damp_end = bufferSize - final_zero_count;
            damp_start = damp_end - steps;
        }

        if (damp_start < 0)
        {
            damp_start = 0;
            steps = damp_end;
        }

        // damp out the last samples
        if (steps > 0)
        {
            //printf("out: %d %d\n", damp_start, damp_end);
            for (auto& out : m_outputs)
                for (int i = 0; i < out->numberOfChannels(); ++i)
                {
                    float* data = out->bus(r)->channel(i)->mutableData();
                    for (int j = damp_start; j < damp_end; ++j)
                        data[j] *= OOS(damp_end - j);
                }
        }
    }

    unsilenceOutputs(r);
    selfScope.finalize(); // ensure profile is not prematurely destructed
}

void AudioNode::conformChannelCounts()
{
    return;

    // no generic count conforming. nodes are responsible if they are going to do it at all
/*
    int inputChannelCount = input->numberOfChannels(r);

    bool channelCountChanged = false;
    for (int i = 0; i < numberOfOutputs() && !channelCountChanged; ++i)
    {
        channelCountChanged = isInitialized() && inputChannelCount != output(i)->numberOfChannels();
    }

    if (channelCountChanged)
    {
        uninitialize();
        for (int i = 0; i < numberOfOutputs(); ++i)
        {
            // This will propagate the channel count to any nodes connected further down the chain...
            output(i)->setNumberOfChannels(r, inputChannelCount);
        }
        initialize();
    }
    */
}

void AudioNode::checkNumberOfChannelsForInput(ContextRenderLock & r, AudioNodeInput * input)
{
    ASSERT(r.context());

    if (!input || input != this->input(0).get())
        return;

    for (auto & in : m_inputs)
    {
        if (in.get() == input)
        {
            input->updateInternalBus(r);
            break;
        }
    }
}

bool AudioNode::propagatesSilence(ContextRenderLock& r) const
{
    return _scheduler._playbackState < SchedulingState::FADE_IN ||
        _scheduler._playbackState == SchedulingState::FINISHED;
}

void AudioNode::pullInputs(ContextRenderLock & r, int bufferSize)
{
    ASSERT(r.context());

    // Process all of the AudioNodes connected to our inputs.
    for (auto & in : m_inputs)
    {
        in->pull(r, 0, bufferSize);
    }
}

bool AudioNode::inputsAreSilent(ContextRenderLock & r)
{
    for (auto & in : m_inputs)
    {
        if (!in->bus(r)->isSilent())
        {
            return false;
        }
    }
    return true;
}

void AudioNode::silenceOutputs(ContextRenderLock & r)
{
    for (auto out : m_outputs)
    {
        out->bus(r)->zero();
    }
}

void AudioNode::unsilenceOutputs(ContextRenderLock & r)
{
    for (auto out : m_outputs)
    {
        out->bus(r)->clearSilentFlag();
    }
}

std::shared_ptr<AudioParam> AudioNode::param(char const * const str)
{
    for (auto & p : m_params)
    {
        if (!strcmp(str, p->name().c_str()))
            return p;
    }
    return {};
}

std::shared_ptr<AudioSetting> AudioNode::setting(char const * const str)
{
    for (auto & p : m_settings)
    {
        if (!strcmp(str, p->name().c_str()))
            return p;
    }
    return {};
}

std::vector<std::string> AudioNode::paramNames() const
{
    std::vector<std::string> ret;
    for (auto & p : m_params)
    {
        ret.push_back(p->name());
    }
    return ret;
}
std::vector<std::string> AudioNode::paramShortNames() const
{
    std::vector<std::string> ret;
    for (auto & p : m_params)
    {
        ret.push_back(p->shortName());
    }
    return ret;
}

std::vector<std::string> AudioNode::settingNames() const
{
    std::vector<std::string> ret;
    for (auto & p : m_settings)
    {
        ret.push_back(p->name());
    }
    return ret;
}
std::vector<std::string> AudioNode::settingShortNames() const
{
    std::vector<std::string> ret;
    for (auto & p : m_settings)
    {
        ret.push_back(p->shortName());
    }
    return ret;
}

}  // namespace lab
