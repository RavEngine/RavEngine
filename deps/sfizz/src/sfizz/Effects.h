// SPDX-License-Identifier: BSD-2-Clause

// This code is part of the sfizz library and is licensed under a BSD 2-clause
// license. You should have receive a LICENSE.md file along with the code.
// If not, contact the sfizz maintainers at https://github.com/sfztools/sfizz

#pragma once
#include "AudioBuffer.h"
#include "Defaults.h"
#include "absl/strings/string_view.h"
#include "absl/types/span.h"
#include <array>
#include <vector>
#include <memory>

namespace sfz {
struct Opcode;

enum {
    // Number of channels processed by effects
    EffectChannels = 2,
};

/**
   @brief Abstract base of SFZ effects
 */
class Effect {
public:
    virtual ~Effect() {}

    /**
       @brief Initializes with the given sample rate.
     */
    virtual void setSampleRate(double sampleRate) = 0;

    /**
     * @brief Sets the maximum number of frames to render at a time. The actual
     * value can be lower but should never be higher.
     */
    virtual void setSamplesPerBlock(int samplesPerBlock) = 0;

    /**
       @brief Reset the state to initial.
     */
    virtual void clear() = 0;

    /**
       @brief Computes a cycle of the effect in stereo.
     */
    virtual void process(const float* const inputs[], float* const outputs[], unsigned nframes) = 0;

    /**
       @brief Type of the factory function used to instantiate an effect given
              the contents of the <effect> block
     */
    typedef std::unique_ptr<Effect>(MakeInstance)(absl::Span<const Opcode> members);
};

/**
   @brief SFZ effects factory
 */
class EffectFactory {
public:
    /**
       @brief Registers all available standard effects into the factory.
     */
    void registerStandardEffectTypes();

    /**
       @brief Registers a user-defined effect into the factory.
     */
    void registerEffectType(absl::string_view name, Effect::MakeInstance& make);

    /**
       @brief Instantiates an effect given the contents of the <effect> block.
     */
    std::unique_ptr<Effect> makeEffect(absl::Span<const Opcode> members);

private:
    struct FactoryEntry {
        std::string name;
        Effect::MakeInstance* make;
    };

    std::vector<FactoryEntry> _entries;
};

/**
   @brief Sequence of effects processed in series
 */
class EffectBus {
public:
    EffectBus();
    ~EffectBus();

    /**
       @brief Adds an effect at the end of the bus.
     */
    void addEffect(std::unique_ptr<Effect> fx);

    /**
     * @brief Get a view into an effect in the chain
     *
     * @param index
     * @return const Effect*
     */
    const Effect* effectView(unsigned index) const;

    /**
       @brief Checks whether this bus can produce output.
     */
    bool hasNonZeroOutput() const { return (_gainToMain != 0 || _gainToMix != 0); }

    /**
       @brief Sets the amount of effect output going to the main.
     */
    void setGainToMain(float gain) { _gainToMain = gain; }

    /**
       @brief Sets the amount of effect output going to the mix.
     */
    void setGainToMix(float gain) { _gainToMix = gain; }

    /**
     * @brief Returns the gain for the main out
     *
     * @return float
     */
    float gainToMain() const { return _gainToMain; }

    /**
     * @brief Returns the gain for the mix out
     *
     * @return float
     */
    float gainToMix() const { return _gainToMix; }

    /**
       @brief Resets the input buffers to zero.
     */
    void clearInputs(unsigned nframes);

    /**
       @brief Adds some audio into the input buffer.
     */
    void addToInputs(const float* const addInput[], float addGain, unsigned nframes);

    /**
       @brief Apply a gain to the inputs
     */
    void applyGain(const float* gain, unsigned nframes);

    /**
       @brief Initializes all effects in the bus with the given sample rate.
     */
    void setSampleRate(double sampleRate);

    /**
       @brief Resets the state of all effects in the bus.
     */
    void clear();

    /**
       @brief Computes a cycle of the effect bus.
     */
    void process(unsigned nframes);

    /**
       @brief Mixes the outputs into a pair of stereo signals: Main and Mix.
     */
    void mixOutputsTo(float* const mainOutput[], float* const mixOutput[], unsigned nframes);

    /**
     * @brief Sets the maximum number of frames to render at a time. The actual value can be lower
     * but should never be higher.
     *
     */
    void setSamplesPerBlock(int samplesPerBlock) noexcept;

    /**
     * @brief Return the number of effects in the bus
     *
     * @return size_t
     */
    size_t numEffects() const noexcept;
private:
    std::vector<std::unique_ptr<Effect>> _effects;
    AudioBuffer<float> _inputs { EffectChannels, config::defaultSamplesPerBlock };
    AudioBuffer<float> _outputs { EffectChannels, config::defaultSamplesPerBlock };
    float _gainToMain { Default::effect };
    float _gainToMix { Default::effect };
};

} // namespace sfz
