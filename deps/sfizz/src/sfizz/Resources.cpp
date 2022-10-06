// SPDX-License-Identifier: BSD-2-Clause

// This code is part of the sfizz library and is licensed under a BSD 2-clause
// license. You should have receive a LICENSE.md file along with the code.
// If not, contact the sfizz maintainers at https://github.com/sfztools/sfizz

#include "Resources.h"
#include "SynthConfig.h"
#include "MidiState.h"
#include "FilePool.h"
#include "BufferPool.h"
#include "Wavetables.h"
#include "Curve.h"
#include "Tuning.h"
#include "BeatClock.h"
#include "Metronome.h"
#include "modulations/ModMatrix.h"

namespace sfz {

struct Resources::Impl {
    SynthConfig synthConfig;
    BufferPool bufferPool;
    MidiState midiState;
    CurveSet curves;
    FilePool filePool;
    WavetablePool wavePool;
    Tuning tuning;
    absl::optional<StretchTuning> stretch;
    ModMatrix modMatrix;
    BeatClock beatClock;
    Metronome metronome;
};

Resources::Resources()
    : impl_(new Impl)
{
}

Resources::~Resources()
{
}

void Resources::setSampleRate(float samplerate)
{
    Impl& impl = *impl_;
    impl.midiState.setSampleRate(samplerate);
    impl.modMatrix.setSampleRate(samplerate);
    impl.beatClock.setSampleRate(samplerate);
    impl.metronome.init(samplerate);
}

void Resources::setSamplesPerBlock(int samplesPerBlock)
{
    Impl& impl = *impl_;
    impl.bufferPool.setBufferSize(samplesPerBlock);
    impl.midiState.setSamplesPerBlock(samplesPerBlock);
    impl.modMatrix.setSamplesPerBlock(samplesPerBlock);
    impl.beatClock.setSamplesPerBlock(samplesPerBlock);
}

void Resources::clearNonState()
{
    Impl& impl = *impl_;
    impl.curves = CurveSet::createPredefined();
    impl.filePool.clear();
    impl.wavePool.clearFileWaves();
    impl.modMatrix.clear();
    impl.metronome.clear();
}

void Resources::clearState()
{
    Impl& impl = *impl_;
    impl.midiState.resetNoteStates();
    impl.midiState.resetEventStates();
    impl.beatClock.clear();
}

const SynthConfig& Resources::getSynthConfig() const noexcept
{
    return impl_->synthConfig;
}

const BufferPool& Resources::getBufferPool() const noexcept
{
    return impl_->bufferPool;
}

const MidiState& Resources::getMidiState() const noexcept
{
    return impl_->midiState;
}


const CurveSet& Resources::getCurves() const noexcept
{
    return impl_->curves;
}

const FilePool& Resources::getFilePool() const noexcept
{
    return impl_->filePool;
}

const WavetablePool& Resources::getWavePool() const noexcept
{
    return impl_->wavePool;
}

const Tuning& Resources::getTuning() const noexcept
{
    return impl_->tuning;
}

const absl::optional<StretchTuning>& Resources::getStretch() const noexcept
{
    return impl_->stretch;
}

const ModMatrix& Resources::getModMatrix() const noexcept
{
    return impl_->modMatrix;
}

const BeatClock& Resources::getBeatClock() const noexcept
{
    return impl_->beatClock;
}

const Metronome& Resources::getMetronome() const noexcept
{
    return impl_->metronome;
}

} // namespace sfz
