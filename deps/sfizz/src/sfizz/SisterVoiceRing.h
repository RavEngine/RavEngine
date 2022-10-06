// SPDX-License-Identifier: BSD-2-Clause

// This code is part of the sfizz library and is licensed under a BSD 2-clause
// license. You should have receive a LICENSE.md file along with the code.
// If not, contact the sfizz maintainers at https://github.com/sfztools/sfizz
#pragma once

#include "Voice.h"
#include "utility/Debug.h"
#include "absl/meta/type_traits.h"

namespace sfz
{

struct SisterVoiceRing {
    /**
     * @brief Apply a lambda function to all sisters in a ring.
     * This function should be robust enough to be able to kill the voice
     * in the lambda.
     *
     * @param voice
     * @param lambda
     */
    template<class F, class T,
        absl::enable_if_t<std::is_same<Voice, absl::remove_const_t<T>>::value, int> = 0>
    static void applyToRing(T* voice, F&& lambda) noexcept
    {
        auto v = voice->getNextSisterVoice();
        while (v != voice) {
            const auto next = v->getNextSisterVoice();
            lambda(v);
            v = next;
        }
        lambda(voice);
    }

    /**
     * @brief Count the number of sister voices in a ring
     *
     * @param start
     * @return unsigned
     */
    static unsigned countSisterVoices(const Voice* start) noexcept
    {
        if (!start)
            return 0;

        unsigned count = 0;
        auto next = start;
        do
        {
            count++;
            next = next->getNextSisterVoice();
        } while (next != start && count < config::maxVoices);

        ASSERT(count < config::maxVoices);
        return count;
    }

    /**
     * @brief Off all sisters in a ring
     *
     * @param voice
     * @param delay
     * @param fast whether to apply a fast release
     */
    template<class T,
        absl::enable_if_t<std::is_same<Voice, absl::remove_const_t<T>>::value, int> = 0>
    static void offAllSisters(T* voice, int delay, bool fast = false) {
        if (voice != nullptr) {
            SisterVoiceRing::applyToRing(voice, [&] (Voice* v) {
                v->off(delay, fast);
            });
        }
    }

    /**
     * @brief Check if a sister voice ring is well formed
     *
     * @param start
     * @return true
     * @return false
     */
    static bool checkRingValidity(const Voice* start) noexcept
    {
        if (start == nullptr)
            return true;

        unsigned idx { 0 };
        const Voice* ring[config::maxVoices];
        ring[idx] = start;
        while (idx < config::maxVoices) {
            const auto* newVoice = ring[idx]->getNextSisterVoice();

            if (newVoice == nullptr) {
                DBG("Error in ring: " << static_cast<const void*>(ring[idx])
                        << " next sister is null");
                return false;
            }

            if (newVoice->getPreviousSisterVoice() != ring[idx]) {
                DBG("Error in ring: " << static_cast<const void*>(newVoice)
                        << " refers " << static_cast<const void*>(newVoice->getPreviousSisterVoice())
                        << " as previous sister voice instead of "
                        << static_cast<const void*>(ring[idx]));
                return false;
            }

            if (newVoice == start)
                break;

            for (unsigned i = 1; i < idx; ++i) {
                if (ring[i] == newVoice) {
                    DBG("Error in ring: " << static_cast<const void*>(newVoice)
                        << " already present in ring at index " << i);
                    return false;
                }
            }
            ring[++idx] = newVoice;
        }

        return true;
    }
};

/**
 * @brief RAII helper to build sister voice rings.
 * Closes the doubly-linked list on destruction.
 *
 */
class SisterVoiceRingBuilder {
public:
    /**
     * @brief Add a voice to the sister ring
     *
     * @param voice
     */
    void addVoiceToRing(Voice* voice) noexcept {
        ASSERT(!voice->isInSisterRing());

        Voice* next = head_;
        if (!next)
            head_ = next = voice;

        Voice* previous = next->getPreviousSisterVoice();
        voice->setNextSisterVoice(next);
        voice->setPreviousSisterVoice(previous);
        next->setPreviousSisterVoice(voice);
        previous->setNextSisterVoice(voice);
    }
private:
    Voice* head_ { nullptr };
};

}
