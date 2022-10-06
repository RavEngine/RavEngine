// SPDX-License-Identifier: BSD-2-Clause

// This code is part of the sfizz library and is licensed under a BSD 2-clause
// license. You should have receive a LICENSE.md file along with the code.
// If not, contact the sfizz maintainers at https://github.com/sfztools/sfizz

/*
   Note(jpc): implementation status

- [x] comp_gain           Gain (dB)
- [x] comp_attack         Attack time (s)
- [x] comp_release        Release time (s)
- [x] comp_ratio          Ratio (linear gain)
- [x] comp_threshold      Threshold (dB)
- [x] comp_stlink         Stereo link (boolean)

*/

#include "Compressor.h"
#include "gen/compressor.hxx"
#include "Opcode.h"
#include "AudioSpan.h"
#include "MathHelpers.h"
#include "OversamplerHelpers.h"
#include "absl/memory/memory.h"

static constexpr int _oversampling = 2;

namespace sfz {
namespace fx {

    struct Compressor::Impl {
        faustCompressor _compressor[2];
        bool _stlink { Default::compSTLink };
        float _inputGain { Default::compGain };
        AudioBuffer<float, 2> _tempBuffer2x { 2, _oversampling * config::defaultSamplesPerBlock };
        AudioBuffer<float, 2> _gain2x { 2, _oversampling * config::defaultSamplesPerBlock };
        hiir::Downsampler2x<12> _downsampler2x[EffectChannels];
        hiir::Upsampler2x<12> _upsampler2x[EffectChannels];
    };

    Compressor::Compressor()
        : _impl(new Impl)
    {
        Impl& impl = *_impl;
        for (faustCompressor& comp : impl._compressor)
            comp.instanceResetUserInterface();
    }

    Compressor::~Compressor()
    {
    }

    void Compressor::setSampleRate(double sampleRate)
    {
        Impl& impl = *_impl;
        for (faustCompressor& comp : impl._compressor) {
            comp.classInit(_oversampling * sampleRate);
            comp.instanceConstants(_oversampling * sampleRate);
        }

        for (unsigned c = 0; c < EffectChannels; ++c) {
            impl._downsampler2x[c].set_coefs(OSCoeffs2x);
            impl._upsampler2x[c].set_coefs(OSCoeffs2x);
        }

        clear();
    }

    void Compressor::setSamplesPerBlock(int samplesPerBlock)
    {
        Impl& impl = *_impl;
        impl._tempBuffer2x.resize(_oversampling * samplesPerBlock);
        impl._gain2x.resize(_oversampling * samplesPerBlock);
    }

    void Compressor::clear()
    {
        Impl& impl = *_impl;
        for (faustCompressor& comp : impl._compressor)
            comp.instanceClear();
    }

    void Compressor::process(const float* const inputs[], float* const outputs[], unsigned nframes)
    {
        Impl& impl = *_impl;
        auto inOut2x = AudioSpan<float>(impl._tempBuffer2x).first(_oversampling * nframes);

        absl::Span<float> left2x = inOut2x.getSpan(0);
        absl::Span<float> right2x = inOut2x.getSpan(1);

        impl._upsampler2x[0].process_block(left2x.data(), inputs[0], nframes);
        impl._upsampler2x[1].process_block(right2x.data(), inputs[1], nframes);

        const float inputGain = impl._inputGain;
        for (unsigned i = 0; i < _oversampling * nframes; ++i) {
            left2x[i] *= inputGain;
            right2x[i] *= inputGain;
        }

        if (!impl._stlink) {
            absl::Span<float> leftGain2x = impl._gain2x.getSpan(0);
            absl::Span<float> rightGain2x = impl._gain2x.getSpan(1);

            {
                faustCompressor& comp = impl._compressor[0];
                float* inputs[] = { left2x.data() };
                float* outputs[] = { leftGain2x.data() };
                comp.compute(_oversampling * nframes, inputs, outputs);
            }

            {
                faustCompressor& comp = impl._compressor[1];
                float* inputs[] = { right2x.data() };
                float* outputs[] = { rightGain2x.data() };
                comp.compute(_oversampling * nframes, inputs, outputs);
            }

            for (unsigned i = 0; i < _oversampling * nframes; ++i) {
                left2x[i] *= leftGain2x[i];
                right2x[i] *= rightGain2x[i];
            }
        }
        else {
            absl::Span<float> compIn2x = impl._gain2x.getSpan(0);
            for (unsigned i = 0; i < _oversampling * nframes; ++i)
                compIn2x[i] = std::abs(left2x[i]) + std::abs(right2x[1]);

            absl::Span<float> gain2x = impl._gain2x.getSpan(1);

            {
                faustCompressor& comp = impl._compressor[0];
                float* inputs[] = { compIn2x.data() };
                float* outputs[] = { gain2x.data() };
                comp.compute(_oversampling * nframes, inputs, outputs);
            }

            for (unsigned i = 0; i < _oversampling * nframes; ++i) {
                left2x[i] *= gain2x[i];
                right2x[i] *= gain2x[i];
            }
        }

        impl._downsampler2x[0].process_block(outputs[0], left2x.data(), nframes);
        impl._downsampler2x[1].process_block(outputs[1], right2x.data(), nframes);
    }

    std::unique_ptr<Effect> Compressor::makeInstance(absl::Span<const Opcode> members)
    {
        Compressor* compressor = new Compressor;
        std::unique_ptr<Effect> fx { compressor };

        Impl& impl = *compressor->_impl;

        for (const Opcode& opc : members) {
            switch (opc.lettersOnlyHash) {
            case hash("comp_attack"):
                {
                    auto value = opc.read(Default::compAttack);
                    for (faustCompressor& comp : impl._compressor)
                        comp.setAttack(value);
                }
                break;
            case hash("comp_release"):
                {
                    auto value = opc.read(Default::compRelease);
                    for (faustCompressor& comp : impl._compressor)
                        comp.setRelease(value);
                }
                break;
            case hash("comp_threshold"):
                {
                    auto value = opc.read(Default::compThreshold);
                    for (faustCompressor& comp : impl._compressor)
                        comp.setThreshold(value);
                }
                break;
            case hash("comp_ratio"):
                {
                    auto value = opc.read(Default::compRatio);
                    for (faustCompressor& comp : impl._compressor)
                        comp.setRatio(value);
                }
                break;
            case hash("comp_gain"):
                impl._inputGain = opc.read(Default::compGain);
                break;
            case hash("comp_stlink"):
                impl._stlink = opc.read(Default::compSTLink);
                break;
            }
        }

        return fx;
    }

} // namespace fx
} // namespace sfz
