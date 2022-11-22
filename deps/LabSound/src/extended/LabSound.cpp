// SPDX-License-Identifier: BSD-2-Clause
// Copyright (C) 2015+, The LabSound Authors. All rights reserved.

#include "LabSound/LabSound.h"
#include "LabSound/core/AudioContext.h"
#include "LabSound/core/AudioHardwareDeviceNode.h"

#include "LabSound/extended/AudioContextLock.h"
#include "LabSound/extended/Logging.h"
#include "LabSound/extended/Registry.h"

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <time.h>

#include <atomic>
#include <chrono>
#include <iostream>
#include <memory>
#include <mutex>
#include <thread>


#if defined(_MSC_VER)
#pragma warning(disable:4996)
#endif

namespace lab
{
const std::vector<AudioDeviceInfo> MakeAudioDeviceList()
{
    LOG_TRACE("MakeAudioDeviceList()");
    return AudioDevice::MakeAudioDeviceList();
}

const AudioDeviceIndex GetDefaultOutputAudioDeviceIndex()
{
    LOG_TRACE("GetDefaultOutputAudioDeviceIndex()");
    return AudioDevice::GetDefaultOutputAudioDeviceIndex();
}

const AudioDeviceIndex GetDefaultInputAudioDeviceIndex()
{
    LOG_TRACE("GetDefaultInputAudioDeviceIndex()");
    return AudioDevice::GetDefaultInputAudioDeviceIndex();
}

std::unique_ptr<lab::AudioContext> MakeRealtimeAudioContext(
    const AudioStreamConfig & outputConfig, const AudioStreamConfig & inputConfig)
{
    LOG_TRACE("MakeRealtimeAudioContext()");

    std::unique_ptr<AudioContext> ctx(new lab::AudioContext(false));
    ctx->setDeviceNode(std::make_shared<lab::AudioHardwareDeviceNode>(*ctx.get(), outputConfig, inputConfig));
    ctx->lazyInitialize();
    return ctx;
}

std::unique_ptr<lab::AudioContext> MakeOfflineAudioContext(
    const AudioStreamConfig & offlineConfig, double recordTimeMilliseconds)
{
    LOG_TRACE("MakeOfflineAudioContext(duration)");

    const double secondsToRun = recordTimeMilliseconds * 0.001;

    std::unique_ptr<AudioContext> ctx(new lab::AudioContext(true));
    ctx->setDeviceNode(std::make_shared<lab::NullDeviceNode>(*ctx.get(), offlineConfig, secondsToRun));
    ctx->lazyInitialize();
    return ctx;
}

OfflineContext MakeOfflineAudioContext(const AudioStreamConfig & offlineConfig)
{
    LOG_TRACE("MakeOfflineAudioContext()");
    OfflineContext ret;
    ret.context = std::make_unique<lab::AudioContext>(true);

    auto dev = std::make_shared<lab::NullDeviceNode>(
        *ret.context.get(),
        offlineConfig, 
        std::numeric_limits<double>::max());
    ret.context->setDeviceNode(dev);
    ret.context->lazyInitialize();
    ret.device = dev.get();
    return ret;
}

void OfflineContext::process(size_t samples)
{
    if (!device || !samples)
        return;

    lab::NullDeviceNode * dev = reinterpret_cast<lab::NullDeviceNode *>(device);
    dev->offlineRenderFrames(samples);
}


std::shared_ptr<AudioHardwareInputNode> MakeAudioHardwareInputNode(ContextRenderLock & r)
{
    LOG_TRACE("MakeAudioHardwareInputNode()");

    auto device = r.context()->device();

    if (device)
    {
        if (auto * hardwareDevice = dynamic_cast<AudioHardwareDeviceNode *>(device.get()))
        {
            std::shared_ptr<AudioHardwareInputNode> inputNode( 
                new AudioHardwareInputNode(*r.context(), hardwareDevice->AudioHardwareInputProvider()));
            return inputNode;
        }
        else
        {
            throw std::runtime_error("Cannot create AudioHardwareInputNode. Context does not own an AudioHardwareInputNode.");
        }
    }
    return {};
}

AudioStreamConfig GetDefaultInputAudioDeviceConfiguration()
{
    AudioStreamConfig inputConfig;

    const std::vector<AudioDeviceInfo> audioDevices = lab::MakeAudioDeviceList();
    /*const AudioDeviceIndex default_output_device =*/ lab::GetDefaultOutputAudioDeviceIndex();
    const AudioDeviceIndex default_input_device = lab::GetDefaultInputAudioDeviceIndex();

    AudioDeviceInfo defaultInputInfo;
    for (auto& info : audioDevices)
    {
        if (info.index == default_input_device.index)
            defaultInputInfo = info;
    }

    if (defaultInputInfo.index == -1)
        throw std::invalid_argument("the default audio input device was requested but none were found");

    inputConfig.device_index = defaultInputInfo.index;
    inputConfig.desired_channels = std::min(uint32_t(1), defaultInputInfo.num_input_channels);
    inputConfig.desired_samplerate = defaultInputInfo.nominal_samplerate;
    return inputConfig;
}

AudioStreamConfig GetDefaultOutputAudioDeviceConfiguration()
{
    AudioStreamConfig outputConfig;

    const std::vector<AudioDeviceInfo> audioDevices = lab::MakeAudioDeviceList();
    const AudioDeviceIndex default_output_device = lab::GetDefaultOutputAudioDeviceIndex();
    const AudioDeviceIndex default_input_device = lab::GetDefaultInputAudioDeviceIndex();

    AudioDeviceInfo defaultOutputInfo;
    for (auto& info : audioDevices)
    {
        if (info.index == default_output_device.index) 
            defaultOutputInfo = info;
    }

    if (defaultOutputInfo.index == -1)
        throw std::invalid_argument("the default audio output device was requested but none were found");

    outputConfig.device_index = defaultOutputInfo.index;
    outputConfig.desired_channels = std::min(uint32_t(2), defaultOutputInfo.num_output_channels);
    outputConfig.desired_samplerate = defaultOutputInfo.nominal_samplerate;
    return outputConfig;
}


}  // lab

