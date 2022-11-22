// License: BSD 3 Clause
// Copyright (C) 2015+, The LabSound Authors. All rights reserved.

#ifndef AudioScheduledSourceNode_h
#define AudioScheduledSourceNode_h

#include "LabSound/core/AudioNode.h"

namespace lab
{

/*
    AudioScheduledSourceNode adds a scheduling interface to
    AudioNode. The scheduler is in the base class, but only nodes
    derived from AudioScheduledSourceNode can be scheduled.
 */

class AudioScheduledSourceNode : public AudioNode
{
public:
    AudioScheduledSourceNode() = delete;

    explicit AudioScheduledSourceNode(AudioContext & ac) : AudioNode(ac) { }
    virtual ~AudioScheduledSourceNode() = default;

    virtual bool isScheduledNode() const override { return true; }

    bool isPlayingOrScheduled() const
    {
        return _scheduler._playbackState >= SchedulingState::SCHEDULED &&
               _scheduler._playbackState <= SchedulingState::STOPPING;
    }

    // Start time, measured as seconds from the current epochal time
    void start(float when) { _scheduler.start(when); }

    uint64_t startWhen() const { return _scheduler._startWhen; }

    // Stop time, measured as seconds from the current epochal time
    void stop(float when) { _scheduler.stop(when); }

    SchedulingState playbackState() const { return _scheduler._playbackState; }
    bool hasFinished() const { return _scheduler.hasFinished(); }
    void setOnEnded(std::function<void()> fn) { _scheduler._onEnded = fn; }
};

}  // namespace lab

#endif  // AudioScheduledSourceNode_h
