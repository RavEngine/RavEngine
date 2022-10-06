// SPDX-License-Identifier: BSD-2-Clause

// This code is part of the sfizz library and is licensed under a BSD 2-clause
// license. You should have receive a LICENSE.md file along with the code.
// If not, contact the sfizz maintainers at https://github.com/sfztools/sfizz

#include "FlexEGDescription.h"
#include "Curve.h"
#include "MidiState.h"
#include <absl/container/flat_hash_map.h>
#include <cmath>

namespace sfz {

void FlexEGPoint::setShape(float shape)
{
    shape_ = shape;
    shapeCurve_ = FlexEGs::getShapeCurve(shape);
}

const Curve& FlexEGPoint::curve() const
{
    if (shapeCurve_)
        return *shapeCurve_;
    else
        return Curve::getDefault();
}

///
typedef absl::flat_hash_map<float, std::weak_ptr<Curve>> FlexEGShapes;

static FlexEGShapes& getShapeMap()
{
    static FlexEGShapes shapes;
    return shapes;
}

std::shared_ptr<Curve> FlexEGs::getShapeCurve(float shape)
{
    static FlexEGShapes& map = getShapeMap();

    std::weak_ptr<Curve>& slot = map[shape];

    std::shared_ptr<Curve> curve = slot.lock();
    if (curve)
        return curve;

    curve.reset(new Curve);

    ///
    constexpr unsigned numPoints = Curve::NumValues;
    float points[numPoints];

    if (shape == 0)
        *curve = Curve::getDefault();
    else if (shape > 0) {
        for (unsigned i = 0; i < numPoints; ++i) {
            float x = float(i) / (numPoints - 1);
            points[i] = std::pow(x, shape);
        }
        *curve = Curve::buildFromPoints(points);
    }
    else if (shape < 0) {
        for (unsigned i = 0; i < numPoints; ++i) {
            float x = float(i) / (numPoints - 1);
            points[i] = 1 - std::pow(1 - x, -shape);
        }
        *curve = Curve::buildFromPoints(points);
    }

    ///
    slot = curve;
    return curve;
}

void FlexEGs::clearUnusedCurves()
{
    static FlexEGShapes& map = getShapeMap();

    for (auto it = map.begin(); it != map.end(); ) {
        if (it->second.use_count() == 0)
            map.erase(it++);
        else
            ++it;
    }
}

///
float FlexEGPoint::getTime(const MidiState& state, int delay) const noexcept
{
    float returnedValue { time };
    for (const CCData<float>& mod : ccTime)
        returnedValue += state.getCCValueAt(mod.cc, delay) * mod.data;
    return returnedValue;
}

float FlexEGPoint::getLevel(const MidiState& state, int delay) const noexcept
{
    float returnedValue { level };
    for (const CCData<float>& mod : ccLevel)
        returnedValue += state.getCCValueAt(mod.cc, delay) * mod.data;
    return returnedValue;
}

} // namespace sfz
