// SPDX-License-Identifier: BSD-2-Clause

// This code is part of the sfizz library and is licensed under a BSD 2-clause
// license. You should have receive a LICENSE.md file along with the code.
// If not, contact the sfizz maintainers at https://github.com/sfztools/sfizz

#pragma once
#include <string>
#include <vector>

namespace sfz {
struct SourceRange;
struct Opcode;

class ParserListener {
public:
    // low-level parsing
    virtual void onParseBegin() {}
    virtual void onParseEnd() {}
    virtual void onParseHeader(const SourceRange& /*range*/, const std::string& /*header*/) {}
    virtual void onParseOpcode(const SourceRange& /*rangeOpcode*/, const SourceRange& /*rangeValue*/, const std::string& /*name*/, const std::string& /*value*/) {}
    virtual void onParseError(const SourceRange& /*range*/, const std::string& /*message*/) {}
    virtual void onParseWarning(const SourceRange& /*range*/, const std::string& /*message*/) {}

    // high-level parsing
    virtual void onParseFullBlock(const std::string& /*header*/, const std::vector<Opcode>& /*opcodes*/) {}
};

} // namespace sfz
