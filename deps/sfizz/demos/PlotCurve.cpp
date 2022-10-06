// SPDX-License-Identifier: BSD-2-Clause

// This code is part of the sfizz library and is licensed under a BSD 2-clause
// license. You should have receive a LICENSE.md file along with the code.
// If not, contact the sfizz maintainers at https://github.com/sfztools/sfizz

/**
   This program generates the data file of a LFO output recorded for a fixed
   duration. The file contains columns for each LFO in the SFZ region.
   The columns are: Time, Lfo1, ... LfoN
   One can use Gnuplot to display this data.
   Example:
     sfizz_plot_lfo file.sfz > lfo.dat
     gnuplot
     plot "lfo.dat" using 1:2 with lines
 */

#include "sfizz/Curve.h"
#include "sfizz/parser/Parser.h"
#include "sfizz/parser/ParserListener.h"
#include "absl/strings/numbers.h"
#include "absl/types/span.h"
#include <iostream>
#include <cmath>

//==============================================================================

/**
 * @brief Print usage information
 */
static void usage()
{
    std::cerr << "Usage: sfizz_plot_curve <index> [file.sfz]"
              << "\n";
}

/**
 * @brief Parser listener which extracts the configuration of curves
 */
class CurveParserListener : public sfz::Parser::Listener {
public:
    explicit CurveParserListener(sfz::CurveSet& curveSet)
        : curveSet(curveSet)
    {
    }

    void onParseFullBlock(const std::string& header, const std::vector<sfz::Opcode>& members) override
    {
        if (header == "curve")
            curveSet.addCurveFromHeader(members);
    }

private:
    sfz::CurveSet& curveSet;
};

/**
 * @brief Program which loads LFO configuration and generates plot data for the given duration.
 */
int main(int argc, char* argv[])
{
    if (argc < 2 || argc > 3)
        return usage(), 1;

    int curveIndex = -1;
    if (!absl::SimpleAtoi(argv[1], &curveIndex))
        return usage(), 1;

    std::string filePath;
    if (argc > 2)
        filePath = argv[2];

    ///
    sfz::CurveSet curveSet = sfz::CurveSet::createPredefined();

    if (!filePath.empty()) {
        sfz::Parser parser;
        CurveParserListener listener(curveSet);
        parser.setListener(&listener);
        parser.parseFile(filePath);
        if (parser.getErrorCount() > 0) {
            std::cerr << "Cannot load SFZ: " << filePath << "\n";
            return 1;
        }
    }

    ///
    const sfz::Curve& curve = curveSet.getCurve(curveIndex);

    constexpr unsigned numSamples = 1024; // number of points to evaluate

    for (unsigned i = 0; i < numSamples; ++i) {
        float x = i * (1.0f / (numSamples - 1));
        std::cout << x << ' ' << curve.evalNormalized(x) << '\n';
    }

    return 0;
}
