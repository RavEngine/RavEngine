// SPDX-License-Identifier: BSD-2-Clause

// This code is part of the sfizz library and is licensed under a BSD 2-clause
// license. You should have receive a LICENSE.md file along with the code.
// If not, contact the sfizz maintainers at https://github.com/sfztools/sfizz

/**
   Note(jpc): implementation status

- [x] bitred
- [ ] bitred_oncc
- [ ] bitred_smoothcc
- [ ] bitred_stepcc
- [ ] bitred_curvecc

- [x] decim
- [ ] decim_oncc
- [ ] decim_smoothcc
- [ ] decim_stepcc
- [ ] decim_curvecc

- [ ] egN_bitred
- [ ] egN_bitred_oncc
- [ ] lfoN_bitred
- [ ] lfoN_bitred_oncc
- [ ] lfoN_bitred_smoothcc
- [ ] lfoN_bitred_stepcc

- [ ] egN_decim
- [ ] egN_decim_oncc
- [ ] lfoN_decim
- [ ] lfoN_decim_oncc
- [ ] lfoN_decim_smoothcc
- [ ] lfoN_decim_stepcc

 */

#include "Lofi.h"
#include "Opcode.h"
#include "absl/memory/memory.h"
#include <algorithm>
#include <cstring>
#include <cmath>

namespace sfz {
namespace fx {

    void Lofi::setSampleRate(double sampleRate)
    {
        for (unsigned c = 0; c < EffectChannels; ++c) {
            _bitred[c].init(sampleRate);
            _decim[c].init(sampleRate);
        }
    }

    void Lofi::setSamplesPerBlock(int samplesPerBlock)
    {
        (void)samplesPerBlock;
    }

    void Lofi::clear()
    {
        for (unsigned c = 0; c < EffectChannels; ++c) {
            _bitred[c].clear();
            _decim[c].clear();
        }
    }

    void Lofi::process(const float* const inputs[2], float* const outputs[2], unsigned nframes)
    {
        for (unsigned c = 0; c < EffectChannels; ++c) {
            _bitred[c].setDepth(_bitred_depth);
            _bitred[c].process(inputs[c], outputs[c], nframes);

            _decim[c].setDepth(_decim_depth);
            _decim[c].process(outputs[c], outputs[c], nframes);
        }
    }

    std::unique_ptr<Effect> Lofi::makeInstance(absl::Span<const Opcode> members)
    {
        Lofi* lofi = new Lofi;
        std::unique_ptr<Effect> fx { lofi };

        for (const Opcode& opcode : members) {
            switch (opcode.lettersOnlyHash) {
            case hash("bitred"):
                lofi->_bitred_depth = opcode.read(Default::lofiBitred);
                break;
            case hash("decim"):
                lofi->_decim_depth = opcode.read(Default::lofiDecim);
                break;
            }
        }

        return fx;
    }

    ///
    void Lofi::Bitred::init(double sampleRate)
    {
        (void)sampleRate;

        fDownsampler2x.set_coefs(OSCoeffs2x);
    }

    void Lofi::Bitred::clear()
    {
        fLastValue = 0.0;
        fDownsampler2x.clear_buffers();
    }

    void Lofi::Bitred::setDepth(float depth)
    {
        fDepth = clamp(depth, 0.0f, 100.0f);
    }

    void Lofi::Bitred::process(const float* in, float* out, uint32_t nframes)
    {
        if (fDepth == 0) {
            if (in != out)
                std::memcpy(out, in, nframes * sizeof(float));
            clear();
            return;
        }

        float lastValue = fLastValue;

        const float steps = (1.0f + (100.0f - fDepth)) * 0.75f;
        const float invSteps = 1.0f / steps;

        for (uint32_t i = 0; i < nframes; ++i) {
            float x = in[i];

            float y = std::copysign((int)(0.5f + std::fabs(x * steps)), x) * invSteps; // NOLINT

            float y2x[2];
            y2x[0] = (y != lastValue) ? (0.5f * (y + lastValue)) : y;
            y2x[1] = y;

            lastValue = y;

            y = fDownsampler2x.process_sample(y2x);
            out[i] = y;
        }

        fLastValue = lastValue;
    }

    ///
    void Lofi::Decim::init(double sampleRate)
    {
        fSampleTime = 1.0f / static_cast<float>(sampleRate);

        fDownsampler2x.set_coefs(OSCoeffs2x);
    }

    void Lofi::Decim::clear()
    {
        fPhase = 0.0;
        fLastValue = 0.0;
        fDownsampler2x.clear_buffers();
    }

    void Lofi::Decim::setDepth(float depth)
    {
        fDepth = clamp(depth, 0.0f, 100.0f);
    }

    void Lofi::Decim::process(const float* in, float* out, uint32_t nframes)
    {
        if (fDepth == 0) {
            if (in != out)
                std::memcpy(out, in, nframes * sizeof(float));
            clear();
            return;
        }

        const float dt = [this]() {
            // exponential curve fit
            const float a = 1.289079e+00, b = 1.384141e-01, c = 1.313298e-04;
            const float denom = std::pow(a, b * fDepth) * c - c;
            return fSampleTime / denom;
        }();

        float phase = fPhase;
        float lastValue = fLastValue;

        for (uint32_t i = 0; i < nframes; ++i) {
            float x = in[i];

            phase += dt;
            float y = (phase > 1.0f) ? x : lastValue;
            phase -= static_cast<float>(static_cast<int>(phase));

            float y2x[2];
            y2x[0] = (y != lastValue) ? (0.5f * (y + lastValue)) : y;
            y2x[1] = y;

            lastValue = y;

            y = fDownsampler2x.process_sample(y2x);
            out[i] = y;
        }

        fPhase = phase;
        fLastValue = lastValue;
    }
} // namespace fx
} // namespace sfz
