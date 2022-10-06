// SPDX-License-Identifier: BSD-2-Clause

// This code is part of the sfizz library and is licensed under a BSD 2-clause
// license. You should have receive a LICENSE.md file along with the code.
// If not, contact the sfizz maintainers at https://github.com/sfztools/sfizz

// Inspired by the public domain implementation at https://en.wikibooks.org/wiki/Algorithm_Implementation/Miscellaneous/Base64

#pragma once

#include "Debug.h"
#include <vector>
#include <absl/strings/string_view.h>

namespace sfz
{

inline std::vector<char> decodeBase64(absl::string_view input)
{
    if (input.empty())
        return {};

    std::size_t padding { 0 };
    auto length = input.length();

    if (length >= 1 and input[length - 1] == '=')
        padding++;

    if (length >= 2 and input[length - 2] == '=')
        padding++;

    input.remove_suffix(padding);

    std::vector<char> decoded;
    decoded.reserve(length);

    auto cursor = input.begin();
    int32_t temp { 0 };
    size_t position { 0 };
    while (cursor != input.end()) {
        char c = *cursor++;
        if (c == ' ' || c == '\n' || c == '\t' || c == '\r') // Ignore whitespace
            continue;

        if (c >= 0x41 && c <= 0x5A)   // This area will need tweaking if
            temp |= c - 0x41;	    // you are using an alternate alphabet
        else if (c >= 0x61 && c <= 0x7A)
            temp |= c - 0x47;
        else if (c >= 0x30 && c <= 0x39)
            temp |= c + 0x04;
        else if (c == 0x2B)
            temp |= 0x3E; //change to 0x2D for URL alphabet
        else if (c == 0x2F)
            temp |= 0x3F; //change to 0x5F for URL alphabet
        else
            return {};

        if (++position == 4) {
            decoded.push_back((temp >> 16) & 0x000000FF);
            decoded.push_back((temp >> 8 ) & 0x000000FF);
            decoded.push_back((temp      ) & 0x000000FF);
            position = 0;
            temp = 0;
        } else {
            temp <<= 6;
        }
    }

    switch(position) {
        case 3:
            decoded.push_back((temp >> 16) & 0x000000FF);
            decoded.push_back((temp >> 8 ) & 0x000000FF);
            break;
        case 2:
            decoded.push_back((temp >> 10) & 0x000000FF);
            break;
        case 1:
            return {};
        default:
            break;
    }

    return decoded;
}

}
