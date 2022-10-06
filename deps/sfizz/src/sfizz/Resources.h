// SPDX-License-Identifier: BSD-2-Clause

// This code is part of the sfizz library and is licensed under a BSD 2-clause
// license. You should have receive a LICENSE.md file along with the code.
// If not, contact the sfizz maintainers at https://github.com/sfztools/sfizz

#pragma once
#include "absl/types/optional.h"
#include <memory>

namespace sfz {

struct SynthConfig;
class BufferPool;
class MidiState;
class CurveSet;
class FilePool;
struct WavetablePool;
class Tuning;
class StretchTuning;
class ModMatrix;
class BeatClock;
class Metronome;

class Resources
{
public:
    Resources();
    ~Resources();

    void setSampleRate(float samplerate);
    void setSamplesPerBlock(int samplesPerBlock);
    /**
     * @brief Clear resources that are related to a currently loaded SFZ file
     *
     */
    void clearNonState();
    /**
     * @brief Clear resources that are unrelated to the currently loaded SFZ file,
     *        i.e. midi state and beat clock.
     *
     */
    void clearState();

    #define ACCESSOR_RW(Accessor, RetTy) \
        RetTy const& Accessor() const noexcept; \
        RetTy& Accessor() noexcept { return const_cast<RetTy&>(const_cast<const Resources*>(this)->Accessor()); }

    ACCESSOR_RW(getSynthConfig, SynthConfig);
    ACCESSOR_RW(getBufferPool, BufferPool);
    ACCESSOR_RW(getMidiState, MidiState);
    ACCESSOR_RW(getCurves, CurveSet);
    ACCESSOR_RW(getFilePool, FilePool);
    ACCESSOR_RW(getWavePool, WavetablePool);
    ACCESSOR_RW(getTuning, Tuning);
    ACCESSOR_RW(getStretch, absl::optional<StretchTuning>);
    ACCESSOR_RW(getModMatrix, ModMatrix);
    ACCESSOR_RW(getBeatClock, BeatClock);
    ACCESSOR_RW(getMetronome, Metronome);

    #undef ACCESSOR_RW

private:
    struct Impl;
    std::unique_ptr<Impl> impl_;
};

} // namespace sfz
