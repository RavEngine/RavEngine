// SPDX-License-Identifier: BSD-2-Clause

// This code is part of the sfizz library and is licensed under a BSD 2-clause
// license. You should have receive a LICENSE.md file along with the code.
// If not, contact the sfizz maintainers at https://github.com/sfztools/sfizz

#include "ModKeyHash.h"
#include "ModKey.h"
#include "ModId.h"
#include "utility/StringViewHelpers.h"
#include <cstdint>

size_t std::hash<sfz::ModKey>::operator()(const sfz::ModKey &key) const
{
    uint64_t k = hashNumber(static_cast<int>(key.id()));
    const sfz::ModKey::Parameters& p = key.parameters();

    switch (key.id()) {
    case sfz::ModId::Controller:
        k = hashNumber(p.cc, k);
        k = hashNumber(p.curve, k);
        k = hashNumber(p.smooth, k);
        k = hashNumber(p.step, k);
        break;
    default:
        k = hashNumber(p.N, k);
        k = hashNumber(p.X, k);
        k = hashNumber(p.Y, k);
        k = hashNumber(p.Z, k);
        break;
    }
    return k;
}
