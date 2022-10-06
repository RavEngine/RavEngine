// SPDX-License-Identifier: BSD-2-Clause

// This code is part of the sfizz library and is licensed under a BSD 2-clause
// license. You should have receive a LICENSE.md file along with the code.
// If not, contact the sfizz maintainers at https://github.com/sfztools/sfizz

#include "sfizz_import.h"
#include "ForeignInstrument.h"

bool sfizz_load_or_import_file(sfizz_synth_t* synth, const char* path, const char** format)
{
    const sfz::InstrumentFormatRegistry& ireg = sfz::InstrumentFormatRegistry::getInstance();
    const sfz::InstrumentFormat* ifmt = ireg.getMatchingFormat(path);

    if (!ifmt) {
        if (!sfizz_load_file(synth, path))
            return false;
        if (format)
            *format = nullptr;
    }
    else {
        auto importer = ifmt->createImporter();
        std::string virtualPath = std::string(path) + ".sfz";
        std::string sfzText = importer->convertToSfz(path);
        if (!sfizz_load_string(synth, virtualPath.c_str(), sfzText.c_str()))
            return false;
        if (format)
            *format = ifmt->name();
    }

    return true;
}
