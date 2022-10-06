// SPDX-License-Identifier: BSD-2-Clause

// This code is part of the sfizz library and is licensed under a BSD 2-clause
// license. You should have receive a LICENSE.md file along with the code.
// If not, contact the sfizz maintainers at https://github.com/sfztools/sfizz

#pragma once

namespace sfz
{

/**
 * @brief Implement a swap and pop idiom for a vector, applying an action on elements
 * fitting a condition and removing them by swapping them out with the last element of
 * the vector. This costs less than `erase` at the cost of the ordering of the vector.
 *
 * @param vector
 * @param condition which elements to remove
 * @param action what to do with them before removing
 * @return unsigned
 */
template<class T, class F, class A>
inline unsigned swapAndPopAll(std::vector<T>& vector, F&& condition, A&& action)
{
    auto it = vector.begin();
    auto sentinel = vector.rbegin();
    while (it < sentinel.base()) {
        if (condition(*it)) {
            action(*it);
            std::iter_swap(it, sentinel);
            ++sentinel;
        } else {
            ++it;
        }
    }
    auto removed = std::distance(sentinel.base(), vector.end());
    vector.resize(std::distance(vector.begin(), sentinel.base()));
    return removed;
}

/**
 * @brief Implement a swap and pop idiom for a vector, removing all the elements
 * fitting some condition by swapping them out with the last element of the vector.
 * This costs less than `erase` at the cost of the ordering of the vector.
 *
 * @param vector
 * @param condition which elements to remove
 * @return unsigned
 */
template<class T, class F>
inline unsigned swapAndPopAll(std::vector<T>& vector, F&& condition)
{
    return swapAndPopAll(vector, std::forward<F>(condition), [](T& v){ (void)v; });
}

/**
 * @brief Implement a swap and pop idiom for a vector, applying an action on the first
 * element fitting a condition and removing it by swapping it out with the last element of
 * the vector. This costs less than `erase` at the cost of the ordering of the vector.
 *
 * @param vector
 * @param condition which element to remove
 * @param action what to do with it before removing
 * @return unsigned
 */
template<class T, class F, class A>
inline bool swapAndPopFirst(std::vector<T>& vector, F&& condition, A&& action)
{
    auto it = vector.begin();
    auto sentinel = vector.rbegin();
    while (it < sentinel.base()) {
        if (condition(*it)) {
            action(*it);
            std::iter_swap(it, sentinel);
            vector.pop_back();
            return true;
        }
        ++it;
    }
    return false;
}

/**
 * @brief Implement a swap and pop idiom for a vector, removing the first element
 * fitting some condition by swapping it out with the last element of the vector.
 * This costs less than `erase` at the cost of the ordering of the vector.
 *
 * @param vector
 * @param condition which element to remove
 * @return unsigned
 */
template<class T, class F>
inline unsigned swapAndPopFirst(std::vector<T>& vector, F&& condition)
{
    return swapAndPopFirst(vector, std::forward<F>(condition), [](T& v){ (void)v; });
}

}
