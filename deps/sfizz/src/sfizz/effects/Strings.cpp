// SPDX-License-Identifier: BSD-2-Clause

// This code is part of the sfizz library and is licensed under a BSD 2-clause
// license. You should have receive a LICENSE.md file along with the code.
// If not, contact the sfizz maintainers at https://github.com/sfztools/sfizz


/**
   Note(jpc): implementation status

- [x] strings_number
- [ ] strings_wet_oncc

Extensions
- [x] strings_wet
 */

#include "Strings.h"
#include "impl/ResonantArray.h"
#include "impl/ResonantArraySSE.h"
#include "impl/ResonantArrayAVX.h"
#include "Opcode.h"
#include "MathHelpers.h"
#include "SIMDHelpers.h"
#include "cpuid/cpuinfo.hpp"
#include "absl/memory/memory.h"
#include <cmath>

namespace sfz {
namespace fx {

    Strings::Strings()
    {
        ResonantArray* array = nullptr;

#if SFIZZ_CPU_FAMILY_X86_64 || SFIZZ_CPU_FAMILY_I386
        cpuid::cpuinfo cpuInfo;
        if (cpuInfo.has_avx())
            array = new ResonantArrayAVX;
        else if (cpuInfo.has_sse())
            array = new ResonantArraySSE;
#endif
        if (!array)
            array = new ResonantArrayScalar;

        _stringsArray.reset(array);
    }

    Strings::~Strings()
    {
    }

    void Strings::setSampleRate(double sampleRate)
    {
        const unsigned numStrings = _numStrings;

        AudioBuffer<float, 4> parameterBuffers { 4, numStrings };

        auto pitches = parameterBuffers.getSpan(0);
        auto bandwidths = parameterBuffers.getSpan(1);
        auto feedbacks = parameterBuffers.getSpan(2);
        auto gains = parameterBuffers.getSpan(3);

        for (unsigned i = 0; i < numStrings; ++i) {
            int midiNote = i + 24;
            pitches[i] = 440.0 * std::exp2((midiNote - 69) * (1.0 / 12.0));
        }

        // 1 Hz works decently as compromise of selectivity/speed
        sfz::fill(bandwidths, 1.0f);

        // TODO(jpc) find how to adjust the string feedbacks
        //     for now set a fixed release time for all strings
        const double releaseTime = 50e-3;
        const double releaseFeedback = std::exp(-6.91 / (releaseTime * sampleRate));
        sfz::fill<float>(feedbacks, releaseFeedback);

        // TODO(jpc) damping of the high frequencies
        //     fixed gains for now
        sfz::fill(gains, 1e-3f);

        _stringsArray->setup(
            sampleRate, numStrings,
            pitches.data(), bandwidths.data(), feedbacks.data(), gains.data());
    }

    void Strings::setSamplesPerBlock(int samplesPerBlock)
    {
        _tempBuffer.resize(samplesPerBlock);

        _stringsArray->setSamplesPerBlock(samplesPerBlock);
    }

    void Strings::clear()
    {
        _stringsArray->clear();
    }

    void Strings::process(const float* const inputs[], float* const outputs[], unsigned nframes)
    {
        auto inputL = absl::MakeConstSpan(inputs[0], nframes);
        auto inputR = absl::MakeConstSpan(inputs[1], nframes);

        // mix down the stereo signal to create the resonator excitation source
        absl::Span<float> resInput = _tempBuffer.getSpan(0).first(nframes);
        sfz::applyGain1<float>(M_SQRT1_2, inputL, resInput);
        sfz::multiplyAdd1<float>(M_SQRT1_2, inputR, resInput);

        // generate the strings summed into a common buffer
        absl::Span<float> resOutput = _tempBuffer.getSpan(1).first(nframes);

        _stringsArray->process(resInput.data(), resOutput.data(), nframes);

        // mix the resonator into the output
        auto outputL = absl::MakeSpan(outputs[0], nframes);
        auto outputR = absl::MakeSpan(outputs[1], nframes);

        absl::Span<float> wet = _tempBuffer.getSpan(2).first(nframes);
        sfz::fill(wet, _wet); // TOD strings_wet_oncc modulation...

        sfz::copy(inputL, outputL);
        sfz::copy(inputR, outputR);
        sfz::multiplyAdd<float>(wet, resOutput, outputL);
        sfz::multiplyAdd<float>(wet, resOutput, outputR);
    }

    std::unique_ptr<Effect> Strings::makeInstance(absl::Span<const Opcode> members)
    {
        Strings* strings = new Strings;
        std::unique_ptr<Effect> fx { strings };

        for (const Opcode& opc : members) {
            switch (opc.lettersOnlyHash) {
            case hash("strings_number"):
                strings->_numStrings = opc.read(Default::stringsNumber);
                break;
            case hash("strings_wet"):
                strings->_wet = opc.read(Default::effect);
                break;
            }
        }

        return fx;
    }

} // namespace fx
} // namespace sfz
