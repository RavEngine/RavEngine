// SPDX-License-Identifier: BSD-2-Clause

// This code is part of the sfizz library and is licensed under a BSD 2-clause
// license. You should have receive a LICENSE.md file along with the code.
// If not, contact the sfizz maintainers at https://github.com/sfztools/sfizz

#include "ForeignInstrument.h"
#include "foreign_instruments/AudioFile.h"
#include "foreign_instruments/DecentSampler.h"

namespace sfz {

InstrumentFormatRegistry::InstrumentFormatRegistry()
    : formats_ {
            &AudioFileInstrumentFormat::getInstance(),
            &DecentSamplerInstrumentFormat::getInstance(),
      }
{
}

InstrumentFormatRegistry& InstrumentFormatRegistry::getInstance()
{
    static InstrumentFormatRegistry registry;
    return registry;
}

const InstrumentFormat* InstrumentFormatRegistry::getMatchingFormat(const fs::path& path) const
{
    const InstrumentFormat* resultFormat = nullptr;

    for (const InstrumentFormat* currentFormat : formats_) {
        if (currentFormat->matchesFilePath(path)) {
            resultFormat = currentFormat;
            break;
        }
    }

    return resultFormat;
}

} // namespace sfz
