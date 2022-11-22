// SPDX-License-Identifier: BSD-2-Clause
// Copyright (C) 2015, The LabSound Authors. All rights reserved.

#pragma once

#ifndef LABSOUND_H
#define LABSOUND_H

// WebAudio Public API
#include "LabSound/core/AudioDevice.h"
#include "LabSound/core/NullDeviceNode.h"
#include "LabSound/core/AudioHardwareDeviceNode.h"
#include "LabSound/core/AudioHardwareInputNode.h"
#include "LabSound/core/AnalyserNode.h"
#include "LabSound/core/AudioBasicInspectorNode.h"
#include "LabSound/core/AudioBasicProcessorNode.h"
#include "LabSound/core/AudioContext.h"
#include "LabSound/core/AudioListener.h"
#include "LabSound/core/AudioNodeInput.h"
#include "LabSound/core/AudioNodeOutput.h"
#include "LabSound/core/AudioScheduledSourceNode.h"
#include "LabSound/core/BiquadFilterNode.h"
#include "LabSound/core/ChannelMergerNode.h"
#include "LabSound/core/ChannelSplitterNode.h"
#include "LabSound/core/ConvolverNode.h"
#include "LabSound/core/DelayNode.h"
#include "LabSound/core/DynamicsCompressorNode.h"
#include "LabSound/core/GainNode.h"
#include "LabSound/core/OscillatorNode.h"
#include "LabSound/core/PannerNode.h"
#include "LabSound/core/SampledAudioNode.h"
#include "LabSound/core/StereoPannerNode.h"
#include "LabSound/core/WaveShaperNode.h"

// LabSound Extended Public API
#include "LabSound/extended/ADSRNode.h"
#include "LabSound/extended/AudioFileReader.h"
#include "LabSound/extended/BPMDelayNode.h"
#include "LabSound/extended/ClipNode.h"
#include "LabSound/extended/DiodeNode.h"
#include "LabSound/extended/FunctionNode.h"
#include "LabSound/extended/GranulationNode.h"
#include "LabSound/extended/NoiseNode.h"
//#include "LabSound/extended/PdNode.h"
#include "LabSound/extended/PeakCompNode.h"
#include "LabSound/extended/PingPongDelayNode.h"
#include "LabSound/extended/PolyBLEPNode.h"
#include "LabSound/extended/PowerMonitorNode.h"
#include "LabSound/extended/PWMNode.h"
#include "LabSound/extended/RealtimeAnalyser.h"
#include "LabSound/extended/Registry.h"
#include "LabSound/extended/RecorderNode.h"
#include "LabSound/extended/SfxrNode.h"
#include "LabSound/extended/SpatializationNode.h"
#include "LabSound/extended/SpectralMonitorNode.h"
#include "LabSound/extended/SupersawNode.h"

namespace lab
{
    const std::vector<AudioDeviceInfo> MakeAudioDeviceList();
    const AudioDeviceIndex GetDefaultOutputAudioDeviceIndex();
    const AudioDeviceIndex GetDefaultInputAudioDeviceIndex();

    std::unique_ptr<AudioContext> MakeRealtimeAudioContext(const AudioStreamConfig & outputConfig, const AudioStreamConfig & inputConfig);
    std::unique_ptr<AudioContext> MakeOfflineAudioContext(const AudioStreamConfig & offlineConfig, double recordTimeMilliseconds);

    struct OfflineContext
    {
        void * device;
        std::unique_ptr<AudioContext> context;
        void process(size_t samples);
    };
    OfflineContext MakeOfflineAudioContext(const AudioStreamConfig &);

    std::shared_ptr<AudioHardwareInputNode> MakeAudioHardwareInputNode(ContextRenderLock & r);

    AudioStreamConfig GetDefaultInputAudioDeviceConfiguration();
    AudioStreamConfig GetDefaultOutputAudioDeviceConfiguration();
}


#endif

