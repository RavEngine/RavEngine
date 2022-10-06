// SPDX-License-Identifier: BSD-2-Clause

// This code is part of the sfizz library and is licensed under a BSD 2-clause
// license. You should have receive a LICENSE.md file along with the code.
// If not, contact the sfizz maintainers at https://github.com/sfztools/sfizz

#pragma once
#include <ghc/fs_std.hpp>
#include <string>
#include <vector>
#include <memory>

namespace sfz {

class InstrumentFormat;
class InstrumentImporter;

/**
 * Registry of known non-SFZ instrument formats
 */
class InstrumentFormatRegistry {
private:
    InstrumentFormatRegistry();

public:
    /**
     * @brief Get the single instance of the registry
     */
    static InstrumentFormatRegistry& getInstance();

    /**
     * @brief Get a format which is able to handle a file which goes by the
     * given path name, or null otherwise.
     */
    const InstrumentFormat* getMatchingFormat(const fs::path& path) const;

    /**
     * @brief Get the list of registered formats.
     */
    const std::vector<const InstrumentFormat*>& getAllFormats() const noexcept { return formats_; }

private:
    std::vector<const InstrumentFormat*> formats_;
};

/**
 * Description of a non-SFZ instrument format
 */
class InstrumentFormat {
public:
    virtual ~InstrumentFormat() {}

    /**
     * @brief Get the name of the instrument format
     */
    virtual const char* name() const noexcept = 0;

    /**
     * @brief Checks whether this importer matches files of the given path
     *
     * This should check for a pattern like such as a file extension, but not
     * examine the contents of the file itself.
     */
    virtual bool matchesFilePath(const fs::path& path) const = 0;

    /**
     * @brief Create a new importer for instrument files of this format
     */
    virtual std::unique_ptr<InstrumentImporter> createImporter() const = 0;
};

/**
 * Importer of non-SFZ instruments
 */
class InstrumentImporter {
public:
    virtual ~InstrumentImporter() {}

    /**
     * @brief Get the format that this importer converts from
     */
    virtual const InstrumentFormat* getFormat() const noexcept = 0;

    /**
     * @brief Process the file and convert to an equivalent SFZ string
     */
    virtual std::string convertToSfz(const fs::path& path) const = 0;
};

} // namespace sfz
