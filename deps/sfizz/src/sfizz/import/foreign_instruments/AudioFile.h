// SPDX-License-Identifier: BSD-2-Clause

// This code is part of the sfizz library and is licensed under a BSD 2-clause
// license. You should have receive a LICENSE.md file along with the code.
// If not, contact the sfizz maintainers at https://github.com/sfztools/sfizz

#pragma once
#include "../ForeignInstrument.h"

namespace sfz {

class AudioFileInstrumentFormat : public InstrumentFormat {
private:
    AudioFileInstrumentFormat() noexcept = default;

public:
    static AudioFileInstrumentFormat& getInstance();
    const char* name() const noexcept override;
    bool matchesFilePath(const fs::path& path) const override;
    std::unique_ptr<InstrumentImporter> createImporter() const override;
};

///
class AudioFileInstrumentImporter : public InstrumentImporter {
public:
    std::string convertToSfz(const fs::path& path) const override;
    const InstrumentFormat* getFormat() const noexcept override;
};

} // namespace sfz
