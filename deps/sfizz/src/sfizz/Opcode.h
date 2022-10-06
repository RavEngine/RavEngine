// SPDX-License-Identifier: BSD-2-Clause

// This code is part of the sfizz library and is licensed under a BSD 2-clause
// license. You should have receive a LICENSE.md file along with the code.
// If not, contact the sfizz maintainers at https://github.com/sfztools/sfizz

#pragma once
#include "Defaults.h"
#include "Range.h"
#include "SfzHelpers.h"
#include "utility/LeakDetector.h"
#include "utility/StringViewHelpers.h"
#include "absl/types/optional.h"
#include "absl/meta/type_traits.h"
#include "absl/strings/ascii.h"
#include "absl/strings/string_view.h"
#include <vector>
#include <type_traits>
#include <iosfwd>

// charconv support is still sketchy with clang/gcc so we use abseil's numbers
#include "absl/strings/numbers.h"

namespace sfz {
/**
 * @brief A category which an opcode may belong to.
 */
enum OpcodeCategory {
    //! An ordinary opcode
    kOpcodeNormal,
    //! A region opcode which matches *_onccN or *_ccN
    kOpcodeOnCcN,
    //! A region opcode which matches *_curveccN
    kOpcodeCurveCcN,
    //! A region opcode which matches *_stepccN
    kOpcodeStepCcN,
    //! A region opcode which matches *_smoothccN
    kOpcodeSmoothCcN,
};

/**
 * @brief A scope where an opcode may appear.
 */
enum OpcodeScope {
    //! unknown scope or other
    kOpcodeScopeGeneric = 0,
    //! global scope
    kOpcodeScopeGlobal,
    //! control scope
    kOpcodeScopeControl,
    //! Master scope
    kOpcodeScopeMaster,
    //! group scope
    kOpcodeScopeGroup,
    //! region scope
    kOpcodeScopeRegion,
    //! effect scope
    kOpcodeScopeEffect,
};

/**
 * @brief Opcode description class. The class parses the parameters
 * of the opcode on construction.
 *
 */
struct Opcode {
    Opcode() = delete;
    Opcode(absl::string_view inputOpcode, absl::string_view inputValue);
    std::string name {};
    std::string value {};
    uint64_t lettersOnlyHash { Fnv1aBasis };
    // This is to handle the integer parameters of some opcodes
    std::vector<uint16_t> parameters;
    OpcodeCategory category;

    /*
     * @brief Normalize in order to make the ampersand-name unique, and
     * facilitate subsequent processing.
     *
     * @param scope scope where this opcode appears
     * @return normalized opcode
     */
    Opcode cleanUp(OpcodeScope scope) const;

    /**
     * @brief Calculate a letter-only name, replacing any digit sequence with
     * in the opcode name with a single ampersand character.
     */
    std::string getLetterOnlyName() const;

    /*
     * @brief Get the derived opcode name to convert it to another category.
     *
     * @param newCategory category to convert to
     * @param number optional CC number, needed if destination is CC and source is not
     * @return derived opcode name
     */
    std::string getDerivedName(OpcodeCategory newCategory, unsigned number = ~0u) const;

    /**
     * @brief Get whether the opcode categorizes as `ccN` of any kind.
     * @return true if `ccN`, otherwise false
     */
    bool isAnyCcN() const
    {
        return category == kOpcodeOnCcN || category == kOpcodeCurveCcN ||
            category == kOpcodeStepCcN || category == kOpcodeSmoothCcN;
    }

    ///
    template <class T>
    absl::optional<T> readOptional(OpcodeSpec<T> spec) const { return readOptional(spec, value); }

    template <class T>
    T read(OpcodeSpec<T> spec) const { return readOptional(spec, value).value_or(spec); }

    ///
    template <class T>
    static absl::optional<T> readOptional(OpcodeSpec<T> spec, absl::string_view value);

    template <class T>
    static T read(OpcodeSpec<T> spec, absl::string_view value) { return readOptional(spec, value).value_or(spec); }

    ///
    template <class T> using Intermediate = typename OpcodeSpec<T>::Intermediate;

    template <class T>
    static absl::optional<T> transformOptional(OpcodeSpec<T> spec, Intermediate<T> value);

    template <class T>
    static T transform(OpcodeSpec<T> spec, Intermediate<T> value) { return transformOptional(spec, value).value_or(spec); }

private:
    static OpcodeCategory identifyCategory(absl::string_view name);
    LEAK_DETECTOR(Opcode);
};

/**
 * @brief Convert a note in string to its equivalent midi note number
 *
 * @param value
 * @return absl::optional<uint8_t>
 */
absl::optional<uint8_t> readNoteValue(absl::string_view value);

/**
 * @brief Read a boolean value from the sfz file and cast it to the destination parameter.
 */
absl::optional<bool> readBoolean(absl::string_view value);

}

std::ostream &operator<<(std::ostream &os, const sfz::Opcode &opcode);
