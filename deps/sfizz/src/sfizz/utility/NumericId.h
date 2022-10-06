// SPDX-License-Identifier: BSD-2-Clause

// This code is part of the sfizz library and is licensed under a BSD 2-clause
// license. You should have receive a LICENSE.md file along with the code.
// If not, contact the sfizz maintainers at https://github.com/sfztools/sfizz

#pragma once
#include "StringViewHelpers.h"
#include <functional>

/**
 * @brief Numeric identifier
 *
 * It is a generic numeric identifier. The template wrapper serves to enforce a
 * stronger compile-time check, such that one kind of identifier can't be
 * mistaken for another kind, or for an unrelated integer such as an index.
 */
template <class T>
struct NumericId {
    constexpr NumericId() = default;

    explicit constexpr NumericId(int number)
        : number_(number)
    {
    }

    constexpr bool valid() const noexcept
    {
        return number_ != -1;
    }

    constexpr int number() const noexcept
    {
        return number_;
    }

    explicit operator bool() const noexcept
    {
        return valid();
    }

    constexpr bool operator==(NumericId other) const noexcept
    {
        return number_ == other.number_;
    }

    constexpr bool operator!=(NumericId other) const noexcept
    {
        return number_ != other.number_;
    }

private:
    int number_ = -1;
};

namespace std {
    template <class U> struct hash<NumericId<U>> {
        size_t operator()(const NumericId<U> &id) const
        {
            return hashNumber(id.number());
        }
    };
}
