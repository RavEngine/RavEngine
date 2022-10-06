// SPDX-License-Identifier: BSD-2-Clause

// This code is part of the sfizz library and is licensed under a BSD 2-clause
// license. You should have receive a LICENSE.md file along with the code.
// If not, contact the sfizz maintainers at https://github.com/sfztools/sfizz

/*
   Note(jpc): implementation status

- [x] disto_tone
- [ ] disto_tone_oncc
- [x] disto_depth
- [ ] disto_depth_oncc
- [x] disto_stages
- [x] disto_dry
- [ ] disto_dry_oncc
- [x] disto_wet
- [ ] disto_wet_oncc
*/

#include "Disto.h"
#include "gen/disto_stage.hxx"
#include "Opcode.h"
#include "Config.h"
#include "MathHelpers.h"
#include "OversamplerHelpers.h"
#include <absl/types/span.h>
#include <cmath>

static constexpr int _oversampling = 8;

namespace sfz {
namespace fx {

struct Disto::Impl {
    enum { maxStages = 4 };

    float _samplePeriod { 1.0f / config::defaultSampleRate };
    float _tone { Default::distoTone };
    float _depth { Default::distoDepth };
    float _dry { Default::effect };
    float _wet { Default::effect };
    unsigned _numStages = { Default::distoStages };

    float _toneLpfMem[EffectChannels] = {};
    faustDisto _stages[EffectChannels][Default::maxDistoStages];

    sfz::Upsampler _upsampler[EffectChannels];
    sfz::Downsampler _downsampler[EffectChannels];
    std::unique_ptr<float[]> _temp[2];

    // use the same formula as reverb
    float toneCutoff() const noexcept
    {
        float mk = 21.0f + _tone * 1.08f;
        return 440.0f * std::exp2((mk - 69.0f) * (1.0f / 12.0f));
    }
};

Disto::Disto()
    : _impl(new Impl)
{
    Impl& impl = *_impl;

    for (unsigned c = 0; c < EffectChannels; ++c) {
        for (faustDisto& stage : impl._stages[c])
            stage.init(_oversampling * config::defaultSampleRate);
    }
}

Disto::~Disto()
{
}

void Disto::setSampleRate(double sampleRate)
{
    Impl& impl = *_impl;
    impl._samplePeriod = 1.0 / sampleRate;

    for (unsigned c = 0; c < EffectChannels; ++c) {
        for (faustDisto& stage : impl._stages[c]) {
            stage.classInit(_oversampling * sampleRate);
            stage.instanceConstants(_oversampling * sampleRate);
        }
    }

    clear();
}

void Disto::setSamplesPerBlock(int samplesPerBlock)
{
    Impl& impl = *_impl;

    for (std::unique_ptr<float[]>& temp : impl._temp)
        temp.reset(new float[_oversampling * samplesPerBlock]);
}

void Disto::clear()
{
    Impl& impl = *_impl;
    for (unsigned c = 0; c < EffectChannels; ++c) {
        for (faustDisto& stage : impl._stages[c])
            stage.instanceClear();
    }

    for (unsigned c = 0; c < EffectChannels; ++c) {
        impl._toneLpfMem[c] = 0.0f;
        impl._downsampler[c].clear();
        impl._upsampler[c].clear();
    }
}

void Disto::process(const float* const inputs[], float* const outputs[], unsigned nframes)
{
    // Note(jpc): assumes `inputs` and `outputs` to be different buffers

    Impl& impl = *_impl;
    const float dry = impl._dry;
    const float wet = impl._wet;
    const float depth = impl._depth;
    const float toneLpfPole = std::exp(float(-2.0 * M_PI) * impl.toneCutoff() * impl._samplePeriod);

    for (unsigned c = 0; c < EffectChannels; ++c) {
        // compute LPF
        absl::Span<const float> channelIn(inputs[c], nframes);
        absl::Span<float> lpfOut(outputs[c], nframes);
        float lpfMem = impl._toneLpfMem[c];
        for (unsigned i = 0; i < nframes; ++i) {
            // Note(jpc) apply `dry` gain, note there is no output if
            //           `dry=0 wet=<any>`, it is the same behavior as reference
            lpfMem = channelIn[i] * dry * (1.0f - toneLpfPole) + lpfMem * toneLpfPole;
            lpfOut[i] = lpfMem;
        }
        impl._toneLpfMem[c] = lpfMem;

        // upsample
        absl::Span<float> temp[2] = {
            absl::Span<float>(impl._temp[0].get(), _oversampling * nframes),
            absl::Span<float>(impl._temp[1].get(), _oversampling * nframes),
        };
        impl._upsampler[c].process(_oversampling, lpfOut.data(), temp[0].data(), nframes, temp[1].data(), static_cast<int>(temp[1].size()));
        absl::Span<float> upsamplerOut = temp[0];

        // run disto stages
        absl::Span<float> stageInOut = upsamplerOut;
        for (unsigned s = 0, numStages = impl._numStages; s < numStages; ++s) {
            // set depth parameter (TODO modulation)
            impl._stages[c][s].setDepth(depth);
            //
            float *faustIn[] = { stageInOut.data() };
            float *faustOut[] = { stageInOut.data() };
            impl._stages[c][s].compute(_oversampling * nframes, faustIn, faustOut);
        }

        // downsample
        impl._downsampler[c].process(_oversampling, stageInOut.data(), outputs[c], nframes, temp[1].data(), static_cast<int>(temp[1].size()));

        // dry/wet mix
        absl::Span<float> mixOut(outputs[c], nframes);
        for (unsigned i = 0; i < nframes; ++i)
            mixOut[i] = mixOut[i] * wet + channelIn[i] * (1.0f - wet);
    }
}

std::unique_ptr<Effect> Disto::makeInstance(absl::Span<const Opcode> members)
{
    Disto* disto = new Disto;
    std::unique_ptr<Effect> fx { disto };

    Impl& impl = *disto->_impl;

    for (const Opcode& opc : members) {
        switch (opc.lettersOnlyHash) {
        case hash("disto_tone"):
            impl._tone = opc.read(Default::distoTone);
            break;
        case hash("disto_depth"):
            impl._depth = opc.read(Default::distoDepth);
            break;
        case hash("disto_stages"):
            impl._numStages = opc.read(Default::distoStages);
            break;
        case hash("disto_dry"):
            impl._dry = opc.read(Default::effect);
            break;
        case hash("disto_wet"):
            impl._wet = opc.read(Default::effect);
            break;
        }
    }

    return fx;
}
} // namespace sfz
} // namespace fx
