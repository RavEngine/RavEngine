// SPDX-License-Identifier: BSD-2-Clause
// Copyright (C) 2015+, The LabSound Authors. All rights reserved.

#ifndef ConvolverNode_h
#define ConvolverNode_h

#include "LabSound/core/AudioScheduledSourceNode.h"
#include "LabSound/core/AudioSetting.h"

#include <memory>
#include <mutex>

namespace lab
{

class AudioBus;
class AudioSetting;
class Reverb;

// private data for reverb computations
struct sp_data;
struct sp_conv;
struct sp_ftbl;

class ConvolverNode final : public AudioScheduledSourceNode
{
public:
    ConvolverNode(AudioContext& ac);
    virtual ~ConvolverNode();

    static const char* static_name() { return "Convolver"; }
    virtual const char* name() const override { return static_name(); }

    bool normalize() const;
    void setNormalize(bool new_n);

    // set impulse will schedule the convolver to begin processing immediately
    // The supplied bus is copied for use as an impulse response.
    void setImpulse(std::shared_ptr<AudioBus> bus);
    std::shared_ptr<AudioBus> getImpulse() const;
    virtual void process(ContextRenderLock & r, int bufferSize) override;
    virtual void reset(ContextRenderLock &) override;

protected:
    virtual double tailTime(ContextRenderLock & r) const override { return 0; }
    virtual double latencyTime(ContextRenderLock & r) const override { return 0; }
    virtual bool propagatesSilence(ContextRenderLock & r) const override;
    double now() const { return _now; }

    void _activateNewImpulse();

    double _now = 0.0;
    float _scale = 1.f;  // normalization value
    sp_data * _sp = nullptr;

    // Normalize the impulse response or not. Must default to true.
    std::shared_ptr<AudioSetting> _normalize;
    std::shared_ptr<AudioSetting> _impulseResponseClip;

    struct ReverbKernel
    {
        ReverbKernel() = default;
        ReverbKernel(ReverbKernel && rh) noexcept;
        ~ReverbKernel();
        sp_conv * conv = nullptr;
        sp_ftbl * ft = nullptr;
    };
    std::vector<ReverbKernel> _kernels;  // one per impulse response channel
    std::vector<ReverbKernel> _pending_kernels; // new kernels when an impulse has been computed
    bool _swap_ready;
    std::mutex _kernel_mutex;
};

}  // namespace lab

#endif  // ConvolverNode_h