///////////////////////
// Logging Utilities //
///////////////////////

/*
 * Copyright (c) 2017 rxi
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to
 * deal in the Software without restriction, including without limitation the
 * rights to use, copy, modify, merge, publish, distribute, sublicense, and/or
 * sell copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
 * IN THE SOFTWARE.
 */

#define LOG_USE_COLOR

static struct {
  void *udata;
  log_LockFn lock;
  FILE *fp;
  int level;
  int quiet;
} L;


static const char *level_names[] = {
  "TRACE", "DEBUG", "INFO", "WARN", "ERROR", "FATAL"
};

#ifdef LOG_USE_COLOR
static const char *level_colors[] = {
  "\x1b[94m", "\x1b[36m", "\x1b[32m", "\x1b[33m", "\x1b[31m", "\x1b[35m"
};
#endif


static void lock(void)   {
  if (L.lock) {
    L.lock(L.udata, 1);
  }
}


static void unlock(void) {
  if (L.lock) {
    L.lock(L.udata, 0);
  }
}


void log_set_udata(void *udata) {
  L.udata = udata;
}


void log_set_lock(log_LockFn fn) {
  L.lock = fn;
}


void log_set_fp(FILE *fp) {
  L.fp = fp;
}


void log_set_level(int level) {
  L.level = level;
}


void log_set_quiet(int enable) {
  L.quiet = enable ? 1 : 0;
}


void LabSoundLog(int level, const char *file, int line, const char *fmt, ...) {
  if (level < L.level) {
    return;
  }

  /* Acquire lock */
  lock();

  /* Get current time */
  time_t t = time(NULL);
  struct tm *lt = localtime(&t);

  /* Log to stderr */
  if (!L.quiet) {
    va_list args;
    char buf[16];
    buf[strftime(buf, sizeof(buf), "%H:%M:%S", lt)] = '\0';
#ifdef LOG_USE_COLOR
    fprintf(
      stderr, "%s %s%-5s\x1b[0m \x1b[90m%s:%d:\x1b[0m ",
      buf, level_colors[level], level_names[level], file, line);
#else
    fprintf(stderr, "%s %-5s %s:%d: ", buf, level_names[level], file, line);
#endif
    va_start(args, fmt);
    vfprintf(stderr, fmt, args);
    va_end(args);
    fprintf(stderr, "\n");
    fflush(stderr);
  }

  /* Log to file */
  if (L.fp) {
    va_list args;
    char buf[32];
    buf[strftime(buf, sizeof(buf), "%Y-%m-%d %H:%M:%S", lt)] = '\0';
    fprintf(L.fp, "%s %-5s %s:%d: ", buf, level_names[level], file, line);
    va_start(args, fmt);
    vfprintf(L.fp, fmt, args);
    va_end(args);
    fprintf(L.fp, "\n");
    fflush(L.fp);
  }

  /* Release lock */
  unlock();
}

void LabSoundAssertLog(const char * file_, int line, const char * function_, const char * assertion_)
{
    const char * file = file_ ? file_ : "Unknown source file";
    const char * function = function_ ? function_ : "";
    const char * assertion = assertion_ ? assertion_ : "Assertion failed";
    printf("Assertion: %s:%s:%d - %s\n", function, file, line, assertion);
}

