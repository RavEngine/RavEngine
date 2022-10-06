// SPDX-License-Identifier: BSD-2-Clause

// This code is part of the sfizz library and is licensed under a BSD 2-clause
// license. You should have receive a LICENSE.md file along with the code.
// If not, contact the sfizz maintainers at https://github.com/sfztools/sfizz

#pragma once

#include "Region.h"
#include "Voice.h"
#include "Opcode.h"
#include "utility/SwapAndPop.h"
#include <vector>

namespace sfz
{

class RegionSet {
public:
    RegionSet() = delete;
    RegionSet(RegionSet* parentSet, OpcodeScope level);
    /**
     * @brief Set the polyphony limit for the set
     *
     * @param limit
     */
    void setPolyphonyLimit(unsigned limit) noexcept;
    /**
     * @brief Add a region to the set
     *
     * @param region
     */
    void addRegion(Region* region) noexcept;
    /**
     * @brief Add a subset to the set
     *
     * @param group
     */
    void addSubset(RegionSet* group) noexcept;
    /**
     * @brief Register a voice as active in this set
     *
     * @param voice
     */
    void registerVoice(Voice* voice) noexcept;
    /**
     * @brief Remove an active voice for this set.
     * If the voice was not registered this has no effect.
     *
     * @param voice
     */
    void removeVoice(const Voice* voice) noexcept;
    /**
     * @brief Register a voice in the whole parent hierarchy of the region
     *
     * @param region
     * @param voice
     */
    static void registerVoiceInHierarchy(const Region* region, Voice* voice) noexcept;
    /**
     * @brief Remove an active voice from the whole parent hierarchy of the region.
     *
     * @param region
     * @param voice
     */
    static void removeVoiceFromHierarchy(const Region* region, const Voice* voice) noexcept;
    /**
     * @brief Get the polyphony limit
     *
     * @return unsigned
     */
    unsigned getPolyphonyLimit() const noexcept { return polyphonyLimit; }
    /**
     * @brief Get the parent set
     *
     * @return RegionSet*
     */
    RegionSet* getParent() const noexcept { return parent; }

    /**
     * @brief Get the set level
     *
     * @return OpcodeScope
     */
    OpcodeScope getLevel() const noexcept { return level; }
    /**
     * @brief Set the parent set
     *
     * @param parent
     */
    void setParent(RegionSet* parent) noexcept { this->parent = parent; }
    /**
     * @brief Returns the number of playing (unreleased) voices
     */
    unsigned numPlayingVoices() const noexcept;
    /**
     * @brief Get the active voices
     *
     * @return const std::vector<Voice*>&
     */
    const std::vector<Voice*>& getActiveVoices() const noexcept { return voices; }
    /**
     * @brief Get the active voices
     *
     * @return std::vector<Voice*>&
     */
    std::vector<Voice*>& getActiveVoices() noexcept { return voices; }
    /**
     * @brief Get the regions in the set
     *
     * @return const std::vector<Region*>&
     */
    const std::vector<Region*>& getRegions() const noexcept { return regions; }
    /**
     * @brief Get the region subsets in this set
     *
     * @return const std::vector<RegionSet*>&
     */
    const std::vector<RegionSet*>& getSubsets() const noexcept { return subsets; }

    /**
     * @brief Remove all voices from the set
     */
    void removeAllVoices() noexcept;
private:
    RegionSet* parent { nullptr };
    OpcodeScope level { kOpcodeScopeGeneric };
    std::vector<Region*> regions;
    std::vector<RegionSet*> subsets;
    std::vector<Voice*> voices;
    unsigned polyphonyLimit { config::maxVoices };
};

}
