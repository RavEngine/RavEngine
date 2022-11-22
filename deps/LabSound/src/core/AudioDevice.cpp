// SPDX-License-Identifier: BSD-2-Clause
// Copyright (C) 2015+, The LabSound Authors. All rights reserved.

#include "LabSound/core/AudioDevice.h"
#include "LabSound/core/AudioBus.h"
#include "LabSound/core/AudioContext.h"
#include "LabSound/core/AudioNodeInput.h"
#include "LabSound/core/AudioSourceProvider.h"
#include "LabSound/extended/AudioContextLock.h"

#include "internal/Assertions.h"
#include "internal/DenormalDisabler.h"

// Non platform-specific helper functions

using namespace lab;

void lab::pull_graph(AudioContext * ctx, AudioNodeInput * required_inlet, AudioBus * src, AudioBus * dst, int frames,
                     const SamplingInfo & info, AudioHardwareInput * optional_hardware_input)
{
    // The audio system might still be invoking callbacks during shutdown, so bail out if so.
    if (!ctx) 
        return;

    // bail if shutting down.
    auto ac = ctx->audioContextInterface().lock();
    if (!ac)
        return;

    ASSERT(required_inlet);

    ContextRenderLock renderLock(ctx, "lab::pull_graph");
    if (!renderLock.context()) return;  // return if couldn't acquire lock

    if (!ctx->isInitialized())
    {
        dst->zero();
        return;
    }

    // Denormals can slow down audio processing.
    // Use an RAII object to protect all AudioNodes processed within this scope.

    /// @TODO under what circumstance do they arise?
    /// If they come from input data such as loaded WAV files, they should be corrected
    /// at source. If they can result from signal processing; again, where? The
    /// signal processing should not produce denormalized values.

    DenormalDisabler denormalDisabler;

    // Let the context take care of any business at the start of each render quantum.
    ctx->handlePreRenderTasks(renderLock);

    // Prepare the local audio input provider for this render quantum.
    if (optional_hardware_input && src)
    {
        optional_hardware_input->set(src);
    }

    // process the graph by pulling the inputs, which will recurse the entire processing graph.
    AudioBus * renderedBus = required_inlet->pull(renderLock, dst, frames);

    if (!renderedBus)
    {
        dst->zero();
    }
    else if (renderedBus != dst)
    {
        // in-place processing was not possible - so copy
        dst->copyFrom(*renderedBus);
    }

    // Process nodes which need extra help because they are not connected to anything,
    // but still have work to do
    ctx->processAutomaticPullNodes(renderLock, frames);

    // Let the context take care of any business at the end of each render quantum.
    ctx->handlePostRenderTasks(renderLock);
}
