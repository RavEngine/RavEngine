// SPDX-License-Identifier: BSD-2-Clause

// This code is part of the sfizz library and is licensed under a BSD 2-clause
// license. You should have receive a LICENSE.md file along with the code.
// If not, contact the sfizz maintainers at https://github.com/sfztools/sfizz

#include "Curve.h"
#include "Opcode.h"
#include "SIMDHelpers.h"
#include "utility/Debug.h"
#include <spline/spline.h>
#include <cmath>

namespace sfz
{

static const Curve defaultCurve = Curve::buildBipolar(0.0, 1.0);

Curve Curve::buildCurveFromHeader(
    absl::Span<const Opcode> members, Interpolator itp, bool limit)
{
    Curve curve;
    bool fillStatus[NumValues] = {};
    const OpcodeSpec<float> fullRange {0.0f, Range<float>{ -1e16, 1e16 }, 0 };
    auto setPoint = [&curve, &fillStatus](int i, float x) {
        curve._points[i] = x;
        fillStatus[i] = true;
    };

    // fill curve ends with default values (verified)
    setPoint(0, 0.0);
    setPoint(NumValues - 1, 1.0);

    for (const Opcode& opc : members) {
        if (opc.lettersOnlyHash != hash("v&"))
            continue;

        unsigned index = opc.parameters.back();
        if (index >= NumValues)
            continue;

        setPoint(static_cast<int>(index), opc.read(fullRange));
    }

    curve.fill(itp, fillStatus);

    if (limit) {
        for (unsigned i = 0; i < NumValues; ++i)
            curve._points[i] = clamp(curve._points[i], -1.0f, +1.0f);
    }

    return curve;
}

Curve Curve::buildFromVelcurvePoints(
    absl::Span<const std::pair<uint8_t, float>> points,
    Interpolator itp, bool invert)
{
    Curve curve;
    bool fillStatus[NumValues] = {};

    auto setPoint = [&curve, &fillStatus](int i, float x) {
        curve._points[i] = x;
        fillStatus[i] = true;
    };

    if (invert) {
        setPoint(0, 1.0);
        setPoint(NumValues - 1, 0.0);
    } else {
        setPoint(0, 0.0);
        setPoint(NumValues - 1, 1.0);
    }

    for (const auto& point: points) {
        setPoint(point.first, point.second);
    }

    curve.fill(itp, fillStatus);
    return curve;
}

Curve Curve::buildPredefinedCurve(int index)
{
    Curve curve;

    switch (index) {
    default:
        ASSERTFALSE;
        // fallthrough
    case 0:
        curve = buildBipolar(0, 1);
        break;
    case 1:
        curve = buildBipolar(-1, +1);
        break;
    case 2:
        curve = buildBipolar(1, 0);
        break;
    case 3:
        curve = buildBipolar(+1, -1);
        break;
    case 4:
        for (unsigned i = 0; i < NumValues; ++i) {
            double x = i / static_cast<double>(NumValues - 1);
            curve._points[i] = x * x;
        }
        break;
    case 5:
        curve._points[0] = 0.0;
        curve._points[NumValues - 1] = 1.0;
        for (unsigned i = 1; i < NumValues - 1; ++i) {
            double x = i / static_cast<double>(NumValues - 1);
            curve._points[i] = std::sqrt(x);
        }
        break;
    case 6:
        curve._points[0] = 1.0;
        curve._points[NumValues - 1] = 0.0;
        for (unsigned i = 1; i < NumValues - 1; ++i) {
            double x = i / static_cast<double>(NumValues - 1);
            curve._points[i] = std::sqrt(1.0 - x);
        }
        break;
    }

    return curve;
}

Curve Curve::buildBipolar(float v1, float v2)
{
    Curve curve;
    bool fillStatus[NumValues] = {};

    curve._points[0] = v1;
    curve._points[NumValues - 1] = v2;

    fillStatus[0] = true;
    fillStatus[NumValues - 1] = true;

    curve.lerpFill(fillStatus);
    return curve;
}

Curve Curve::buildFromPoints(const float points[NumValues])
{
    Curve curve;
    copy(absl::MakeConstSpan(points, NumValues), absl::Span<float>(curve._points));
    return curve;
}

const Curve& Curve::getDefault()
{
    return defaultCurve;
}

void Curve::fill(Interpolator itp, const bool fillStatus[NumValues])
{
    switch (itp) {
    default:
    case Interpolator::Linear:
        lerpFill(fillStatus);
        break;
    case Interpolator::Spline:
        splineFill(fillStatus);
        break;
    }
}

void Curve::lerpFill(const bool fillStatus[NumValues])
{
    int left { 0 };
    int right { 1 };
    auto pointSpan = absl::MakeSpan(_points);

    while (right < NumValues) {
        for (; right < NumValues && !fillStatus[right]; ++right);
        const auto length = right - left;
        if (length > 1) {
            const float mu = (_points[right] - _points[left]) / length;
            linearRamp<float>(pointSpan.subspan(left, length), _points[left], mu);
        }
        left = right++;
    }
}

void Curve::splineFill(const bool fillStatus[NumValues])
{
    std::array<double, NumValues> x;
    std::array<double, NumValues> y;
    int count = 0;

    for (unsigned i = 0; i < NumValues; ++i) {
        if (fillStatus[i]) {
            x[count] = i;
            y[count] = _points[i];
            ++count;
        }
    }

    if (count < 3)
        return lerpFill(fillStatus);

    Spline spline(x.data(), y.data(), count);
    for (unsigned i = 0; i < NumValues; ++i) {
        if (!fillStatus[i])
            _points[i] = spline.interpolate(i);
    }
}

///
CurveSet CurveSet::createPredefined()
{
    CurveSet cs;
    cs._curves.reserve(16);

    for (int i = 0; i < Curve::NumPredefinedCurves; ++i) {
        Curve curve = Curve::buildPredefinedCurve(i);
        cs._curves.emplace_back(new Curve(curve));
    }

    return cs;
}

void CurveSet::addCurve(const Curve& curve, int explicitIndex)
{
    std::unique_ptr<Curve>* slot;

    if (explicitIndex < -1)
        return;

    if (explicitIndex >= config::maxCurves)
        return;

    if (explicitIndex == -1) {
        if (_useExplicitIndexing)
            return; // reject implicit indices if any were explicit before
        _curves.emplace_back();
        slot = &_curves.back();
    } else {
        size_t index = static_cast<unsigned>(explicitIndex);
        if (index >= _curves.size())
            _curves.resize(index + 1);
        _useExplicitIndexing = true;
        slot = &_curves[index];
    }

    slot->reset(new Curve(curve));
}

void CurveSet::addCurveFromHeader(absl::Span<const Opcode> members)
{
    auto findOpcode = [members](uint64_t name_hash) -> const Opcode* {
        const Opcode* opc = nullptr;
        for (size_t i = members.size(); !opc && i-- > 0;) {
            if (members[i].lettersOnlyHash == name_hash)
                opc = &members[i];
        }
        return opc;
    };

    int curveIndex = -1;
    Curve::Interpolator itp = Curve::Interpolator::Linear;

    if (const Opcode* opc = findOpcode(hash("curve_index")))
        curveIndex = opc->read(Default::curveCC);

#if 0 // potential sfizz extension
    if (const Opcode* opc = findOpcode(hash("sfizz:curve_interpolator"))) {
        if (opc->value == "spline")
            itp = Curve::Interpolator::Spline;
        else if (opc->value != "linear")
            DBG("Invalid value for curve interpolator: " << opc.value);
    }
#endif

    addCurve(Curve::buildCurveFromHeader(members, itp), curveIndex);
}

const Curve& CurveSet::getCurve(unsigned index) const
{
    const Curve* curve = nullptr;

    if (index < _curves.size())
        curve = _curves[index].get();

    if (!curve)
        curve = &Curve::getDefault();

    return *curve;
}

} // namespace sfz
