// SPDX-License-Identifier: BSD-2-Clause

// This code is part of the sfizz library and is licensed under a BSD 2-clause
// license. You should have receive a LICENSE.md file along with the code.
// If not, contact the sfizz maintainers at https://github.com/sfztools/sfizz


/**
   Note(jpc): implementation status

- [x] filter_type
- [x] filter_cutoff
- [ ] filter_cutoff_oncc
- [x] filter_resonance
- [ ] filter_resonance_oncc

Potential extensions (like ARIA)
- [/] filter_gain
- [ ] filter_gain_oncc

 */

#include "Filter.h"
#include "Opcode.h"
#include "SIMDHelpers.h"
#include "utility/Debug.h"
#include <absl/memory/memory.h>

namespace sfz {
namespace fx {
    Filter::Filter(const FilterDescription& desc)
        : _desc(desc)
    {
        _filter.setType(desc.type);
        _filter.setChannels(2);
    }

    void Filter::setSampleRate(double sampleRate)
    {
        _filter.init(sampleRate);
        prepareFilter();
    }

    void Filter::setSamplesPerBlock(int samplesPerBlock)
    {
        _tempBuffer.resize(samplesPerBlock);
    }

    void Filter::clear()
    {
        _filter.clear();
        prepareFilter();
    }

    void Filter::process(const float* const inputs[], float* const outputs[], unsigned nframes)
    {
        absl::Span<float> cutoff = _tempBuffer.getSpan(0).first(nframes);
        absl::Span<float> q = _tempBuffer.getSpan(1).first(nframes);
        absl::Span<float> pksh = _tempBuffer.getSpan(2).first(nframes);

        sfz::fill(cutoff, _desc.cutoff);
        sfz::fill(q, _desc.resonance);
        sfz::fill(pksh, _desc.gain);

        _filter.processModulated(inputs, outputs, cutoff.data(), q.data(), pksh.data(), nframes);
    }

    std::unique_ptr<Effect> Filter::makeInstance(absl::Span<const Opcode> members)
    {
        FilterDescription desc;

        for (const Opcode& opc : members) {
            switch (opc.lettersOnlyHash) {
            case hash("filter_cutoff"):
                desc.cutoff = opc.read(Default::filterCutoff);
                break;
            case hash("filter_resonance"):
                desc.resonance = opc.read(Default::filterResonance);
                break;
            case hash("filter_type"):
                desc.type = opc.read(Default::filter);
                break;
            // extension
            case hash("sfizz:filter_gain"):
                desc.gain = opc.read(Default::filterGain);
                break;
            }
        }

        Filter* filter = new Filter(desc);
        std::unique_ptr<Effect> fx { filter };
        return fx;
    }

    void Filter::prepareFilter()
    {
        _filter.prepare(_desc.cutoff, _desc.resonance, _desc.gain);
    }

} // namespace fx
} // namespace sfz
