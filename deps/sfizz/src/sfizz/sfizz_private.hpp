// SPDX-License-Identifier: BSD-2-Clause

// This code is part of the sfizz library and is licensed under a BSD 2-clause
// license. You should have receive a LICENSE.md file along with the code.
// If not, contact the sfizz maintainers at https://github.com/sfztools/sfizz

#pragma once
#include "Synth.h"
#include <atomic>

struct sfizz_synth_t {
public:
    sfizz_synth_t() : rc{1} {}

    sfizz_synth_t(const sfizz_synth_t&) = delete;
    sfizz_synth_t& operator=(const sfizz_synth_t&) = delete;
    sfizz_synth_t(sfizz_synth_t&&) = delete;
    sfizz_synth_t& operator=(sfizz_synth_t&&) = delete;

private:
    ~sfizz_synth_t() {}

public:
    void remember()
    {
        rc.fetch_add(1, std::memory_order_relaxed);
    }

    void forget()
    {
        if (rc.fetch_sub(1, std::memory_order_acq_rel) == 1)
            delete this;
    }

    sfz::Synth synth;
    std::atomic<size_t> rc;
};
