// SPDX-License-Identifier: BSD-2-Clause

// This code is part of the sfizz library and is licensed under a BSD 2-clause
// license. You should have receive a LICENSE.md file along with the code.
// If not, contact the sfizz maintainers at https://github.com/sfztools/sfizz

#pragma once
#include "SIMDConfig.h"
#include <absl/types/span.h>
#include <memory>
#include <simde/simde-features.h>
#if SIMDE_NATURAL_VECTOR_SIZE_GE(128)
#include <simde/x86/sse.h>
#endif

namespace sfz {

namespace WindowedSincDetail {
    void calculateTable(absl::Span<float> table, size_t sincExtent, double beta, size_t extra);
    double calculateExact(double x, size_t sincExtent, double beta);
};

template <class T>
class AbstractWindowedSinc {
protected:
    explicit AbstractWindowedSinc(double beta) noexcept : beta_(beta) {}

public:
    virtual ~AbstractWindowedSinc() noexcept {}

    // interpolate f(x), where x must be in domain [-Points/2:+Points/2]
    float getUnchecked(float x) const noexcept;

#if SIMDE_NATURAL_VECTOR_SIZE_GE(128)
    // interpolate f(x), 4 values at once
    simde__m128 getUncheckedX4(simde__m128 x) const noexcept;
#endif

    // calculate exact f(x), where x must be in domain [-Points/2:+Points/2]
    double getExact(double x) const noexcept;

    // get the Kaiser window Beta parameter
    double getBeta() const noexcept { return beta_; }

protected:
    void fillTable() noexcept;

    // allows interpolating f(Points/2), and SSE, provided for safety
    enum { TableExtra = 4 };

private:
    double beta_ {};
};

/**
 * @brief Windowed-sinc using fixed compile-time parameters
 * This can help to save some instructions in a resampler loop.
 */
template <size_t Points, size_t TableSize>
class FixedWindowedSinc final :
        public AbstractWindowedSinc<FixedWindowedSinc<Points, TableSize>> {
public:
    using Self = FixedWindowedSinc<Points, TableSize>;
    using Super = AbstractWindowedSinc<Self>;

    explicit FixedWindowedSinc(double beta) : Super(beta) { Super::fillTable(); }

    // the number of points where this sinc will be evaluated (zero crossings + 1)
    static constexpr size_t getNumPoints() noexcept { return Points; }

    // the size of the lookup table
    static constexpr size_t getTableSize() noexcept { return TableSize; }

    // the lookup table
    const float* getTablePointer() const noexcept { return table_; }

    // the lookup table
    absl::Span<const float> getTableSpan() const noexcept { return absl::MakeConstSpan(table_, TableSize); }

protected:
    using Super::TableExtra;

private:
    float table_[TableSize + TableExtra];
};

/**
 * @brief Windowed-sinc using run-time parameters
 */
class WindowedSinc final : public AbstractWindowedSinc<WindowedSinc>
{
public:
    using Self = WindowedSinc;
    using Super = AbstractWindowedSinc<WindowedSinc>;

    WindowedSinc(size_t points, size_t tableSize, double beta)
        : Super(beta), points_(points), tableSize_(tableSize),
          table_(new float[tableSize + TableExtra])
    { Super::fillTable(); }

    // the number of points where this sinc will be evaluated (zero crossings + 1)
    size_t getNumPoints() const noexcept { return points_; }

    // the size of the lookup table
    size_t getTableSize() const noexcept { return tableSize_; }

    // the lookup table
    const float* getTablePointer() const noexcept { return table_.get(); }

    // the lookup table
    absl::Span<const float> getTableSpan() const noexcept { return absl::MakeConstSpan(table_.get(), tableSize_); }

private:
    size_t points_ {};
    size_t tableSize_ {};
    std::unique_ptr<float[]> table_;
};

} // namespace sfz

#include "WindowedSinc.hpp"
