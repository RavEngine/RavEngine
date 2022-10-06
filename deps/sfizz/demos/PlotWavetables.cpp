// SPDX-License-Identifier: BSD-2-Clause

// This code is part of the sfizz library and is licensed under a BSD 2-clause
// license. You should have receive a LICENSE.md file along with the code.
// If not, contact the sfizz maintainers at https://github.com/sfztools/sfizz

#include "sfizz/Wavetables.h"
#include <absl/strings/numbers.h>
#include <absl/strings/string_view.h>
#include <iostream>

static void usage()
{
    std::cerr << "Usage: sfizz_plot_wavetables [-w wave] [-a amplitude] [-c cutoff] [-m]\n";
}

int main(int argc, char* argv[])
{
    absl::string_view waveName;
    double amplitude = 1.0;
    double cutoff = 0.5;
    bool generateMulti = false;

    for (int i = 1; i < argc; ++i) {
        absl::string_view arg = argv[i];

        if (arg == "-w") {
            if (i + 1 >= argc)
                return usage(), 1;
            waveName = argv[++i];
        } else if (arg == "-a") {
            if (i + 1 >= argc || !absl::SimpleAtod(argv[++i], &amplitude))
                return usage(), 1;
        } else if (arg == "-c") {
            if (i + 1 >= argc || !absl::SimpleAtod(argv[++i], &cutoff))
                return usage(), 1;
        } else if (arg == "-m")
            generateMulti = true;
        else
            return usage(), 1;
    }

    if (waveName.empty())
        waveName = "saw";

    const sfz::HarmonicProfile* hp = nullptr;
    if (waveName == "sine")
        hp = &sfz::HarmonicProfile::getSine();
    else if (waveName == "square")
        hp = &sfz::HarmonicProfile::getSquare();
    else if (waveName == "triangle")
        hp = &sfz::HarmonicProfile::getTriangle();
    else if (waveName == "saw")
        hp = &sfz::HarmonicProfile::getSaw();
    else {
        std::cerr << "Unknown wave: " << waveName << '\n';
        return 1;
    }

    constexpr size_t tableSize = 2048;
    float table[tableSize];

    if (!generateMulti) {
        hp->generate(table, amplitude, cutoff);

        for (size_t i = 0; i < tableSize; ++i)
            std::cout << (i * (1.0 / tableSize)) << ' ' << table[i] << '\n';
    } else {
        sfz::WavetableMulti multi = sfz::WavetableMulti::createForHarmonicProfile(
            *hp, 1.0, tableSize);
        unsigned numTables = multi.numTables();

        if (true) {
            // print all tables one after another
            for (unsigned m = 0; m < numTables; ++m) {
                absl::Span<const float> table = multi.getTable(m);
                for (size_t i = 0; i < tableSize; ++i) {
                    std::cout << ((i + m * tableSize) * (1.0 / tableSize))
                              << ' ' << table[i] << '\n';
                }
            }
        } else {
            // print all tables separately
            for (size_t i = 0; i < tableSize; ++i) {
                std::cout << (i * (1.0 / tableSize));
                for (unsigned m = 0; m < numTables; ++m) {
                    absl::Span<const float> table = multi.getTable(m);
                    std::cout << ' ' << table[i];
                }
                std::cout << '\n';
            }
        }
    }

    return 0;
}
