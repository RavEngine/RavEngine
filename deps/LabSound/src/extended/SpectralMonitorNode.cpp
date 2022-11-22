// SPDX-License-Identifier: BSD-2-Clause
// Copyright (C) 2015+, The LabSound Authors. All rights reserved.

#include "LabSound/core/AudioBus.h"
#include "LabSound/core/AudioNodeInput.h"
#include "LabSound/core/AudioNodeOutput.h"
#include "LabSound/core/AudioSetting.h"
#include "LabSound/core/Macros.h"
#include "LabSound/core/WindowFunctions.h"

#include "LabSound/extended/SpectralMonitorNode.h"
#include "LabSound/extended/Registry.h"

#include <cmath>
#include <ooura/fftsg.h>

namespace lab
{

using namespace lab;

//////////////////////////////////////////////////////
// Prviate FFT + SpectralMonitorNode Implementation //
//////////////////////////////////////////////////////

// FFT class directly inspired by that in Cinder Audio 2.
struct FFT
{
public:
    FFT(int size)
        : size(size)
    {
        oouraIp = (int *) calloc(2 + (int) sqrt(size / 2), sizeof(int));
        oouraW = (float *) calloc(size / 2, sizeof(float));
    }

    ~FFT()
    {
        free(oouraIp);
        free(oouraW);
    }

    // does an in place transform of waveform to real and imag.
    // real values are on even, imag on odd
    void forward(std::vector<float> & waveform)
    {
        ooura::rdft(static_cast<int>(size), 1, &waveform[0], oouraIp, oouraW);
    }

    int size;
    int * oouraIp;
    float * oouraW;
};

class SpectralMonitorNode::SpectralMonitorNodeInternal
{
public:
    SpectralMonitorNodeInternal()
        : windowSize(std::make_shared<AudioSetting>("windowSize", "WNDW", AudioSetting::Type::Integer))
        , fft(nullptr)
    {
        setWindowSize(512);
    }

    ~SpectralMonitorNodeInternal()
    {
        delete fft;
    }

    void setWindowSize(int s)
    {
        cursor = 0;
        windowSize->setUint32(static_cast<int>(s));

        buffer.resize(s);

        for (int i = 0; i < s; ++i)
        {
            buffer[i] = 0;
        }

        delete fft;
        fft = new FFT(s);
    }

    float _db;

    int cursor;

    std::vector<float> buffer;
    std::recursive_mutex magMutex;

    std::shared_ptr<AudioSetting> windowSize;

    FFT * fft;
};

////////////////////////////////
// Public SpectralMonitorNode //
////////////////////////////////

SpectralMonitorNode::SpectralMonitorNode(AudioContext & ac)
    : AudioBasicInspectorNode(ac, 2)
    , internalNode(new SpectralMonitorNodeInternal())
{
    m_settings.push_back(internalNode->windowSize);
    initialize();
}

SpectralMonitorNode::~SpectralMonitorNode()
{
    uninitialize();
    delete internalNode;
}

void SpectralMonitorNode::process(ContextRenderLock &r, int bufferSize)
{
    // deal with the output in case the power monitor node is embedded in a signal chain for some reason.
    // It's merely a pass through though.

    AudioBus * outputBus = output(0)->bus(r);

    if (!isInitialized() || !input(0)->isConnected())
    {
        if (outputBus)
            outputBus->zero();
        return;
    }

    AudioBus * bus = input(0)->bus(r);
    bool isBusGood = bus && bus->numberOfChannels() > 0 && bus->channel(0)->length() >= bufferSize;
    if (!isBusGood)
    {
        outputBus->zero();
        return;
    }

    // specific to this node
    {
        std::vector<const float *> channels;
        int numberOfChannels = bus->numberOfChannels();
        for (int c = 0; c < numberOfChannels; ++c)
            channels.push_back(bus->channel(c)->data());

        int sz = internalNode->windowSize->valueUint32();

        // if the fft is smaller than the quantum, just grab a chunk
        if (sz < bufferSize)
        {
            internalNode->cursor = 0;
            bufferSize = internalNode->windowSize->valueUint32();
        }

        // if the quantum overlaps the end of the window, just fill up the buffer
        if (internalNode->cursor + bufferSize > sz)
            bufferSize = sz - internalNode->cursor;

        {
            std::lock_guard<std::recursive_mutex> lock(internalNode->magMutex);

            internalNode->buffer.resize(sz);

            for (int i = 0; i < bufferSize; ++i)
            {
                internalNode->buffer[i + internalNode->cursor] = 0;
            }

            for (int c = 0; c < numberOfChannels; ++c)
            {
                for (int i = 0; i < bufferSize; ++i)
                {
                    float p = channels[c][i];
                    internalNode->buffer[i + internalNode->cursor] += p;
                }
            }
        }

        // advance the cursor
        internalNode->cursor += bufferSize;
        if (internalNode->cursor >= static_cast<int>(internalNode->windowSize->valueUint32()))
            internalNode->cursor = 0;
    }
    // to here

    if (bus != outputBus)
        outputBus->copyFrom(*bus);
}

void SpectralMonitorNode::reset(ContextRenderLock &)
{
    internalNode->setWindowSize(internalNode->windowSize->valueUint32());
}

void SpectralMonitorNode::spectralMag(std::vector<float> & result)
{
    std::vector<float> window;

    {
        std::lock_guard<std::recursive_mutex> lock(internalNode->magMutex);
        window.swap(internalNode->buffer);
        internalNode->setWindowSize(internalNode->windowSize->valueUint32());
    }

    // http://www.ni.com/white-paper/4844/en/
    ApplyWindowFunctionInplace(WindowFunction::blackman, window.data(), static_cast<int>(window.size()));
    internalNode->fft->forward(window);

    // similar to cinder audio2 Scope object, although Scope smooths spectral samples frame by frame
    // remove nyquist component - the first imaginary component
    window[1] = 0.0f;

    // compute normalized magnitude spectrum
    /// @TODO @tofix - break this into vector Cartesian -> polar and then vector lowpass. skip lowpass if smoothing factor is very small
    const float kMagScale = 1.0f;  /// detail->windowSize;
    for (int i = 0; i < window.size(); i += 2)
    {
        float re = window[i];
        float im = window[i + 1];
        window[i / 2] = sqrt(re * re + im * im) * kMagScale;
    }

    result.swap(window);
}

void SpectralMonitorNode::windowSize(unsigned int ws)
{
    internalNode->setWindowSize(ws);
}

unsigned int SpectralMonitorNode::windowSize() const
{
    return internalNode->windowSize->valueUint32();
}

}  // namespace lab
