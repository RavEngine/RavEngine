// SPDX-License-Identifier: BSD-2-Clause

// This code is part of the sfizz library and is licensed under a BSD 2-clause
// license. You should have receive a LICENSE.md file along with the code.
// If not, contact the sfizz maintainers at https://github.com/sfztools/sfizz

#include "FileId.h"
#include "utility/StringViewHelpers.h"
#include <iostream>

size_t std::hash<sfz::FileId>::operator()(const sfz::FileId &id) const
{
    uint64_t h = ::hash(id.filename());
    h = ::hash(id.isReverse() ? "!" : "", h);
    return h;
}

std::ostream &operator<<(std::ostream &os, const sfz::FileId &fileId)
{
    os << fileId.filename();
    if (fileId.isReverse())
        os << " (reverse)";
    return os;
}

const std::string sfz::FileId::emptyFilename;
