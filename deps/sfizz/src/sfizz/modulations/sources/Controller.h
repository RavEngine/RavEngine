// SPDX-License-Identifier: BSD-2-Clause

// This code is part of the sfizz library and is licensed under a BSD 2-clause
// license. You should have receive a LICENSE.md file along with the code.
// If not, contact the sfizz maintainers at https://github.com/sfztools/sfizz

#pragma once
#include "../ModGenerator.h"
#include "../../VoiceManager.h"
#include <memory>

namespace sfz {

class Resources;

class ControllerSource : public ModGenerator {
public:
    explicit ControllerSource(Resources& res, VoiceManager& manager);
    ~ControllerSource();
    void setSampleRate(double sampleRate) override;
    void setSamplesPerBlock(unsigned count) override;
    void init(const ModKey& sourceKey, NumericId<Voice> voiceId, unsigned delay) override;
    void generate(const ModKey& sourceKey, NumericId<Voice> voiceId, absl::Span<float> buffer) override;

    /**
     * @brief Reset the smoothers.
     */
    void resetSmoothers();
private:
    struct Impl;
    std::unique_ptr<Impl> impl_;
};

} // namespace sfz
