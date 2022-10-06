// SPDX-License-Identifier: BSD-2-Clause

// This code is part of the sfizz library and is licensed under a BSD 2-clause
// license. You should have receive a LICENSE.md file along with the code.
// If not, contact the sfizz maintainers at https://github.com/sfztools/sfizz

#pragma once
#include <pugixml.hpp>
#include <string>

class string_xml_writer : public pugi::xml_writer {
public:
    explicit string_xml_writer(size_t capacity = 8192)
    {
        result_.reserve(capacity);
    }

    void write(const void* data, size_t size) override
    {
        result_.append(static_cast<const char*>(data), size);
    }

    std::string& str() noexcept
    {
        return result_;
    }

private:
    std::string result_;
};
