// SPDX-License-Identifier: BSD-2-Clause

// This code is part of the sfizz library and is licensed under a BSD 2-clause
// license. You should have receive a LICENSE.md file along with the code.
// If not, contact the sfizz maintainers at https://github.com/sfztools/sfizz

#include "sfizz/Tuning.h"
#include "sfizz/Opcode.h"
#include "sfizz/SfzHelpers.h"
#include "cxxopts.hpp"
#include <string>
#include <cstdio>
#include <cstring>
#include <cmath>

static const char *octNoteNames[12] = {
    "C", "C#", "D", "D#", "E", "F", "F#", "G", "G#", "A", "A#", "B",
};

static std::string noteName(int key)
{
    int octNum;
    int octNoteNum;
    if (key >= 0) {
        octNum = key / 12 - 1;
        octNoteNum = key % 12;
    }
    else {
        octNum = -2 - (key + 1) / -12;
        octNoteNum = (key % 12 + 12) % 12;
    }
    return std::string(octNoteNames[octNoteNum]) + std::to_string(octNum);
}

int main(int argc, char* argv[])
{
    cxxopts::Options options(argv[0], " - command line options");

    options.add_options()
      ("h,help", "Print help")
      ("s,scale", "Path of scala tuning file", cxxopts::value<std::string>())
      ("f,frequency", "Tuning frequency", cxxopts::value<float>()->default_value("440.0"))
      ("r,root-key", "Root key", cxxopts::value<std::string>()->default_value("C4"));

    auto result = options.parse(argc, argv);

    if (result.count("help")) {
        std::cout << options.help({""}) << std::endl;
        return 0;
    }

    ///
    sfz::Tuning tuning;

    if (result.count("scale")) {
        if (!tuning.loadScalaFile(result["scale"].as<std::string>())) {
            fprintf(stderr, "Could not load the scale file.\n");
            return 1;
        }
    }

    absl::optional<uint8_t> noteNumber = sfz::readNoteValue(
        result["root-key"].as<std::string>());
    if (!noteNumber) {
        fprintf(stderr, "The root key is not a valid note name.\n");
        return 1;
    }

    tuning.setScalaRootKey(*noteNumber);
    tuning.setTuningFrequency(result["frequency"].as<float>());

    const int numRows = 3;
    const int numCols = 4;

    for (int row = 0; row < numRows; ++row) {
        for (int i = 0; i < 73; ++i)
            putchar('-');
        putchar('\n');
        for (int nthKey = 0; nthKey < 12; ++nthKey) {
            for (int col = 0; col < numCols; ++col) {
                const int key = nthKey + (col + row * numCols) * 12;

                std::string label;
                label.push_back('|');
                label.append(noteName(key));
                while (label.size() < 5)
                    label.push_back(' ');
                label.push_back('|');

                if (col > 0) {
                    for (int i = 0; i < 1; ++i)
                        putchar(' ');
                }

                printf("%s %10.4f", label.c_str(), tuning.getFrequencyOfKey(key));
            }
            putchar(' ');
            putchar('|');
            putchar('\n');
        }
    }
    for (int i = 0; i < 73; ++i)
        putchar('-');
    putchar('\n');

    return 0;
}
