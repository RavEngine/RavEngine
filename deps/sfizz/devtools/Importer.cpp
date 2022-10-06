#include "sfizz/import/ForeignInstrument.h"
#include <iostream>

int main(int argc, char* argv[])
{
    const sfz::InstrumentFormatRegistry& formatRegistry = sfz::InstrumentFormatRegistry::getInstance();

    if (argc != 2) {
        std::cerr << "Usage: sfizz_importer <foreign-instrument>\n";
        std::cerr << "--\n" "Supported formats:\n";
        for (const sfz::InstrumentFormat* format : formatRegistry.getAllFormats())
            std::cerr << " * " << format->name() << '\n';
        return 1;
    }

    const fs::path foreignPath = fs::u8path(argv[1]);

    const sfz::InstrumentFormat* format = formatRegistry.getMatchingFormat(foreignPath);

    if (!format) {
        std::cerr << "There is no support for files of this format.\n";
        return 1;
    }

    auto importer = format->createImporter();
    std::string text = importer->convertToSfz(foreignPath);

    if (text.empty()) {
        std::cerr << "The conversion has failed.\n";
        return 1;
    }

    std::cout << text;
    if (text.back() != '\n')
        std::cout << '\n';
    std::cout << std::flush;

    return 0;
}
