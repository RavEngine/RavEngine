// SPDX-License-Identifier: BSD-2-Clause

// This code is part of the sfizz library and is licensed under a BSD 2-clause
// license. You should have receive a LICENSE.md file along with the code.
// If not, contact the sfizz maintainers at https://github.com/sfztools/sfizz

#include "Interpolators.h"

namespace sfz {

void initializeInterpolators()
{
    SincInterpolatorTraits<8>::initialize();
    SincInterpolatorTraits<12>::initialize();
    SincInterpolatorTraits<16>::initialize();
    SincInterpolatorTraits<24>::initialize();
    SincInterpolatorTraits<36>::initialize();
    SincInterpolatorTraits<48>::initialize();
    SincInterpolatorTraits<60>::initialize();
    SincInterpolatorTraits<72>::initialize();
}

} // namespace sfz
