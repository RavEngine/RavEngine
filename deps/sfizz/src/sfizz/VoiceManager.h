// SPDX-License-Identifier: BSD-2-Clause

// This code is part of the sfizz library and is licensed under a BSD 2-clause
// license. You should have receive a LICENSE.md file along with the code.
// If not, contact the sfizz maintainers at https://github.com/sfztools/sfizz

#pragma once

#include "absl/container/flat_hash_map.h"
#include "Config.h"
#include "PolyphonyGroup.h"
#include "Region.h"
#include "Resources.h"
#include "Voice.h"
#include "VoiceStealing.h"
#include <vector>

namespace sfz {

struct VoiceManager final : public Voice::StateListener
{
    /**
     * @brief The voice callback which is called during a change of state.
     */
    void onVoiceStateChanging(NumericId<Voice> id, Voice::State state) final;

    /**
     * @brief Find the voice which is associated with the given identifier.
     *
     * @param id
     * @return const Voice*
     */
    const Voice* getVoiceById(NumericId<Voice> id) const noexcept;

    /**
     * @brief Find the voice which is associated with the given identifier.
     *
     * @param id
     * @return Voice*
     */
    Voice* getVoiceById(NumericId<Voice> id) noexcept;

    /**
     * @brief Reset all voices and clear the polyphony groups
     */
    void reset();

    /**
     * @brief Check if a compatible attack voice is playing for the release region.
     *
     * @param releaseRegion
     * @return true
     * @return false
     */
    bool playingAttackVoice(const Region* releaseRegion) noexcept;

    /**
     * @brief Ensures that the polyphony groups are at least this size.
     * Call this each time a new `group=N` is given in an sfz file.
     *
     * @param groupIdx
     */
    void ensureNumPolyphonyGroups(int groupIdx) noexcept;

    /**
     * @brief Set the polyphony for a given group
     * If the number of polyphony groups is too small, it will
     * be increased.
     *
     * @param groupIdx
     * @param polyphony
     */
    void setGroupPolyphony(int groupIdx, unsigned polyphony) noexcept;

    /**
     * @brief Get a view into a given polyphony group
     *
     * @param idx
     * @return const PolyphonyGroup*
     */
    const PolyphonyGroup* getPolyphonyGroupView(int idx) noexcept;

    /**
     * @brief Clear all voices and polyphony groups.
     * Also resets the stealing algorithm to default.
     */
    void clear();

    /**
     * @brief Set the stealing algorithm
     *
     * @param algorithm
     */
    void setStealingAlgorithm(StealingAlgorithm algorithm);

    /**
     * @brief Off voices as necessary depending on the trigger event and started region
     *
     * @param region
     * @param delay
     * @param triggerEvent
     */
    void checkPolyphony(const Region* region, int delay, const TriggerEvent& triggerEvent) noexcept;

    /**
     * @brief Get the number of active voices
     *
     * @return size_t
     */
    size_t getNumActiveVoices() const { return activeVoices_.size(); }

    /**
     * @brief Get the number of polyphony groups
     *
     * @return size_t
     */
    size_t getNumPolyphonyGroups() const noexcept { return polyphonyGroups_.size(); }

    /**
     * @brief Find a voice that is not currently playing
     *
     * @return Voice*
     */
    Voice* findFreeVoice() noexcept;
    /**
     * @brief Require a number of voices from this manager.
     * In practice, the manager will handle slightly more, in order to
     * allow voices to die off upon reaching higher polyphony count.
     *
     * @param numVoices
     * @param resources
     */
    void requireNumVoices(int numVoices, Resources& resources);

private:
    int numRequiredVoices_ { config::numVoices };
    int getNumEffectiveVoices() const noexcept { return config::calculateActualVoices(numRequiredVoices_); }
    std::vector<Voice> list_;
    std::vector<Voice*> activeVoices_;
    std::vector<Voice*> temp_;
    // These are the `group=` groups where you can off voices
    absl::flat_hash_map<int, PolyphonyGroup> polyphonyGroups_;
    std::unique_ptr<VoiceStealer> stealer_ { absl::make_unique<OldestStealer>() };

    /**
     * @brief Check the region polyphony, releasing voices if necessary
     *
     * @param region
     * @param delay
     */
    void checkRegionPolyphony(const Region* region, int delay) noexcept;

    /**
     * @brief Check the note polyphony, releasing voices if necessary
     *
     * @param region
     * @param delay
     * @param triggerEvent
     */
    void checkNotePolyphony(const Region* region, int delay, const TriggerEvent& triggerEvent) noexcept;

    /**
     * @brief Check the group polyphony, releasing voices if necessary
     *
     * @param region
     * @param delay
     */
    void checkGroupPolyphony(const Region* region, int delay) noexcept;

    /**
     * @brief Check the region set polyphony at all levels, releasing voices if necessary
     *
     * @param region
     * @param delay
     */
    void checkSetPolyphony(const Region* region, int delay) noexcept;

    /**
     * @brief Check the engine polyphony, fast releasing voices if necessary
     *
     * @param delay
     */
    void checkEnginePolyphony(int delay) noexcept;

public:
    // Vector shortcuts
    typename decltype(list_)::iterator begin() { return list_.begin(); }
    typename decltype(list_)::const_iterator cbegin() const { return list_.cbegin(); }
    typename decltype(list_)::iterator end() { return list_.end(); }
    typename decltype(list_)::const_iterator cend() const { return list_.cend(); }
    typename decltype(list_)::reference operator[] (size_t n) { return list_[n]; }
    typename decltype(list_)::const_reference operator[] (size_t n) const { return list_[n]; }
};

} // namespace sfz
