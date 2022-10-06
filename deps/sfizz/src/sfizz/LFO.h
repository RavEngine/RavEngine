// SPDX-License-Identifier: BSD-2-Clause

// This code is part of the sfizz library and is licensed under a BSD 2-clause
// license. You should have receive a LICENSE.md file along with the code.
// If not, contact the sfizz maintainers at https://github.com/sfztools/sfizz

#pragma once
#include "utility/NumericId.h"
#include <absl/types/span.h>
#include <memory>

namespace sfz {
class Resources;
struct Region;

enum class LFOWave : int;
struct LFODescription;

/*
  * General

  lfoN_freq: Base frequency - Allow modulations at A-rate
  lfoN_phase: Initial phase
  lfoN_delay: Delay
  lfoN_fade: Time to fade-in
  lfoN_count: Number of repetitions - not implemented in ARIA
  lfoN_steps: Length of the step sequence - 1 to 128
  lfoN_steps_onccX: ??? TODO(jpc) seen in Rapture
  lfoN_stepX: Value of the Xth step of the sequence - -100% to +100%
  lfoN_stepX_onccY: ??? TODO(jpc) check this. override/modulate step in sequence?

    note: LFO evaluates between -1 to +1

    note: make the step sequencer override the main wave when present.
          subwaves are ARIA, step sequencer is Cakewalk, so do our own thing
          which makes the most sense.

  * Subwaveforms
    X: - #1/omitted: the main wave
       - #2-#8: a subwave

    note: if there are gaps in subwaveforms, these subwaveforms which are gaps
          will be initialized and processed.

    example: lfo1_ratio4=1.0 // instantiate implicitly the subs #2 and #3

  lfoN_wave[X]: Wave
  lfoN_offset[X]: DC offset - Add to LFO output; not affected by scale.
  lfoN_ratio[X]: Sub ratio - Frequency = (Ratio * Base Frequency)
  lfoN_scale[X]: Sub scale - Amplitude of sub
*/

class LFO {
public:
    explicit LFO(Resources& resources);
    ~LFO();

    /**
       Sets the sample rate.
     */
    void setSampleRate(double sampleRate);

    /**
       Attach some control parameters to this LFO.
       The control structure is owned by the caller.
     */
    void configure(const LFODescription* desc);

    /**
       Start processing a LFO as a region is triggered.
       Prepares the delay, phases, fade-in, etc..
     */
    void start(unsigned triggerDelay);

    /**
       Process a cycle of the oscillator.

       TODO(jpc) frequency modulations
     */
    void process(absl::Span<float> out);

private:
    /**
       Evaluate the wave at a given phase.
       Phase must be in the range 0 to 1 excluded.
     */
    template <LFOWave W>
    static float eval(float phase);

    /**
       Process the nth subwaveform, adding to the buffer.

       This definition is duplicated per each wave, a strategy to avoid a switch
       on wave type inside the frame loop.
     */
    template <LFOWave W>
    void processWave(unsigned nth, absl::Span<float> out, const float* phaseIn);

    /**
       Process a sample-and-hold subwaveform, adding to the buffer.
     */
    template <LFOWave W>
    void processSH(unsigned nth, absl::Span<float> out, const float* phaseIn);

    /**
       Process the step sequencer, adding to the buffer.
     */
    void processSteps(absl::Span<float> out, const float* phaseIn);

    /**
       Process the fade in gain, and apply it to the buffer.
     */
    void processFadeIn(absl::Span<float> out);

    /**
       Generate the phase of the N-th generator
     */
    void generatePhase(unsigned nth, absl::Span<float> phases);

private:
    struct Impl;
    std::unique_ptr<Impl> impl_;
};

} // namespace sfz
