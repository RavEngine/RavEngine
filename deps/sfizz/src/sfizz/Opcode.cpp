// SPDX-License-Identifier: BSD-2-Clause

// This code is part of the sfizz library and is licensed under a BSD 2-clause
// license. You should have receive a LICENSE.md file along with the code.
// If not, contact the sfizz maintainers at https://github.com/sfztools/sfizz

#include "Opcode.h"
#include "LFODescription.h"
#include "utility/StringViewHelpers.h"
#include "utility/Debug.h"
#include <absl/strings/ascii.h>
#include <absl/strings/match.h>
#include <absl/strings/str_cat.h>
#include <limits>
#include <iostream>
#include <cctype>
#include <cassert>

namespace sfz {

Opcode::Opcode(absl::string_view inputOpcode, absl::string_view inputValue)
    : name(trim(inputOpcode))
    , value(trim(inputValue))
    , category(identifyCategory(inputOpcode))
{
    size_t nextCharIndex { 0 };
    int parameterPosition { 0 };
    auto nextNumIndex = name.find_first_of("1234567890");
    while (nextNumIndex != name.npos) {
        const auto numLetters = nextNumIndex - nextCharIndex;
        parameterPosition += numLetters;
        lettersOnlyHash = hashNoAmpersand(name.substr(nextCharIndex, numLetters), lettersOnlyHash);
        nextCharIndex = name.find_first_not_of("1234567890", nextNumIndex);

        uint32_t returnedValue;
        const auto numDigits = (nextCharIndex == name.npos) ? name.npos : nextCharIndex - nextNumIndex;
        if (absl::SimpleAtoi(name.substr(nextNumIndex, numDigits), &returnedValue)) {
            lettersOnlyHash = hash("&", lettersOnlyHash);
            parameters.push_back(returnedValue);
        }

        nextNumIndex = name.find_first_of("1234567890", nextCharIndex);
    }

    if (nextCharIndex != name.npos)
        lettersOnlyHash = hashNoAmpersand(name.substr(nextCharIndex), lettersOnlyHash);
}

static absl::string_view extractBackInteger(absl::string_view opcodeName)
{
    size_t n = opcodeName.size();
    size_t i = n;
    while (i > 0 && absl::ascii_isdigit(opcodeName[i - 1])) --i;
    return opcodeName.substr(i);
}

std::string Opcode::getLetterOnlyName() const
{
    absl::string_view name { this->name };

    std::string letterOnlyName;
    letterOnlyName.reserve(name.size());

    bool charWasDigit = false;
    for (unsigned char c : name) {
        bool charIsDigit = absl::ascii_isdigit(c);

        if (!charIsDigit)
            letterOnlyName.push_back(c);
        else if (!charWasDigit)
            letterOnlyName.push_back('&');

        charWasDigit = charIsDigit;
    }

    return letterOnlyName;
}

std::string Opcode::getDerivedName(OpcodeCategory newCategory, unsigned number) const
{
    std::string derivedName(name);

    switch (category) {
    case kOpcodeNormal:
        break;
    case kOpcodeOnCcN:
    case kOpcodeCurveCcN:
    case kOpcodeStepCcN:
    case kOpcodeSmoothCcN:
        {
            // when the input is cc, first delete the suffix `_*cc`
            size_t pos = name.rfind('_');
            ASSERT(pos != name.npos);
            derivedName.resize(pos);
        }
        break;
    }

    // helper to extract the cc number optionally if the next part needs it
    auto ccNumberSuffix = [this, number]() -> std::string {
        return (number != ~0u) ? std::to_string(number) :
            std::string(extractBackInteger(name));
    };

    switch (newCategory) {
    case kOpcodeNormal:
        break;
    case kOpcodeOnCcN:
        absl::StrAppend(&derivedName, "_oncc", ccNumberSuffix());
        break;
    case kOpcodeCurveCcN:
        absl::StrAppend(&derivedName, "_curvecc", ccNumberSuffix());
        break;
    case kOpcodeStepCcN:
        absl::StrAppend(&derivedName, "_stepcc", ccNumberSuffix());
        break;
    case kOpcodeSmoothCcN:
        absl::StrAppend(&derivedName, "_smoothcc", ccNumberSuffix());
        break;
    }

    return derivedName;
}

OpcodeCategory Opcode::identifyCategory(absl::string_view name)
{
    OpcodeCategory category = kOpcodeNormal;

    if (!name.empty() && absl::ascii_isdigit(name.back())) {
        absl::string_view part = name;
        part.remove_suffix(extractBackInteger(name).size());
        if (absl::EndsWith(part, "_oncc") || absl::EndsWith(part, "_cc"))
            category = kOpcodeOnCcN;
        else if (absl::EndsWith(part, "_curvecc"))
            category = kOpcodeCurveCcN;
        else if (absl::EndsWith(part, "_stepcc"))
            category = kOpcodeStepCcN;
        else if (absl::EndsWith(part, "_smoothcc"))
            category = kOpcodeSmoothCcN;
    }

    return category;
}

template <typename T>
absl::optional<T> transformInt_(OpcodeSpec<T> spec, int64_t v)
{
    using Limits = std::numeric_limits<T>;

    if (v > static_cast<int64_t>(spec.bounds.getEnd())) {
        if (spec.flags & kEnforceUpperBound)
            return spec.bounds.getEnd();
        else if (!(spec.flags & kPermissiveUpperBound))
            return absl::nullopt;
    } else if (v < static_cast<int64_t>(spec.bounds.getStart())) {
        if (spec.flags & kEnforceLowerBound)
            return spec.bounds.getStart();
        else if (!(spec.flags & kPermissiveLowerBound))
            return absl::nullopt;
    }

    v = std::max<int64_t>(v, Limits::min());
    v = std::min<int64_t>(v, Limits::max());

    return static_cast<T>(v);
}

template <typename T>
absl::optional<T> readInt_(OpcodeSpec<T> spec, absl::string_view v)
{
    int64_t returnedValue;
    bool readValueSuccess = false;

    if (readLeadingInt(v, &returnedValue))
        readValueSuccess = true;

    if (!readValueSuccess && (spec.flags & kCanBeNote)) {
        if (absl::optional<uint8_t> noteValue = readNoteValue(v)) {
            returnedValue = *noteValue;
            readValueSuccess = true;
        }
    }

    if (!readValueSuccess)
        return absl::nullopt;

    return transformInt_(spec, returnedValue);
}

#define INSTANTIATE_FOR_INTEGRAL(T)                             \
    template <>                                                 \
    absl::optional<T> Opcode::readOptional(OpcodeSpec<T> spec, absl::string_view value)    \
    {                                                           \
        return readInt_<T>(spec, value);                               \
    }                                                                  \
    template <>                                                         \
    absl::optional<T> Opcode::transformOptional(OpcodeSpec<T> spec, int64_t value) \
    {                                                                   \
        return transformInt_<T>(spec, value);                           \
    }

INSTANTIATE_FOR_INTEGRAL(uint8_t)
INSTANTIATE_FOR_INTEGRAL(uint16_t)
INSTANTIATE_FOR_INTEGRAL(uint32_t)
INSTANTIATE_FOR_INTEGRAL(int8_t)
INSTANTIATE_FOR_INTEGRAL(int16_t)
INSTANTIATE_FOR_INTEGRAL(int32_t)
INSTANTIATE_FOR_INTEGRAL(int64_t)


template <typename T>
absl::optional<T> transformFloat_(OpcodeSpec<T> spec, T v)
{
    if (spec.flags & kWrapPhase)
        v = wrapPhase(v);

    if (v > spec.bounds.getEnd()) {
        if (spec.flags & kEnforceUpperBound)
            return spec.bounds.getEnd();
        else if (!(spec.flags & kPermissiveUpperBound))
            return absl::nullopt;
    } else if (v < spec.bounds.getStart()) {
        if (spec.flags & kEnforceLowerBound)
            return spec.bounds.getStart();
        else if (!(spec.flags & kPermissiveLowerBound))
            return absl::nullopt;
    }

    return spec.normalizeInput(v);
}

template <typename T>
absl::optional<T> readFloat_(OpcodeSpec<T> spec, absl::string_view v)
{
    T returnedValue;
    if (!readLeadingFloat(v, &returnedValue))
        return absl::nullopt;

    return transformFloat_(spec, returnedValue);
}

#define INSTANTIATE_FOR_FLOATING_POINT(T)                       \
    template <>                                                 \
    absl::optional<T> Opcode::readOptional(OpcodeSpec<T> spec, absl::string_view value)    \
    {                                                           \
        return readFloat_<T>(spec, value);                             \
    }                                                                   \
    template <>                                                         \
    absl::optional<T> Opcode::transformOptional(OpcodeSpec<T> spec, T value) \
    {                                                                   \
        return transformFloat_<T>(spec, value);                         \
    }

INSTANTIATE_FOR_FLOATING_POINT(float)
INSTANTIATE_FOR_FLOATING_POINT(double)

absl::optional<uint8_t> readNoteValue(absl::string_view value)
{
    char noteLetter = absl::ascii_tolower(value.empty() ? '\0' : value.front());
    value.remove_prefix(1);
    if (noteLetter < 'a' || noteLetter > 'g')
        return absl::nullopt;

    constexpr int offsetsABCDEFG[] = { 9, 11, 0, 2, 4, 5, 7 };
    int noteNumber = offsetsABCDEFG[noteLetter - 'a'];

    ///
    absl::string_view validSharpLetters = "cdfga";
    absl::string_view validFlatLetters = "degab";

    ///
    std::pair<absl::string_view, int> flatSharpPrefixes[] = {
        {   "#", +1 },
        { u8"♯", +1 },
        {   "b", -1 },
        { u8"♭", -1 },
    };

    for (const auto& prefix : flatSharpPrefixes) {
        if (absl::StartsWith(value, prefix.first)) {
            if (prefix.second == +1) {
                if (validSharpLetters.find(noteLetter) == absl::string_view::npos)
                    return absl::nullopt;
            }
            else if (prefix.second == -1) {
                if (validFlatLetters.find(noteLetter) == absl::string_view::npos)
                    return absl::nullopt;
            }
            noteNumber += prefix.second;
            value.remove_prefix(prefix.first.size());
            break;
        }
    }

    int octaveNumber;
    if (!absl::SimpleAtoi(value, &octaveNumber))
        return absl::nullopt;

    noteNumber += (octaveNumber + 1) * 12;

    if (noteNumber < 0 || noteNumber >= 128)
        return absl::nullopt;

    return static_cast<uint8_t>(noteNumber);
}

absl::optional<bool> readBoolean(absl::string_view value)
{
    // Cakewalk-style booleans, case-insensitive
    if (absl::EqualsIgnoreCase(value, "off"))
        return false;

    if (absl::EqualsIgnoreCase(value, "on"))
        return true;

    // ARIA-style booleans? (seen in egN_dynamic=1 for example)
    // TODO check this
    const OpcodeSpec<int64_t> fullInt64 { 0, Range<int64_t>::wholeRange(), 0 };
    const auto v = Opcode::readOptional(fullInt64, value);

    if (v)
        return v != 0;

    return absl::nullopt;
}

template <>
absl::optional<OscillatorEnabled> Opcode::readOptional(OpcodeSpec<OscillatorEnabled>, absl::string_view value)
{
    if (value == "auto")
        return OscillatorEnabled::Auto;

    auto v = readBoolean(value);
    if (!v)
        return absl::nullopt;

    return *v ? OscillatorEnabled::On : OscillatorEnabled::Off;
}

template <class E>
absl::optional<E> transformEnum_(OpcodeSpec<E> spec, int64_t value)
{
    OpcodeSpec<int64_t> intermediateSpec;
    intermediateSpec.defaultInputValue = static_cast<int64_t>(spec.defaultInputValue);
    intermediateSpec.bounds = spec.bounds.template to<int64_t>();
    intermediateSpec.flags = static_cast<int64_t>(spec.flags);
    absl::optional<int64_t> intermediateValue = transformInt_(intermediateSpec, value);
    if (!intermediateValue)
        return absl::nullopt;
    return static_cast<E>(*intermediateValue);
}

#define INSTANTIATE_FOR_ENUM(T)                                         \
    template <>                                                         \
    absl::optional<T> Opcode::transformOptional(OpcodeSpec<T> spec, int64_t value) \
    {                                                                   \
        return transformEnum_<T>(spec, static_cast<int64_t>(value));    \
    }

template <>
absl::optional<Trigger> Opcode::readOptional(OpcodeSpec<Trigger>, absl::string_view value)
{
    switch (hash(value)) {
    case hash("attack"): return Trigger::attack;
    case hash("first"): return Trigger::first;
    case hash("legato"): return Trigger::legato;
    case hash("release"): return Trigger::release;
    case hash("release_key"): return Trigger::release_key;
    }

    DBG("Unknown trigger value: " << value);
    return absl::nullopt;
}

INSTANTIATE_FOR_ENUM(Trigger)

template <>
absl::optional<CrossfadeCurve> Opcode::readOptional(OpcodeSpec<CrossfadeCurve>, absl::string_view value)
{
    switch (hash(value)) {
    case hash("power"): return CrossfadeCurve::power;
    case hash("gain"): return CrossfadeCurve::gain;
    }

    DBG("Unknown crossfade power curve: " << value);
    return absl::nullopt;
}

INSTANTIATE_FOR_ENUM(CrossfadeCurve)

template <>
absl::optional<OffMode> Opcode::readOptional(OpcodeSpec<OffMode>, absl::string_view value)
{
    switch (hash(value)) {
    case hash("fast"): return OffMode::fast;
    case hash("normal"): return OffMode::normal;
    case hash("time"): return OffMode::time;
    }

    DBG("Unknown off mode: " << value);
    return absl::nullopt;
}

INSTANTIATE_FOR_ENUM(OffMode)

template <>
absl::optional<FilterType> Opcode::readOptional(OpcodeSpec<FilterType>, absl::string_view value)
{
    switch (hash(value)) {
    case hash("lpf_1p"): return kFilterLpf1p;
    case hash("hpf_1p"): return kFilterHpf1p;
    case hash("lpf_2p"): return kFilterLpf2p;
    case hash("hpf_2p"): return kFilterHpf2p;
    case hash("bpf_2p"): return kFilterBpf2p;
    case hash("brf_2p"): return kFilterBrf2p;
    case hash("bpf_1p"): return kFilterBpf1p;
    case hash("brf_1p"): return kFilterBrf2p; // Sforzando does this
    // case hash("apf_1p"): return kFilterApf1p; // TODO: replace with the delay-less version later
    case hash("lpf_2p_sv"): return kFilterLpf2pSv;
    case hash("hpf_2p_sv"): return kFilterHpf2pSv;
    case hash("bpf_2p_sv"): return kFilterBpf2pSv;
    case hash("brf_2p_sv"): return kFilterBrf2pSv;
    case hash("lpf_4p"): return kFilterLpf4p;
    case hash("hpf_4p"): return kFilterHpf4p;
    case hash("lpf_6p"): return kFilterLpf6p;
    case hash("hpf_6p"): return kFilterHpf6p;
    case hash("pink"): return kFilterPink;
    case hash("lsh"): return kFilterLsh;
    case hash("hsh"): return kFilterHsh;
    case hash("bpk_2p"): //fallthrough
    case hash("pkf_2p"): //fallthrough
    case hash("peq"): return kFilterPeq;
    }

    DBG("Unknown/unsupported filter type: " << value);
    return absl::nullopt;
}

INSTANTIATE_FOR_ENUM(FilterType)

template <>
absl::optional<EqType> Opcode::readOptional(OpcodeSpec<EqType>, absl::string_view value)
{
    switch (hash(value)) {
    case hash("peak"): return kEqPeak;
    case hash("lshelf"): return kEqLshelf;
    case hash("hshelf"): return kEqHshelf;
    }

    DBG("Unknown EQ type: " << value);
    return absl::nullopt;
}

INSTANTIATE_FOR_ENUM(EqType)

template <>
absl::optional<VelocityOverride> Opcode::readOptional(OpcodeSpec<VelocityOverride>, absl::string_view value)
{
    switch (hash(value)) {
    case hash("current"): return VelocityOverride::current;
    case hash("previous"): return VelocityOverride::previous;
    }

    DBG("Unknown velocity override: " << value);
    return absl::nullopt;
}

INSTANTIATE_FOR_ENUM(VelocityOverride)

template <>
absl::optional<SelfMask> Opcode::readOptional(OpcodeSpec<SelfMask>, absl::string_view value)
{
    switch (hash(value)) {
    case hash("on"):
    case hash("mask"): return SelfMask::mask;
    case hash("off"): return SelfMask::dontMask;
    }

    DBG("Unknown velocity override: " << value);
    return absl::nullopt;
}

INSTANTIATE_FOR_ENUM(SelfMask)

template <>
absl::optional<LoopMode> Opcode::readOptional(OpcodeSpec<LoopMode>, absl::string_view value)
{
    switch (hash(value)) {
    case hash("no_loop"): return LoopMode::no_loop;
    case hash("one_shot"): return LoopMode::one_shot;
    case hash("loop_continuous"): return LoopMode::loop_continuous;
    case hash("loop_sustain"): return LoopMode::loop_sustain;
    }

    DBG("Unknown loop mode: " << value);
    return absl::nullopt;
}

INSTANTIATE_FOR_ENUM(LoopMode)

template <>
absl::optional<bool> Opcode::readOptional(OpcodeSpec<bool>, absl::string_view value)
{
    return readBoolean(value);
}

template <>
absl::optional<LFOWave> Opcode::readOptional(OpcodeSpec<LFOWave> spec, absl::string_view value)
{
    const OpcodeSpec<int> intSpec {
        static_cast<int>(spec.defaultInputValue),
        Range<int>(static_cast<int>(spec.bounds.getStart()), static_cast<int>(spec.bounds.getEnd())),
        0
    };

    if (auto intValue = readOptional(intSpec, value))
        return static_cast<LFOWave>(*intValue);

    return absl::nullopt;
}

INSTANTIATE_FOR_ENUM(LFOWave)

} // namespace sfz

std::ostream &operator<<(std::ostream &os, const sfz::Opcode &opcode)
{
    return os << opcode.name << '=' << '"' << opcode.value << '"';
}
