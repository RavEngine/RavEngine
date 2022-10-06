// SPDX-License-Identifier: BSD-2-Clause

// This code is part of the sfizz library and is licensed under a BSD 2-clause
// license. You should have receive a LICENSE.md file along with the code.
// If not, contact the sfizz maintainers at https://github.com/sfztools/sfizz

#pragma once
#include "../ForeignInstrument.h"
#include <pugixml.hpp>
#include <iosfwd>

namespace sfz {

class DecentSamplerInstrumentFormat : public InstrumentFormat {
private:
    DecentSamplerInstrumentFormat() noexcept = default;

public:
    static DecentSamplerInstrumentFormat& getInstance();
    const char* name() const noexcept override;
    bool matchesFilePath(const fs::path& path) const override;
    std::unique_ptr<InstrumentImporter> createImporter() const override;
};

///
class DecentSamplerInstrumentImporter : public InstrumentImporter {
public:
    std::string convertToSfz(const fs::path& path) const override;
    const InstrumentFormat* getFormat() const noexcept override;

private:
    void emitRegionalOpcodes(std::ostream& os, pugi::xml_node node) const;
};

} // namespace sfz
