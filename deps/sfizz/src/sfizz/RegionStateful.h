// SPDX-License-Identifier: BSD-2-Clause

// This code is part of the sfizz library and is licensed under a BSD 2-clause
// license. You should have receive a LICENSE.md file along with the code.
// If not, contact the sfizz maintainers at https://github.com/sfztools/sfizz

#include "Region.h"
#include "MidiState.h"
#include "Curve.h"

namespace sfz {
/**
 * @brief Get the note-related gain of the region depending on which note has been
 * pressed and at which velocity.
 *
 * @param region
 * @param noteNumber
 * @param velocity
 * @param midiState
 * @param curveSet
 * @return float
 */
float noteGain(const Region& region, int noteNumber, float velocity, const MidiState& midiState, const CurveSet& curveSet) noexcept;

/**
 * @brief Get the additional crossfade gain of the region depending on the
 * CC values
 *
 * @param region
 * @param midiState
 * @return float
 */
float crossfadeGain(const Region& region, const MidiState& midiState) noexcept;

/**
 * @brief Get the base volume of the region depending on which note has been
 * pressed to trigger the region.
 *
 * @param region
 * @param midiState
 * @param noteNumber
 * @return float
 */
float baseVolumedB(const Region& region, const MidiState& midiState, int noteNumber) noexcept;

/**
 * @brief Get the region offset in samples
 *
 * @param region
 * @param midiState
 * @return uint32_t
 */
uint64_t sampleOffset(const Region& region, const MidiState& midiState) noexcept;
/**
 * @brief Get the region delay in seconds
 *
 * @param region
 * @param midiState
 * @return float
 */
float regionDelay(const Region& region, const MidiState& midiState) noexcept;
/**
 * @brief Get the index of the sample end, either natural end or forced
 * loop.
 *
 * @param region
 * @param midiState
 * @return uint32_t
 */
uint32_t sampleEnd(const Region& region, MidiState& midiState) noexcept;

/**
 * @brief Computes the gain value related to the velocity of the note
 *
 * @return float
 */
float velocityCurve(const Region& region, float velocity, const MidiState& midiState, const CurveSet& curveSet) noexcept;

/**
 * @brief Returns the start of the loop for a given region
 *
 * @param region
 * @param midiState
 * @return uint32_t
 */
uint32_t loopStart(const Region& region, MidiState& midiState) noexcept;

/**
 * @brief Returns the end of the loop for a given region
 *
 * @param region
 * @param midiState
 * @return uint32_t
 */
uint32_t loopEnd(const Region& region, MidiState& midiState) noexcept;

/**
 * @brief Get the base pitch of the region depending on which note has been
 * pressed and at which velocity.
 *
 * @param region
 * @param noteNumber
 * @param velocity
 * @param midiState
 * @param curveSet
 * @return float
 */
float basePitchVariation(const Region& region, float noteNumber, float velocity, const MidiState& midiState, const CurveSet& curveSet) noexcept;

}
