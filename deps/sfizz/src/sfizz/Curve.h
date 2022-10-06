// SPDX-License-Identifier: BSD-2-Clause

// This code is part of the sfizz library and is licensed under a BSD 2-clause
// license. You should have receive a LICENSE.md file along with the code.
// If not, contact the sfizz maintainers at https://github.com/sfztools/sfizz

#pragma once
#include "absl/types/span.h"
#include "MathHelpers.h"
#include "SfzHelpers.h"
#include <array>
#include <vector>
#include <memory>

namespace sfz
{
struct Opcode;

/**
 * @brief A value-mapping controller curve, built-in or user-defined.
 */
class Curve {
public:
    enum { NumValues = 128 };

    /**
     * @brief Compute the curve for integral x in domain [0:127]
     */
    float evalCC7(int value7) const
    {
        return _points[clamp(value7, 0, 127)];
    }
    /**
     * @brief Compute the curve for real x in domain [0:127]
     */
    float evalCC7(float value7) const
    {
        value7 = clamp(value7, 0.0f, 127.0f);
        int i1 = static_cast<int>(value7);
        int i2 = std::min(127, i1 + 1);
        float mu = value7 - i1;
        return _points[i1] + mu * (_points[i2] - _points[i1]);
    }

    /**
     * @brief Compute the curve for real x in domain [0:1]
     */
    float evalNormalized(float value) const
    {
        return evalCC7(value * 127.0f);
    }

    /**
     * @brief Kind of curve interpolator
     */
    enum class Interpolator {
        Linear,
        Spline,
    };

    /**
     * @brief Build a curve based on contents of a <curve> header
     *
     * @param members contents of the <curve> header
     * @param itp kind of interpolator to fill between values
     * @param limit whether to force values in domain [-1:+1]
     */
    static Curve buildCurveFromHeader(
        absl::Span<const Opcode> members,
        Interpolator itp = Interpolator::Linear, bool limit = false);

    /**
     * @brief Build a curve based on sfz v1 "amp_velcurve_&" points
     *
     * @param points the points vector
     * @param itp kind of interpolator to fill between values
     * @param invert whether to invert the curve
     */
    static Curve buildFromVelcurvePoints(
        absl::Span<const std::pair<uint8_t, float>> points,
        Interpolator itp = Interpolator::Linear, bool invert = false);

    /**
     * @brief Number of predefined curves
     */
    enum { NumPredefinedCurves = 7 };

    /**
     * @brief Build a predefined curve
     */
    static Curve buildPredefinedCurve(int index);

    /**
     * @brief Build a linear curve from v1 to v2
     */
    static Curve buildBipolar(float v1, float v2);

    /**
     * @brief Build a curve from a table of points
     */
    static Curve buildFromPoints(const float points[NumValues]);

    /**
     * @brief Get a linear curve from 0 to 1
     */
    static const Curve& getDefault();

private:
    void fill(Interpolator itp, const bool fillStatus[NumValues]);
    void lerpFill(const bool fillStatus[NumValues]);
    void splineFill(const bool fillStatus[NumValues]);

private:
    std::array<float, NumValues> _points {{}};
};

/**
 * @brief A collection of curves organized by index.
 */
class CurveSet {
public:
    /*
     * @brief Create a curve set initialized with the default curves.
     */
    static CurveSet createPredefined();

    /*
     * @brief Add a curve.
     *
     * @param curve a curve to add
     * @param explicitIndex if not -1, the explicit index of the curve
     */
    void addCurve(const Curve& curve, int explicitIndex = -1);

    /**
     * @brief Add a curve based on contents of a <curve> header
     *
     * @param members contents of the <curve> header
     */
    void addCurveFromHeader(absl::Span<const Opcode> members);

    /**
     * @brief Get a curve given its index
     */
    const Curve& getCurve(unsigned index) const;

    /**
     * @brief Get the number of slots.
     */
    unsigned getNumCurves() const
    {
        return static_cast<unsigned>(_curves.size());
    }

private:
    std::vector<std::unique_ptr<Curve>> _curves;
    bool _useExplicitIndexing = false;
};

} // namespace sfz
