// SPDX-License-Identifier: BSD-2-Clause

// This code is part of the sfizz library and is licensed under a BSD 2-clause
// license. You should have receive a LICENSE.md file along with the code.
// If not, contact the sfizz maintainers at https://github.com/sfztools/sfizz

#pragma once
#include "SfzHelpers.h"
#include "utility/LeakDetector.h"
#include <vector>
#include <absl/algorithm/container.h>
#include <absl/types/optional.h>

namespace sfz {
/**
 * @brief A simple map that holds ValueType elements at different indices, and can return a default one
 * if not present. Used mostly for CC modifiers in region descriptions as to store only the CC modifiers
 * that are specified in the SFZ file rather than a gazillion of dummy "disabled" modifiers. The default
 * value is set on construction.
 *
 * @tparam ValueType The type held in the map
 */
template <class ValueType>
class CCMap {
public:
    CCMap()
        : defaultValue(ValueType {})
    {
    }
    /**
     * @brief Construct a new CCMap object with the specified default value.
     *
     * @param defaultValue
     */
    CCMap(const ValueType& defaultValue)
        : defaultValue(defaultValue)
    {
    }
    CCMap(CCMap&&) = default;
    CCMap(const CCMap&) = default;
    ~CCMap() = default;
    CCMap& operator=(CCMap&&) = default;
    CCMap& operator=(const CCMap&) = default;

    /**
     * @brief Returns the held object at the index, or a default value if not present
     *
     * @param index
     * @return const ValueType&
     */
    const ValueType& getWithDefault(int index) const noexcept
    {
        auto it = absl::c_lower_bound(container, index, CCDataComparator<ValueType> {});
        if (it == container.end() || it->cc != index) {
            return defaultValue;
        } else {
            return it->data;
        }
    }

    /**
     * @brief Returns the held object at the index, or a default value if not present
     *
     * @param index
     * @return const ValueType&
     */
    absl::optional<ValueType> get(int index) const noexcept
    {
        auto it = absl::c_lower_bound(container, index, CCDataComparator<ValueType> {});
        if (it == container.end() || it->cc != index) {
            return {};
        } else {
            return it->data;
        }
    }

    /**
     * @brief Get the value at index or emplace a new one if not present
     *
     * @param index the index of the element
     * @return ValueType&
     */
    ValueType& operator[](const int& index) noexcept
    {
        auto it = absl::c_lower_bound(container, index, CCDataComparator<ValueType> {});
        if (it == container.end() || it->cc != index) {
            auto inserted = container.insert(it, { index, defaultValue });
            return inserted->data;
        } else {
            return it->data;
        }
    }

    /**
     * @brief Is the container empty
     *
     * @return true
     * @return false
     */
    inline bool empty() const { return container.empty(); }
    /**
     * @brief Returns true if the container containers an element at index
     *
     * @param index
     * @return true
     * @return false
     */
    bool contains(int index) const noexcept
    {
        return absl::c_binary_search(container, index, CCDataComparator<ValueType> {});
    }

    /**
     * @brief Container size
     *
     * @return size_t
     */
    size_t size() const noexcept { return container.size(); }

    typename std::vector<CCData<ValueType>>::const_iterator begin() const { return container.cbegin(); }
    typename std::vector<CCData<ValueType>>::const_iterator end() const { return container.cend(); }

private:
    // typename std::vector<std::pair<int, ValueType>>::iterator begin() { return container.begin(); }
    // typename std::vector<std::pair<int, ValueType>>::iterator end() { return container.end(); }

    ValueType defaultValue;
    std::vector<CCData<ValueType>> container;
    LEAK_DETECTOR(CCMap);
};
}
