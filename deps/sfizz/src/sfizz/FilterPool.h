#pragma once
#include "SfzFilter.h"
#include "Defaults.h"
#include "modulations/ModMatrix.h"
#include <vector>
#include <memory>

namespace sfz {
struct Region;
class Resources;
struct FilterDescription;

class FilterHolder
{
public:
    FilterHolder() = delete;
    FilterHolder(Resources& resources);
    /**
     * @brief Setup a new filter based on a filter description, and a triggering note parameters.
     *
     * @param description   the region from which we take the filter
     * @param filterId      the filter index in the region
     * @param noteNumber    the triggering note number
     * @param velocity      the triggering note velocity/value
     */
    void setup(const Region& region, unsigned filterId, int noteNumber = static_cast<int>(Default::key), float velocity = 0);
    /**
     * @brief Process a block of stereo inputs
     *
     * @param inputs
     * @param outputs
     * @param numFrames
     */
    void process(const float** inputs, float** outputs, unsigned numFrames);
    /**
     * @brief Set the sample rate for a filter
     *
     * @param sampleRate
     */
    void setSampleRate(float sampleRate);
    /**
     * Reset the filter.
     */
    void reset();
private:
    Resources& resources;
    const FilterDescription* description;
    std::unique_ptr<Filter> filter;
    float baseCutoff { Default::filterCutoff };
    float baseResonance { Default::filterResonance };
    float baseGain { Default::filterGain };
    ModMatrix::TargetId gainTarget;
    ModMatrix::TargetId cutoffTarget;
    ModMatrix::TargetId resonanceTarget;
    bool prepared { false };
};

} // namespace sfz
