// SPDX-License-Identifier: BSD-2-Clause

// This code is part of the sfizz library and is licensed under a BSD 2-clause
// license. You should have receive a LICENSE.md file along with the code.
// If not, contact the sfizz maintainers at https://github.com/sfztools/sfizz

#pragma once
#include <absl/types/span.h>
#include <memory>

namespace sfz {
struct FlexEGDescription;
class Resources;

/**
   Flex envelope generator (according to ARIA)
 */
class FlexEnvelope {
public:
    explicit FlexEnvelope(Resources &resources);
    ~FlexEnvelope();

    /**
       Sets the sample rate.
     */
    void setSampleRate(double sampleRate);

    /**
       Attach some control parameters to this EG.
       The control structure is owned by the caller.
     */
    void configure(const FlexEGDescription* desc);

    /**
     * @brief Set the EG to be freeRunning or not
     *
     * @param freeRunning
     */
    void setFreeRunning(bool freeRunning);

    /**
       Start processing an EG as a region is triggered.
     */
    void start(unsigned triggerDelay);

    /**
       Release the EG.
     */
    void release(unsigned releaseDelay);

    /**
       Cancel the release
     */
    void cancelRelease(unsigned delay);

    /**
       Get the remaining delay samples
     */
    unsigned getRemainingDelay() const noexcept;

    /**
       Is the envelope released?
    */
    bool isReleased() const noexcept;

    /**
       Is the envelope finished?
    */
    bool isFinished() const noexcept;

    /**
       Process a cycle of the generator.
     */
    void process(absl::Span<float> out);

private:
    struct Impl;
    std::unique_ptr<Impl> impl_;
};

} // namespace sfz
