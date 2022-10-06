// SPDX-License-Identifier: BSD-2-Clause

// This code is part of the sfizz library and is licensed under a BSD 2-clause
// license. You should have receive a LICENSE.md file along with the code.
// If not, contact the sfizz maintainers at https://github.com/sfztools/sfizz

/*
   Note(jpc): implementation status

   Complete (no opcodes)
*/

#include "Limiter.h"
#include "gen/limiter.hxx"
#include "Opcode.h"
#include "AudioSpan.h"
#include "absl/memory/memory.h"

static constexpr int _oversampling = 2;

namespace sfz {
namespace fx {

    Limiter::Limiter()
        : _limiter(new faustLimiter)
    {
        _limiter->instanceResetUserInterface();
    }

    Limiter::~Limiter()
    {
    }

    void Limiter::setSampleRate(double sampleRate)
    {
        _limiter->classInit(_oversampling * sampleRate);
        _limiter->instanceConstants(_oversampling * sampleRate);

        for (unsigned c = 0; c < EffectChannels; ++c) {
            _downsampler2x[c].set_coefs(OSCoeffs2x);
            _upsampler2x[c].set_coefs(OSCoeffs2x);
        }

        clear();
    }

    void Limiter::setSamplesPerBlock(int samplesPerBlock)
    {
        _tempBuffer2x.resize(2 * samplesPerBlock);
    }

    void Limiter::clear()
    {
        _limiter->instanceClear();
    }

    void Limiter::process(const float* const inputs[], float* const outputs[], unsigned nframes)
    {
        auto inOut2x = AudioSpan<float>( _tempBuffer2x).first(2 * nframes);

        for (unsigned c = 0; c < EffectChannels; ++c)
            _upsampler2x[c].process_block(inOut2x.getSpan(c).data(), inputs[c], nframes);

        _limiter->compute(2 * nframes, inOut2x, inOut2x);

        for (unsigned c = 0; c < EffectChannels; ++c)
            _downsampler2x[c].process_block(outputs[c], inOut2x.getSpan(c).data(), nframes);
    }

    std::unique_ptr<Effect> Limiter::makeInstance(absl::Span<const Opcode> members)
    {
        Limiter* limiter = new Limiter;
        std::unique_ptr<Effect> fx { limiter };

        for (const Opcode& opc : members) {
            // no opcodes
            (void)opc;
        }

        return fx;
    }

} // namespace fx
} // namespace sfz
