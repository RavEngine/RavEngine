// SPDX-License-Identifier: BSD-2-Clause

// This code is part of the sfizz library and is licensed under a BSD 2-clause
// license. You should have receive a LICENSE.md file along with the code.
// If not, contact the sfizz maintainers at https://github.com/sfztools/sfizz

#pragma once

#include "Config.h"
#include "Region.h"
#include "Voice.h"
#include "utility/SwapAndPop.h"

namespace sfz
{
class PolyphonyGroup {
public:
    PolyphonyGroup();

    /**
     * @brief Set the polyphony limit for this polyphony group.
     *
     * @param limit
     */
    void setPolyphonyLimit(unsigned limit) noexcept;
    /**
     * @brief Register an active voice in this polyphony group.
     *
     * @param voice
     */
    void registerVoice(Voice* voice) noexcept;
    /**
     * @brief Remove a voice from this polyphony group.
     * If the voice was not registered before, this has no effect.
     *
     * @param voice
     */
    void removeVoice(const Voice* voice) noexcept;
    /**
     * @brief Remove all the voices from this polyphony group.
     */
    void removeAllVoices() noexcept;
    /**
     * @brief Get the polyphony limit for this group
     *
     * @return unsigned
     */
    unsigned getPolyphonyLimit() const noexcept { return polyphonyLimit; }
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
private:
    unsigned polyphonyLimit { config::maxVoices };
    std::vector<Voice*> voices;
};

}
