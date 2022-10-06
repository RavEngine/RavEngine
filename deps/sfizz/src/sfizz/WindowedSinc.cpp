// SPDX-License-Identifier: BSD-2-Clause

// This code is part of the sfizz library and is licensed under a BSD 2-clause
// license. You should have receive a LICENSE.md file along with the code.
// If not, contact the sfizz maintainers at https://github.com/sfztools/sfizz

#include "WindowedSinc.h"
#include "MathHelpers.h"
#include <absl/memory/memory.h>

namespace sfz {

void WindowedSincDetail::calculateTable(absl::Span<float> table, size_t sincExtent, double beta, size_t extra)
{
    size_t tableSize = table.size();

    auto window = absl::make_unique<float[]>(tableSize);
    kaiserWindow(beta, absl::MakeSpan(window.get(), tableSize));

    // table domain [-N/2:+N/2]
    double scale = sincExtent / static_cast<double>(tableSize - 1);
    double offset = sincExtent / -2.0;

    for (size_t i = 0; i < tableSize; ++i) {
        double x = i * scale + offset;
        table[i] = window[i] * normalizedSinc(x);
    }

    for (size_t i = 0; i < extra; ++i)
        table[extra + i] = table[tableSize - 1];
}

double WindowedSincDetail::calculateExact(double x, size_t sincExtent, double beta)
{
    return normalizedSinc(x) *
        kaiserWindowSinglePoint(beta, (x + sincExtent / 2.0f) / sincExtent);
}

} // namespace sfz
