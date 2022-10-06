// SPDX-License-Identifier: BSD-2-Clause

// This code is part of the sfizz library and is licensed under a BSD 2-clause
// license. You should have receive a LICENSE.md file along with the code.
// If not, contact the sfizz maintainers at https://github.com/sfztools/sfizz

#pragma once
#include "Defaults.h"
#include "CCMap.h"
#include <vector>
#include <memory>

namespace sfz {
class Curve;
class MidiState;

namespace FlexEGs {
    std::shared_ptr<Curve> getShapeCurve(float shape);
    void clearUnusedCurves();
};

struct FlexEGPoint {
    float time { Default::flexEGPointTime }; // duration until next step (s)
    float level { Default::flexEGPointLevel }; // normalized amplitude

    CCMap<float> ccTime;
    CCMap<float> ccLevel;

    float getTime(const MidiState& state, int delay = 0) const noexcept;
    float getLevel(const MidiState& state, int delay = 0) const noexcept;

    void setShape(float shape);
    float shape() const noexcept { return shape_; }
    const Curve& curve() const;

private:
    float shape_ { Default::flexEGPointShape }; // 0: linear, positive: exp, negative: log
    std::shared_ptr<Curve> shapeCurve_;
};

struct FlexEGDescription {
    bool dynamic { Default::flexEGDynamic }; // whether parameters can be modulated while EG runs
    int sustain { Default::flexEGSustain }; // index of the sustain point (default to 0 in ARIA)
    std::vector<FlexEGPoint> points;
    // ARIA
    bool ampeg { Default::flexEGAmpeg }; // replaces the SFZv1 AmpEG (lowest with this bit wins)
};

} // namespace sfz
