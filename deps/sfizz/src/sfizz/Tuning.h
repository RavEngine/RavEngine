// SPDX-License-Identifier: BSD-2-Clause

// This code is part of the sfizz library and is licensed under a BSD 2-clause
// license. You should have receive a LICENSE.md file along with the code.
// If not, contact the sfizz maintainers at https://github.com/sfztools/sfizz

#pragma once
#include "ghc/fs_std.hpp"
#include <memory>

namespace sfz {

class Tuning {
public:
    Tuning();
    ~Tuning();

    /**
     * @brief Load a scale from a file in the Scala format.
     */
    bool loadScalaFile(const fs::path& path);

    /**
     * @brief Load a scale from memory in the Scala format.
     */
    bool loadScalaString(const std::string& text);

    /**
     * @brief Set the root key.
     */
    void setScalaRootKey(int rootKey);

    /**
     * @brief Get the root key.
     */
    int getScalaRootKey() const;

    /**
     * @brief Set the tuning frequency.
     */
    void setTuningFrequency(float frequency);

    /**
     * @brief Get the tuning frequency.
     */
    float getTuningFrequency() const;

    /**
     * @brief Load the equal-temperament scale.
     */
    void loadEqualTemperamentScale();

    /**
     * @brief Get the MIDI key frequency under the present tuning.
     */
    float getFrequencyOfKey(int midiKey) const;

    /**
     * @brief Get the fractional MIDI key reconverted into equal temperament scale.
     */
    float getKeyFractional12TET(int midiKey);

    /**
     * @brief Check whether the underlying scala file has changed.
     *
     */
    bool shouldReloadScala();

private:
    struct Impl;
    std::unique_ptr<Impl> impl_;
};

class StretchTuning {
public:
    float getRatioForIntegralKey(int key) const noexcept;
    float getRatioForFractionalKey(float key) const noexcept;

    static StretchTuning createFromDetuneRatios(const float detune[128]);
    static StretchTuning createRailsbackFromRatio(float stretch);

private:
    float keyDetuneRatio_[128] {};
};

} // namespace sfz
