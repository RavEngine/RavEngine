// SPDX-License-Identifier: BSD-2-Clause

// This code is part of the sfizz library and is licensed under a BSD 2-clause
// license. You should have receive a LICENSE.md file along with the code.
// If not, contact the sfizz maintainers at https://github.com/sfztools/sfizz

#include "ParserPrivate.h"

namespace sfz {

template <class P>
size_t Reader::extractWhile(std::string* dst, const P& pred)
{
    int byte;
    size_t count = 0;
    bool more = true;

    while (more && (byte = getChar()) != EOF) {
        more = pred(static_cast<unsigned char>(byte));
        if (!more)
            putBackChar(byte);
        else {
            if (dst)
                dst->push_back(static_cast<unsigned char>(byte));
            ++count;
        }
    }

    return count;
}

template <class P>
size_t Reader::extractUntil(std::string* dst, const P& pred)
{
    return extractWhile(dst, [&pred](char c) -> bool { return !pred(c); });
}

template <class P>
size_t Reader::skipWhile(const P& pred)
{
    return extractWhile(nullptr, pred);
}

template <class P>
size_t Reader::skipUntil(const P& pred)
{
    return extractUntil(nullptr, pred);
}

}  // namespace sfz
