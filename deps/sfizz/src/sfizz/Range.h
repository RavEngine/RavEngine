// SPDX-License-Identifier: BSD-2-Clause

// This code is part of the sfizz library and is licensed under a BSD 2-clause
// license. You should have receive a LICENSE.md file along with the code.
// If not, contact the sfizz maintainers at https://github.com/sfztools/sfizz

#pragma once
#include "MathHelpers.h"
#include "utility/Macros.h"
#include <type_traits>
#include <limits>

namespace sfz
{
/**
 * @brief This class holds a range with functions to clamp and test if a value is in the range
 *
 * @tparam Type
 * @tparam Checked whether this range adapts itself to guarantee (start<=end)
 */
template <class Type, bool Checked = true>
class Range {
    // static_assert(std::is_arithmetic<Type>::value
    //     || (std::is_enum<Type>::value && std::is_same<typename std::underlying_type<Type>::type, int>::value),
    //     "The Type should be arithmetic");

public:
    constexpr Range() = default;
    constexpr Range(Type start, Type end) noexcept
        : _start(start)
        , _end(Checked ? max(start, end) : end)
    {
    }

    constexpr Range(const Range<Type, !Checked>& other)
        : Range(other.getStart(), other.getEnd())
    {
    }

    constexpr Type getStart() const noexcept { return _start; }
    constexpr Type getEnd() const noexcept { return _end; }
    /**
     * @brief Get the range as an std::pair of the endpoints
     *
     * @return std::pair<Type, Type>
     */
    std::pair<Type, Type> getPair() const noexcept { return std::make_pair<Type, Type>(_start, _end); }
    constexpr Type length() const { return _end - _start; }
    void setStart(Type start) noexcept
    {
        _start = start;
        IF_CONSTEXPR (Checked) {
            if (start > _end)
                _end = start;
        }
    }

    void setEnd(Type end) noexcept
    {
        _end = end;
        IF_CONSTEXPR (Checked) {
            if (end < _start)
                _start = end;
        }
    }

    /**
     * @brief Check whether the range verifies (start<=end)
     */
    bool isValid() const noexcept
    {
        IF_CONSTEXPR (Checked)
            return true; // always valid
        else
            return _start <= _end;
    }

    /**
     * @brief Clamp a value within the range including the endpoints
     *
     * @param value
     * @return Type
     */
    constexpr Type clamp(Type value) const noexcept { return ::clamp(value, _start, _end); }
    /**
     * @brief Checks if a value is in the range, including the endpoints
     *
     * @param value
     * @return true
     * @return false
     */
    bool containsWithEnd(Type value) const noexcept { return (value >= _start && value <= _end); }
    /**
     * @brief Checks if a value is in the range, excluding the end of the range
     *
     * @param value
     * @return true
     * @return false
     */
    bool contains(Type value) const noexcept { return (value >= _start && value < _end); }
    /**
     * @brief Shrink the region if it is smaller than the provided start and end points
     *
     * @param start
     * @param end
     */
    void shrinkIfSmaller(Type start, Type end)
    {
        IF_CONSTEXPR (Checked) {
            if (start > end)
                std::swap(start, end);
        }

        if (start > _start)
            _start = start;

        if (end < _end)
            _end = end;
    }

    void expandTo(Type value)
    {
        if (value > _end)
            _end = value;
        else if (value < _start)
            _start = value;
    }

    /**
     * @brief Convert the range to a different value type
     *
     * @return Range<Other>
     */
    template <class Other>
    Range<Other> to() const noexcept
    {
        return Range<Other> {
            static_cast<Other>(_start),
            static_cast<Other>(_end),
        };
    }

    /**
     * @brief Construct a range which covers the whole numeric domain
     */
    static constexpr Range<Type> wholeRange() noexcept
    {
        return Range<Type> {
            std::numeric_limits<Type>::min(),
            std::numeric_limits<Type>::max(),
        };
    }

private:
    Type _start { static_cast<Type>(0.0) };
    Type _end { static_cast<Type>(0.0) };
};

template <class T> using UncheckedRange = Range<T, false>;

template <class Type, bool C1, bool C2>
bool operator==(const Range<Type, C1>& lhs, const Range<Type, C2>& rhs) noexcept
{
    return lhs.getStart() == rhs.getStart() && lhs.getEnd() == rhs.getEnd();
}

template <class Type, bool C1, bool C2>
bool operator!=(const Range<Type, C1>& lhs, const Range<Type, C2>& rhs) noexcept
{
    return !operator==(lhs, rhs);
}

} // namespace sfz
