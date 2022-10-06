// SPDX-License-Identifier: BSD-2-Clause

// This code is part of the sfizz library and is licensed under a BSD 2-clause
// license. You should have receive a LICENSE.md file along with the code.
// If not, contact the sfizz maintainers at https://github.com/sfztools/sfizz

#pragma once
#include "Config.h"
#include "MathHelpers.h"
#include "SIMDHelpers.h"
#include "utility/Macros.h"
//#include <string>
#include <array>
#include <cmath>
#include <absl/types/optional.h>
#include <absl/strings/string_view.h>
#include <absl/algorithm/container.h>
#include <absl/meta/type_traits.h>

namespace sfz {

using CCNamePair = std::pair<uint16_t, std::string>;
using NoteNamePair = std::pair<uint8_t, std::string>;

template<class T>
struct ModifierCurvePair
{
    ModifierCurvePair(const T& modifier, uint8_t curve)
    : modifier(modifier), curve(curve) {}
    T modifier {};
    uint8_t curve {};
};

template <class T>
using MidiNoteArray = std::array<T, 128>;
template <class ValueType>
struct CCData {
    int cc;
    ValueType data;
    static_assert(config::numCCs - 1 < std::numeric_limits<decltype(cc)>::max(), "The cc type in the CCData struct cannot support the required number of CCs");
};

template <class ValueType>
struct CCDataComparator {
    bool operator()(const CCData<ValueType>& ccData, const int& cc)
    {
        return (ccData.cc < cc);
    }

    bool operator()(const int& cc, const CCData<ValueType>& ccData)
    {
        return (cc < ccData.cc);
    }

    bool operator()(const CCData<ValueType>& lhs, const CCData<ValueType>& rhs)
    {
        return (lhs.cc < rhs.cc);
    }
};

struct MidiEvent {
    int delay;
    float value;
};

using EventVector = std::vector<MidiEvent>;

struct MidiEventDelayComparator {
    bool operator()(const MidiEvent& event, const int& delay)
    {
        return (event.delay < delay);
    }

    bool operator()(const int& delay, const MidiEvent& event)
    {
        return (delay < event.delay);
    }

    bool operator()(const MidiEvent& lhs, const MidiEvent& rhs)
    {
        return (lhs.delay < rhs.delay);
    }
};

struct MidiEventValueComparator {
    bool operator()(const MidiEvent& event, const float& value)
    {
        return (event.value < value);
    }

    bool operator()(const float& value, const MidiEvent& event)
    {
        return (value < event.value);
    }

    bool operator()(const MidiEvent& lhs, const MidiEvent& rhs)
    {
        return (lhs.value < rhs.value);
    }
};

/**
 * @brief Converts cents to a pitch ratio
 *
 * @tparam T
 * @param cents
 * @param centsPerOctave
 * @return constexpr float
 */
template <class T>
constexpr float centsFactor(T cents, T centsPerOctave = 1200)
{
    return std::pow(2.0f, static_cast<float>(cents) / centsPerOctave);
}

template <class T, absl::enable_if_t<std::is_integral<T>::value, int> = 0>
constexpr T denormalize7Bits(float value)
{
    return static_cast<T>(value * 127.0f);
}

constexpr uint8_t denormalizeCC(float value)
{
    return denormalize7Bits<uint8_t>(value);
}

constexpr uint8_t denormalizeVelocity(float value)
{
    return denormalize7Bits<uint8_t>(value);
}

template <class T>
constexpr float normalize7Bits(T value)
{
    return static_cast<float>(min(max(value, T { 0 }), T { 127 })) / 127.0f;
}

template <>
constexpr float normalize7Bits(bool value)
{
    return value ? 1.0f : 0.0f;
}

/**
 * @brief Normalize a CC value between 0.0 and 1.0
 *
 * @tparam T
 * @param ccValue
 * @return constexpr float
 */
template <class T>
constexpr float normalizeCC(T ccValue)
{
    return normalize7Bits(ccValue);
}

/**
 * @brief Normalize a velocity between 0.0 and 1.0
 *
 * @tparam T
 * @param ccValue
 * @return constexpr float
 */
template <class T>
constexpr float normalizeVelocity(T velocity)
{
    return normalize7Bits(velocity);
}

/**
 * @brief Normalize a percentage between 0 and 1
 *
 * @tparam T
 * @param percentValue
 * @return constexpr float
 */
template <class T>
constexpr float normalizePercents(T percentValue)
{
    return percentValue / 100.0f;
}

/**
 * @brief Normalize bends between -1 and 1. We clamp to 8191 instead of 8192 in the low end
 * to have something symmetric with respect to 0.
 *
 * @param bendValue
 * @return constexpr float
 */
constexpr float normalizeBend(float bendValue)
{
    return clamp(bendValue, -8191.0f, 8191.0f) / 8191.0f;
}

/**
 * @brief Offset a key and clamp it to a reasonable range
 *
 * @param key
 * @param offset
 * @return uint8_t
 */
inline CXX14_CONSTEXPR uint8_t offsetAndClampKey(uint8_t key, int offset)
{
    const int offsetKey { key + offset };
    if (offsetKey > std::numeric_limits<uint8_t>::max())
        return 127;
    if (offsetKey < std::numeric_limits<uint8_t>::min())
        return 0;

    return clamp<uint8_t>(static_cast<uint8_t>(offsetKey), 0, 127);
}

namespace literals {
    inline float operator""_norm(unsigned long long int value)
    {
        if (value > 127)
            value = 127;

        return normalize7Bits(value);
    }

    inline float operator""_norm(long double value)
    {
        if (value < 0)
            value = 0;
        if (value > 127)
            value = 127;

        return normalize7Bits(value);
    }
}

template <class Type>
inline CXX14_CONSTEXPR Type vaGain(Type cutoff, Type sampleRate)
{
    return std::tan(cutoff / sampleRate * pi<Type>());
}

/**
 * @brief Insert an item uniquely into a vector of pairs.
 *
 * @param pairVector the vector of pairs
 * @param key the unique key
 * @param value the value
 * @param replace whether to replace the value if the key is already present
 * @return whether the item was inserted
 */
template <class P, class T, class U>
bool insertPairUniquely(std::vector<P>& pairVector, const T& key, U value, bool replace = true)
{
    bool result = false;
    auto it = absl::c_find_if(
        pairVector, [&key](const P& pair) { return pair.first == key; });
    if (it != pairVector.end()) {
        if (replace) {
            it->second = std::move(value);
            result = true;
        }
    }
    else {
        pairVector.emplace_back(key, std::move(value));
        result = true;
    }
    return result;
}

} // namespace sfz