std::once_flag register_all_flag;
void LabSoundRegistryInit(lab::NodeRegistry& reg)
{
    using namespace lab;
    
    std::call_once(register_all_flag, [&reg]() {
        
        // core
        
        reg.Register(AnalyserNode::static_name(),
            [](AudioContext& ac)->AudioNode* { return new AnalyserNode(ac); },
            [](AudioNode* n) { delete n; });

        reg.Register(AudioHardwareDeviceNode::static_name(),
            [](AudioContext& ac)->AudioNode* { return nullptr; },
            [](AudioNode* n) { delete n; });
        
        reg.Register(AudioHardwareInputNode::static_name(),
            [](AudioContext& ac)->AudioNode* { return nullptr; },
            [](AudioNode* n) { delete n; });
        
        reg.Register(BiquadFilterNode::static_name(),
            [](AudioContext& ac)->AudioNode* { return new BiquadFilterNode(ac); },
            [](AudioNode* n) { delete n; });
        
        reg.Register(ChannelMergerNode::static_name(),
            [](AudioContext& ac)->AudioNode* { return new ChannelMergerNode(ac); },
            [](AudioNode* n) { delete n; });
        
        reg.Register(ChannelSplitterNode::static_name(),
            [](AudioContext& ac)->AudioNode* { return new ChannelSplitterNode(ac); },
            [](AudioNode* n) { delete n; });
        
        reg.Register(ConvolverNode::static_name(),
            [](AudioContext& ac)->AudioNode* { return new ConvolverNode(ac); },
            [](AudioNode* n) { delete n; });
        
        reg.Register(DelayNode::static_name(),
            [](AudioContext& ac)->AudioNode* { return new DelayNode(ac); },
            [](AudioNode* n) { delete n; });
        
        reg.Register(DynamicsCompressorNode::static_name(),
            [](AudioContext& ac)->AudioNode* { return new DynamicsCompressorNode(ac); },
            [](AudioNode* n) { delete n; });
        
        reg.Register("GainNode",
            [](AudioContext& ac)->AudioNode* { return new GainNode(ac); },
            [](AudioNode* n) { delete n; });
        
        reg.Register(NullDeviceNode::static_name(),
            [](AudioContext& ac)->AudioNode* { return nullptr; },
            [](AudioNode* n) { delete n; });
        
        reg.Register(OscillatorNode::static_name(),
            [](AudioContext& ac)->AudioNode* { return new OscillatorNode(ac); },
            [](AudioNode* n) { delete n; });
        
        reg.Register(PannerNode::static_name(),
            [](AudioContext& ac)->AudioNode* { return new PannerNode(ac); },
            [](AudioNode* n) { delete n; });
        
        reg.Register(SampledAudioNode::static_name(),
            [](AudioContext& ac)->AudioNode* { return new SampledAudioNode(ac); },
            [](AudioNode* n) { delete n; });
        
        reg.Register(StereoPannerNode::static_name(),
            [](AudioContext& ac)->AudioNode* { return new StereoPannerNode(ac); },
            [](AudioNode* n) { delete n; });
        
        reg.Register(WaveShaperNode::static_name(),
            [](AudioContext& ac)->AudioNode* { return new WaveShaperNode(ac); },
            [](AudioNode* n) { delete n; });
        
        // extended
        
        reg.Register(ADSRNode::static_name(),
           [](AudioContext& ac)->AudioNode* { return new ADSRNode(ac); },
           [](AudioNode* n) { delete n; });
        
        reg.Register(ClipNode::static_name(),
            [](AudioContext& ac)->AudioNode* { return new ClipNode(ac); },
            [](AudioNode* n) { delete n; });
        
        reg.Register(DiodeNode::static_name(),
            [](AudioContext& ac)->AudioNode* { return new DiodeNode(ac); },
            [](AudioNode* n) { delete n; });
        
        reg.Register(FunctionNode::static_name(),
            [](AudioContext& ac)->AudioNode* { return new FunctionNode(ac); },
            [](AudioNode* n) { delete n; });
        
        reg.Register(GranulationNode::static_name(),
            [](AudioContext& ac)->AudioNode* { return new GranulationNode(ac); },
            [](AudioNode* n) { delete n; });
        
        reg.Register(NoiseNode::static_name(),
           [](AudioContext& ac)->AudioNode* { return new NoiseNode(ac); },
           [](AudioNode* n) { delete n; });
        
        reg.Register(PWMNode::static_name(),
            [](AudioContext& ac)->AudioNode* { return new PWMNode(ac); },
            [](AudioNode* n) { delete n; });
        
        //reg.Register(PdNode::static_name());
        
        reg.Register(PolyBLEPNode::static_name(),
            [](AudioContext& ac)->AudioNode* { return new PolyBLEPNode(ac); },
            [](AudioNode* n) { delete n; });
        
        reg.Register(PowerMonitorNode::static_name(),
            [](AudioContext& ac)->AudioNode* { return new PowerMonitorNode(ac); },
            [](AudioNode* n) { delete n; });
        
        reg.Register(RecorderNode::static_name(),
            [](AudioContext& ac)->AudioNode* { return new RecorderNode(ac); },
            [](AudioNode* n) { delete n; });
        
        reg.Register(SfxrNode::static_name(),
            [](AudioContext& ac)->AudioNode* { return new SfxrNode(ac); },
            [](AudioNode* n) { delete n; });
        
        reg.Register(SpectralMonitorNode::static_name(),
            [](AudioContext& ac)->AudioNode* { return new SpectralMonitorNode(ac); },
            [](AudioNode* n) { delete n; });
        
        reg.Register(SupersawNode::static_name(),
            [](AudioContext& ac)->AudioNode* { return new SupersawNode(ac); },
            [](AudioNode* n) { delete n; });
    });
}

