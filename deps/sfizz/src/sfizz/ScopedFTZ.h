// SPDX-License-Identifier: BSD-2-Clause

// This code is part of the sfizz library and is licensed under a BSD 2-clause
// license. You should have receive a LICENSE.md file along with the code.
// If not, contact the sfizz maintainers at https://github.com/sfztools/sfizz

#pragma once
#include <cstdint>

/**
 * @brief Flush floating points to zero and disable denormals as an RAII helper.
 *
 */
class ScopedFTZ {

public:
    ScopedFTZ();
    ~ScopedFTZ();
private:
    uintptr_t registerState;
};
