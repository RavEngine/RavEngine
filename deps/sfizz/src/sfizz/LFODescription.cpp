// SPDX-License-Identifier: BSD-2-Clause

// This code is part of the sfizz library and is licensed under a BSD 2-clause
// license. You should have receive a LICENSE.md file along with the code.
// If not, contact the sfizz maintainers at https://github.com/sfztools/sfizz

#include "LFODescription.h"

namespace sfz {

LFODescription::LFODescription()
{
    sub.resize(1);
}

LFODescription::~LFODescription()
{
}

const LFODescription& LFODescription::getDefault()
{
    static LFODescription desc = []() -> LFODescription
    {
        LFODescription desc;
        desc.sub.resize(1);
        return desc;
    }();
    return desc;
}

} // namespace sfz
