// SPDX-License-Identifier: BSD-2-Clause

// This code is part of the sfizz library and is licensed under a BSD 2-clause
// license. You should have receive a LICENSE.md file along with the code.
// If not, contact the sfizz maintainers at https://github.com/sfztools/sfizz

#include "Fverb.h"
#include "gen/fverb.hxx"
#include "Opcode.h"
#include "Config.h"
#include "MathHelpers.h"
#include <absl/memory/memory.h>
#include <absl/strings/ascii.h>
#include <cmath>

/**
   Note(jpc): implementation status

- [x] reverb_type
- [x] reverb_dry
- [ ] reverb_dry_oncc
- [x] reverb_wet
- [ ] reverb_wet_oncc
- [x] reverb_input
- [ ] reverb_input_oncc
- [x] reverb_size
- [ ] reverb_size_oncc
- [x] reverb_predelay
- [ ] reverb_predelay_oncc
- [x] reverb_tone
- [ ] reverb_tone_oncc
- [x] reverb_damp
- [ ] reverb_damp_oncc
 */

namespace sfz {
namespace fx {

    struct Fverb::Impl {
        faustFverb dsp;

        struct Profile {
            float tailDensity; // %
            float decayAtMaxSize; // %
            float modulationFrequency; // Hz
            float modulationDepth; // ms
            float dry; // %
            float wet; // %
        };

        static const Profile largeRoom;
        static const Profile midRoom;
        static const Profile smallRoom;
        static const Profile largeHall;
        static const Profile midHall;
        static const Profile smallHall;
        static const Profile chamber;

        static double lpfCutoff(double x)
        {
            double midiPitch = 21.0 + clamp(x, 0.0, 100.0) * 1.08;
            return 440.0 * std::exp2((midiPitch - 69.0) * (1.0 / 12.0));
        }
    };

    ///
    const Fverb::Impl::Profile Fverb::Impl::largeRoom {
        80, // tail density
        65, // decay at max size
        0.6, // modulation frequency
        0.5, // modulation depth
        100, // dry
        60, // wet
    };
    const Fverb::Impl::Profile Fverb::Impl::midRoom {
        50, // tail density
        50, // decay at max size
        1.25, // modulation frequency
        0.5, // modulation depth
        100, // dry
        60, // wet
    };
    const Fverb::Impl::Profile Fverb::Impl::smallRoom {
        20, // tail density
        5, // decay at max size
        1.5, // modulation frequency
        0.5, // modulation depth
        100, // dry
        60, // wet
    };
    const Fverb::Impl::Profile Fverb::Impl::largeHall {
        80, // tail density
        90, // decay at max size
        0.275, // modulation frequency
        1.5, // modulation depth
        100, // dry
        60, // wet
    };
    const Fverb::Impl::Profile Fverb::Impl::midHall {
        50, // tail density
        75, // decay at max size
        0.5, // modulation frequency
        1.5, // modulation depth
        100, // dry
        60, // wet
    };
    const Fverb::Impl::Profile Fverb::Impl::smallHall {
        20, // tail density
        50, // decay at max size
        0.65, // modulation frequency
        1.5, // modulation depth
        100, // dry
        60, // wet
    };
    const Fverb::Impl::Profile Fverb::Impl::chamber {
        80, // tail density
        95, // decay at max size
        0.85, // modulation frequency
        1.5, // modulation depth
        100, // dry
        60, // wet
    };

    ///
    Fverb::Fverb()
        : impl_(new Impl)
    {
        Impl& impl = *impl_;
        auto& dsp = impl.dsp;

        dsp.init(config::defaultSampleRate);
    }

    Fverb::~Fverb()
    {
    }

    void Fverb::setSampleRate(double sampleRate)
    {
        Impl& impl = *impl_;
        auto& dsp = impl.dsp;

        dsp.classInit(sampleRate);
        dsp.instanceConstants(sampleRate);

        clear();
    }

    void Fverb::setSamplesPerBlock(int samplesPerBlock)
    {
        (void)samplesPerBlock;
    }

    void Fverb::clear()
    {
        Impl& impl = *impl_;
        auto& dsp = impl.dsp;

        dsp.instanceClear();
    }

    void Fverb::process(const float* const inputs[], float* const outputs[], unsigned nframes)
    {
        Impl& impl = *impl_;
        auto& dsp = impl.dsp;

        dsp.compute(nframes, const_cast<float**>(inputs), const_cast<float**>(outputs));
    }

    std::unique_ptr<Effect> Fverb::makeInstance(absl::Span<const Opcode> members)
    {
        Fverb* reverb = new Fverb;
        std::unique_ptr<Effect> fx { reverb };

        const Impl::Profile* profile = &Impl::largeHall;
        float dry { Default::effectPercent };
        float wet { Default::effectPercent };
        float input { Default::effectPercent };
        float size { Default::fverbSize };
        float predelay { Default::fverbPredelay };
        float tone { Default::fverbTone };
        float damp { Default::fverbDamp };

        for (const Opcode& opc : members) {
            switch (opc.lettersOnlyHash) {
            case hash("reverb_type"):
                {
                    std::string value = opc.value;
                    absl::AsciiStrToLower(&value);
                    if (value == "large_room")
                        profile = &Impl::largeRoom;
                    else if (value == "mid_room")
                        profile = &Impl::midRoom;
                    else if (value == "small_room")
                        profile = &Impl::smallRoom;
                    else if (value == "large_hall")
                        profile = &Impl::largeHall;
                    else if (value == "mid_hall")
                        profile = &Impl::midHall;
                    else if (value == "small_hall")
                        profile = &Impl::smallHall;
                    else if (value == "chamber")
                        profile = &Impl::chamber;
                }
                break;
            case hash("reverb_dry"):

                dry = opc.read(Default::effectPercent);
                break;
            case hash("reverb_wet"):
                wet = opc.read(Default::effectPercent);
                break;
            case hash("reverb_input"):
                input = opc.read(Default::effectPercent);
                break;
            case hash("reverb_size"):
                size = opc.read(Default::fverbSize);
                break;
            case hash("reverb_predelay"):
                predelay = opc.read(Default::fverbPredelay);
                break;
            case hash("reverb_tone"):
                tone = opc.read(Default::fverbTone);
                break;
            case hash("reverb_damp"):
                damp = opc.read(Default::fverbDamp);
                break;
            }
        }

        // NOTE(jpc) determine a range for decays 0-100. not calibrated
        const float decayMax = profile->decayAtMaxSize;
        const float decayMin = decayMax * 0.5f;

        Impl& impl = *reverb->impl_;
        faustFverb& dsp = impl.dsp;
        dsp.setPredelay(predelay * 1e3);
        dsp.setTailDensity(profile->tailDensity);
        dsp.setDecay(decayMax * size * 0.01f + decayMin * (1.0f - size * 0.01f));
        dsp.setModulatorFrequency(profile->modulationFrequency);
        dsp.setModulatorDepth(profile->modulationDepth);
        dsp.setDry(profile->dry * dry * 0.01f);
        dsp.setWet(profile->wet * wet * 0.01f);
        dsp.setInputAmount(input);
        dsp.setInputLowPassCutoff(Impl::lpfCutoff(tone));
        // NOTE(jpc): damp formula not well calibrated, but sounds ok-ish
        dsp.setDamping(Impl::lpfCutoff(100 - 0.5 * damp));

        return fx;
    }

} // namespace fx
} // namespace sfz
