// SPDX-License-Identifier: BSD-2-Clause

// This code is part of the sfizz library and is licensed under a BSD 2-clause
// license. You should have receive a LICENSE.md file along with the code.
// If not, contact the sfizz maintainers at https://github.com/sfztools/sfizz

#include "Voice.h"
#include "Layer.h"
#include "AudioBuffer.h"
#include "Config.h"
#include "Defaults.h"
#include "EQPool.h"
#include "FilterPool.h"
#include "FlexEnvelope.h"
#include "Interpolators.h"
#include "LFO.h"
#include "MathHelpers.h"
#include "ModifierHelpers.h"
#include "RegionStateful.h"
#include "TriggerEvent.h"
#include "modulations/ModId.h"
#include "modulations/ModKey.h"
#include "modulations/ModMatrix.h"
#include "OnePoleFilter.h"
#include "Panning.h"
#include "PowerFollower.h"
#include "SfzHelpers.h"
#include "SIMDHelpers.h"
#include "Smoothers.h"
#include "FilePool.h"
#include "Wavetables.h"
#include "Tuning.h"
#include "BufferPool.h"
#include "SynthConfig.h"
#include "utility/Macros.h"
#include "utility/Timing.h"
#include <absl/algorithm/container.h>
#include <absl/types/span.h>
#include <random>

namespace sfz {

struct Voice::Impl
{
    Impl() = delete;
    Impl(int voiceNumber, Resources& resources);
    /**
     * @brief Fill a span with data from a file source. This is the first step
     * in rendering each block of data.
     *
     * @param buffer
     */
    void fillWithData(AudioSpan<float> buffer) noexcept;
    /**
     * @brief Fill a span with data from a generator source. This is the first step
     * in rendering each block of data.
     *
     * @param buffer
     */
    void fillWithGenerator(AudioSpan<float> buffer) noexcept;

    /**
     * @brief Fill a destination with an interpolated source.
     *
     * @param source the source sample
     * @param dest the destination buffer
     * @param indices the integral parts of the source positions
     * @param coeffs the fractional parts of the source positions
     */
    template <InterpolatorModel M, bool Adding>
    static void fillInterpolated(
        const AudioSpan<const float>& source, const AudioSpan<float>& dest,
        absl::Span<const int> indices, absl::Span<const float> coeffs,
        absl::Span<const float> addingGains);

    /**
     * @brief Fill a destination with an interpolated source, selecting
     *        interpolation type dynamically by quality level.
     *
     * @param source the source sample
     * @param dest the destination buffer
     * @param indices the integral parts of the source positions
     * @param coeffs the fractional parts of the source positions
     * @param quality the quality level 1-10
     */
    template <bool Adding>
    static void fillInterpolatedWithQuality(
        const AudioSpan<const float>& source, const AudioSpan<float>& dest,
        absl::Span<const int> indices, absl::Span<const float> coeffs,
        absl::Span<const float> addingGains, int quality);

    /**
     * @brief Get a S-shaped curve that is applicable to loop crossfading.
     */
    static const Curve& getSCurve();

    /**
     * @brief Compute the amplitude envelope, applied as a gain to a mono
     * or stereo buffer
     *
     * @param modulationSpan
     */
    void amplitudeEnvelope(absl::Span<float> modulationSpan) noexcept;

    /**
     * @brief Apply the crossfade envelope to a span.
     *
     * @param modulationSpan
     */
    void applyCrossfades(absl::Span<float> modulationSpan) noexcept;
    void resetCrossfades() noexcept;

    /**
     * @brief Amplitude stage for a mono source
     *
     * @param buffer
     */
    void ampStageMono(AudioSpan<float> buffer) noexcept;
    /**
     * @brief Amplitude stage for a stereo source
     *
     * @param buffer
     */
    void ampStageStereo(AudioSpan<float> buffer) noexcept;
    /**
     * @brief Amplitude stage for a mono source
     *
     * @param buffer
     */
    void panStageMono(AudioSpan<float> buffer) noexcept;
    void panStageStereo(AudioSpan<float> buffer) noexcept;
    /**
     * @brief Amplitude stage for a mono source
     *
     * @param buffer
     */
    void filterStageMono(AudioSpan<float> buffer) noexcept;
    void filterStageStereo(AudioSpan<float> buffer) noexcept;
    /**
     * @brief Compute the pitch envelope. This envelope is meant to multiply
     * the frequency parameter for each sample (which translates to floating
     * point intervals for sample-based voices, or phases for generators)
     *
     * @param pitchSpan
     */
    void pitchEnvelope(absl::Span<float> pitchSpan) noexcept;

    /**
     * @brief Initialize frequency and gain coefficients for the oscillators.
     */
    void setupOscillatorUnison();
    void updateChannelPowers(AudioSpan<float> buffer);

    /**
     * @brief Modify the voice state and notify any listeners.
     */
    void switchState(State s);

    /**
     * @brief Save the modulation targets to avoid recomputing them in every callback.
     * Must be called during startVoice() ideally.
     */
    void saveModulationTargets(const Region* region) noexcept;

    /**
     * @brief Get the sample quality determined by the active region.
     *
     * @return int
     */
    int getCurrentSampleQuality() const noexcept;
    /**
     * @brief Get the oscillator quality determined by the active region.
     *
     * @return int
     */
    int getCurrentOscillatorQuality() const noexcept;
    /**
     * @brief Reset the loop information
     *
     */
    void resetLoopInformation() noexcept;
    /**
     * @brief Read the loop information data from the region.
     * This requires that the region and promise is properly set.
     *
     */
    void updateLoopInformation() noexcept;

    /**
     * @brief Check whether the voice is released
     *
     * @return true
     * @return false
     */
    bool released() const noexcept;

    /**
     * @brief Release the voice after a given delay
     *
     * @param delay
     */
    void release(int delay) noexcept;

    /**
     * @brief Off the voice (steal). This will respect the off mode of the region
     *      and set the envelopes if necessary.
     *
     * @param delay
     * @param fast whether to apply a fast release regardless of the off mode
     */
    void off(int delay, bool fast = false) noexcept;

    /**
     * @brief Setup the extended CC values for the voice
     *
     */
    void updateExtendedCCValues() noexcept;

    const NumericId<Voice> id_;
    StateListener* stateListener_ = nullptr;

    const Layer* layer_ { nullptr };
    const Region* region_ { nullptr };

    State state_ { State::idle };
    bool noteIsOff_ { false };
    bool offed_ { false };
    enum class SustainState { Up, Sustaining };
    SustainState sustainState_ { SustainState::Up };
    enum class SostenutoState { Up, Sustaining, PreviouslyDown };
    SostenutoState sostenutoState_ { SostenutoState::Up };

    TriggerEvent triggerEvent_;
    absl::optional<int> triggerDelay_;

    float speedRatio_ { 1.0 };
    float pitchRatio_ { 1.0 };
    float baseVolumedB_ { 0.0 };
    float baseGain_ { 1.0 };
    float baseFrequency_ { 440.0 };
    uint8_t pitchKeycenter_ { Default::key };

    float floatPositionOffset_ { 0.0f };
    int sourcePosition_ { 0 };
    int initialDelay_ { 0 };
    int age_ { 0 };
    uint32_t count_ { 1 };
    int sampleEnd_ { 0 };
    int sampleSize_ { 0 };

    struct {
        int start { 0 };
        int end { 0 };
        int size { 0 };
        int xfSize { 0 };
        int xfOutStart { 0 };
        int xfInStart { 0 };
        uint32_t restarts { 0 };
    } loop_;

    FileDataHolder currentPromise_;

    int samplesPerBlock_ { config::defaultSamplesPerBlock };
    float sampleRate_ { config::defaultSampleRate };

    Resources& resources_;

    std::vector<FilterHolder> filters_;
    std::vector<EQHolder> equalizers_;
    std::vector<std::unique_ptr<LFO>> lfos_;
    std::vector<std::unique_ptr<FlexEnvelope>> flexEGs_;

    std::unique_ptr<LFO> lfoAmplitude_;
    std::unique_ptr<LFO> lfoPitch_;
    std::unique_ptr<LFO> lfoFilter_;

    ADSREnvelope egAmplitude_ { resources_.getMidiState() };
    std::unique_ptr<ADSREnvelope> egPitch_;
    std::unique_ptr<ADSREnvelope> egFilter_;

    WavetableOscillator waveOscillators_[config::oscillatorsPerVoice];

    // unison of oscillators
    unsigned waveUnisonSize_ { 0 };
    float waveDetuneRatio_[config::oscillatorsPerVoice] {};
    float waveLeftGain_[config::oscillatorsPerVoice] {};
    float waveRightGain_[config::oscillatorsPerVoice] {};

    double dataDuration_;
    double amplitudeDuration_;
    double panningDuration_;
    double filterDuration_;

    fast_real_distribution<float> uniformNoiseDist_ { -config::uniformNoiseBounds, config::uniformNoiseBounds };
    fast_gaussian_generator<float> gaussianNoiseDist_ { 0.0f, config::noiseVariance };

    Smoother gainSmoother_;
    Smoother bendSmoother_;
    Smoother xfadeSmoother_;
    void resetSmoothers() noexcept;

    ModMatrix::TargetId masterAmplitudeTarget_;
    ModMatrix::TargetId amplitudeTarget_;
    ModMatrix::TargetId volumeTarget_;
    ModMatrix::TargetId panTarget_;
    ModMatrix::TargetId positionTarget_;
    ModMatrix::TargetId widthTarget_;
    ModMatrix::TargetId pitchTarget_;
    ModMatrix::TargetId oscillatorDetuneTarget_;
    ModMatrix::TargetId oscillatorModDepthTarget_;

    bool followPower_ { false };
    PowerFollower powerFollower_;

    ExtendedCCValues extendedCCValues_;
};

Voice::Voice(int voiceNumber, Resources& resources)
: impl_(new Impl(voiceNumber, resources))
{

}

// Need to define the dtor after Impl has been defined
Voice::~Voice()
{

}

Voice::Voice(Voice&& other) noexcept {
    impl_ = std::move(other.impl_);

    if (other.nextSisterVoice_ != &other) {
        nextSisterVoice_ = other.nextSisterVoice_;
        other.nextSisterVoice_ = &other;
        nextSisterVoice_->setPreviousSisterVoice(this);
    } else {
        nextSisterVoice_ = this;
    }

    if (other.previousSisterVoice_ != &other) {
        previousSisterVoice_ = other.previousSisterVoice_;
        other.previousSisterVoice_ = &other;
        previousSisterVoice_->setNextSisterVoice(this);
    } else {
        previousSisterVoice_ = this;
    }
}

Voice& Voice::operator=(Voice&& other) noexcept {
    impl_ = std::move(other.impl_);

    if (other.nextSisterVoice_ != &other) {
        nextSisterVoice_ = other.nextSisterVoice_;
        other.nextSisterVoice_ = &other;
        nextSisterVoice_->setPreviousSisterVoice(this);
    } else {
        nextSisterVoice_ = this;
    }

    if (other.previousSisterVoice_ != &other) {
        previousSisterVoice_ = other.previousSisterVoice_;
        other.previousSisterVoice_ = &other;
        previousSisterVoice_->setNextSisterVoice(this);
    } else {
        previousSisterVoice_ = this;
    }

    return *this;
}

Voice::Impl::Impl(int voiceNumber, Resources& resources)
: id_ { voiceNumber }, stateListener_(nullptr), resources_(resources)
{
    for (unsigned i = 0; i < config::filtersPerVoice; ++i)
        filters_.emplace_back(resources);

    for (unsigned i = 0; i < config::eqsPerVoice; ++i)
        equalizers_.emplace_back(resources);

    for (WavetableOscillator& osc : waveOscillators_)
        osc.init(sampleRate_);

    gainSmoother_.setSmoothing(config::gainSmoothing, sampleRate_);
    xfadeSmoother_.setSmoothing(config::xfadeSmoothing, sampleRate_);

    // prepare curves
    getSCurve();
}

const ExtendedCCValues& Voice::getExtendedCCValues() const noexcept
{
    Impl& impl = *impl_;
    return impl.extendedCCValues_;
}

void Voice::Impl::updateExtendedCCValues() noexcept
{
    MidiState& midiState = resources_.getMidiState();
    extendedCCValues_.unipolar = midiState.getCCValue(ExtendedCCs::unipolarRandom);
    extendedCCValues_.bipolar = midiState.getCCValue(ExtendedCCs::bipolarRandom);
    extendedCCValues_.alternate = midiState.getCCValue(ExtendedCCs::alternate);
    extendedCCValues_.noteGate = midiState.getCCValue(ExtendedCCs::keyboardNoteGate);
    extendedCCValues_.keydelta = midiState.getCCValue(ExtendedCCs::keydelta);
}

bool Voice::startVoice(Layer* layer, int delay, const TriggerEvent& event) noexcept
{
    Impl& impl = *impl_;
    ASSERT(event.value >= 0.0f && event.value <= 1.0f);

    Resources& resources = impl.resources_;
    MidiState& midiState = resources.getMidiState();
    CurveSet& curveSet = resources.getCurves();

    impl.layer_ = layer;
    const Region& region = layer->getRegion();
    impl.region_ = &region;

    impl.triggerEvent_ = event;
    if (impl.triggerEvent_.type == TriggerEventType::CC)
        impl.triggerEvent_.number = region.pitchKeycenter;

    if (region.velocityOverride == VelocityOverride::previous)
        impl.triggerEvent_.value = midiState.getVelocityOverride();

    if (region.disabled()) {
        impl.switchState(State::cleanMeUp);
        return false;
    }

    impl.switchState(State::playing);

    impl.updateExtendedCCValues();

    ASSERT(delay >= 0);
    if (delay < 0)
        delay = 0;

    if (region.isOscillator()) {
        WavetablePool& wavePool = resources.getWavePool();
        const WavetableMulti* wave = nullptr;
        if (!region.isGenerator())
            wave = wavePool.getFileWave(region.sampleId->filename());
        else {
            switch (hash(region.sampleId->filename())) {
            default:
            case hash("*silence"):
                break;
            case hash("*sine"):
                wave = wavePool.getWaveSin();
                break;
            case hash("*triangle"): // fallthrough
            case hash("*tri"):
                wave = wavePool.getWaveTriangle();
                break;
            case hash("*square"):
                wave = wavePool.getWaveSquare();
                break;
            case hash("*saw"):
                wave = wavePool.getWaveSaw();
                break;
            }
        }
        const float phase = region.getPhase();
        const int quality =
            region.oscillatorQuality.value_or(Default::oscillatorQuality);
        for (WavetableOscillator& osc : impl.waveOscillators_) {
            osc.setWavetable(wave);
            osc.setPhase(phase);
            osc.setQuality(quality);
        }
        impl.setupOscillatorUnison();
    } else {
        FilePool& filePool = resources.getFilePool();
        impl.currentPromise_ = filePool.getFilePromise(region.sampleId);
        if (!impl.currentPromise_) {
            impl.switchState(State::cleanMeUp);
            return false;
        }
        impl.updateLoopInformation();
        impl.speedRatio_ = static_cast<float>(impl.currentPromise_->information.sampleRate / impl.sampleRate_);
        impl.sourcePosition_ = sampleOffset(region, midiState);
    }

    // do Scala retuning and reconvert the frequency into a 12TET key number
    Tuning& tuning = resources.getTuning();
    const float numberRetuned = tuning.getKeyFractional12TET(impl.triggerEvent_.number);

    impl.pitchRatio_ = basePitchVariation(region, numberRetuned, impl.triggerEvent_.value, midiState, curveSet);

    // apply stretch tuning if set
    if (absl::optional<StretchTuning>& stretch = resources.getStretch())
        impl.pitchRatio_ *= stretch->getRatioForFractionalKey(numberRetuned);

    impl.pitchKeycenter_ = region.pitchKeycenter;
    impl.baseVolumedB_ = baseVolumedB(region, midiState, impl.triggerEvent_.number);
    impl.baseGain_ = region.getBaseGain();
    if (impl.triggerEvent_.type != TriggerEventType::CC || region.velocityOverride == VelocityOverride::previous)
        impl.baseGain_ *= noteGain(region, impl.triggerEvent_.number, impl.triggerEvent_.value, midiState, curveSet);

    impl.gainSmoother_.reset();
    impl.resetCrossfades();

    for (unsigned i = 0; i < region.filters.size(); ++i) {
        impl.filters_[i].setup(region, i, impl.triggerEvent_.number, impl.triggerEvent_.value);
    }

    for (unsigned i = 0; i < region.equalizers.size(); ++i) {
        impl.equalizers_[i].setup(region, i, impl.triggerEvent_.value);
    }

    impl.triggerDelay_ = delay;
    impl.initialDelay_ = delay + static_cast<int>(regionDelay(region, midiState) * impl.sampleRate_);
    impl.baseFrequency_ = tuning.getFrequencyOfKey(impl.triggerEvent_.number);
    impl.sampleEnd_ = int(sampleEnd(region, midiState));
    impl.sampleSize_ = impl.sampleEnd_- impl.sourcePosition_ - 1;
    impl.bendSmoother_.setSmoothing(region.bendSmooth, impl.sampleRate_);
    impl.bendSmoother_.reset(region.getBendInCents(midiState.getPitchBend()));

    ModMatrix& modMatrix = resources.getModMatrix();
    modMatrix.initVoice(impl.id_, region.getId(), impl.initialDelay_);
    impl.saveModulationTargets(&region);

    if (region.checkSustain) {
        const bool sustainPressed =
            midiState.getCCValue(region.sustainCC) >= region.sustainThreshold;
        impl.sustainState_ =
            sustainPressed ? Impl::SustainState::Sustaining : Impl::SustainState::Up;
    }

    if (region.checkSostenuto) {
        const bool sostenutoPressed =
            midiState.getCCValue(region.sostenutoCC) >= region.sostenutoThreshold;
        impl.sostenutoState_ =
            sostenutoPressed ? Impl::SostenutoState::PreviouslyDown : Impl::SostenutoState::Up;
    }

    return true;
}

int Voice::Impl::getCurrentSampleQuality() const noexcept
{
    return (region_ && region_->sampleQuality) ?
        *region_->sampleQuality : resources_.getSynthConfig().currentSampleQuality();
}

int Voice::getCurrentSampleQuality() const noexcept
{
    Impl& impl = *impl_;
    return impl.getCurrentSampleQuality();
}

int Voice::Impl::getCurrentOscillatorQuality() const noexcept
{
    return (region_ && region_->oscillatorQuality) ?
        *region_->oscillatorQuality : resources_.getSynthConfig().currentOscillatorQuality();
}

int Voice::getCurrentOscillatorQuality() const noexcept
{
    Impl& impl = *impl_;
    return impl.getCurrentOscillatorQuality();
}

bool Voice::isFree() const noexcept
{
    Impl& impl = *impl_;
    return (impl.state_ == State::idle);
}

void Voice::release(int delay) noexcept
{
    Impl& impl = *impl_;
    impl.release(delay);
}

void Voice::Impl::release(int delay) noexcept
{
    if (state_ != State::playing)
        return;

    if (!region_->flexAmpEG) {
        if (egAmplitude_.getRemainingDelay() > delay)
            switchState(State::cleanMeUp);
    }
    else {
        if (flexEGs_[*region_->flexAmpEG]->getRemainingDelay() > static_cast<unsigned>(delay))
            switchState(State::cleanMeUp);
    }

    ModMatrix& modMatrix = resources_.getModMatrix();
    modMatrix.releaseVoice(id_, region_->getId(), delay);
}

void Voice::off(int delay, bool fast) noexcept
{
    Impl& impl = *impl_;
    impl.off(delay, fast);
}

void Voice::Impl::off(int delay, bool fast) noexcept
{
    if (!region_->flexAmpEG) {
        if (region_->offMode == OffMode::fast || fast) {
            egAmplitude_.setReleaseTime(Default::offTime);
        } else if (region_->offMode == OffMode::time) {
            egAmplitude_.setReleaseTime(region_->offTime);
        }
    }
    else {
        // TODO(jpc): Flex AmpEG
    }

    offed_ = true;
    release(delay);
}

void Voice::registerNoteOff(int delay, int noteNumber, float velocity) noexcept
{
    ASSERT(velocity >= 0.0 && velocity <= 1.0);
    UNUSED(velocity);
    Impl& impl = *impl_;

    if (impl.region_ == nullptr)
        return;

    if (impl.state_ != State::playing)
        return;

    if (impl.triggerEvent_.number == noteNumber && impl.triggerEvent_.type == TriggerEventType::NoteOn) {
        impl.noteIsOff_ = true;

        if (impl.region_->loopMode == LoopMode::one_shot)
            return;

        const bool sustainPedalReleaseCondition = !impl.region_->checkSustain
            || impl.sustainState_ != Impl::SustainState::Sustaining;

        const bool sostenutoPedalReleaseCondition = !impl.region_->checkSostenuto
            || impl.sostenutoState_ != Impl::SostenutoState::Sustaining;

        if (sustainPedalReleaseCondition && sostenutoPedalReleaseCondition)
            release(delay);
    }
}

void Voice::registerCC(int delay, int ccNumber, float ccValue) noexcept
{
    Impl& impl = *impl_;
    if (impl.region_ == nullptr)
        return;

    const Region& region = *impl.region_;

    if (impl.state_ != State::playing)
        return;

    if (ccNumber != region.sustainCC && ccNumber != region.sostenutoCC)
        return;

    if (region.checkSustain && (ccNumber == region.sostenutoCC)) {
        if (ccValue < region.sostenutoThreshold) {
            impl.sostenutoState_ = Impl::SostenutoState::Up;
        } else if (impl.sostenutoState_ == Impl::SostenutoState::Up) {
            impl.sostenutoState_ = Impl::SostenutoState::Sustaining;
        }
    }

    if (region.checkSostenuto && (ccNumber == region.sustainCC)) {
        if (ccValue < region.sustainThreshold) {
            impl.sustainState_ = Impl::SustainState::Up;
        } else {
            impl.sustainState_ = Impl::SustainState::Sustaining;
        }
    }

    const bool sustainPedalReleaseCondition = !region.checkSustain
        || (impl.sustainState_ != Impl::SustainState::Sustaining);

    const bool sostenutoPedalReleaseCondition = !region.checkSostenuto
        || (impl.sostenutoState_ != Impl::SostenutoState::Sustaining);

    if (impl.noteIsOff_ && region.loopMode != LoopMode::one_shot
        && sostenutoPedalReleaseCondition && sustainPedalReleaseCondition)
        release(delay);

    if (region.checkSustain && (impl.sustainState_ == Impl::SustainState::Sustaining)
        && impl.resources_.getSynthConfig().sustainCancelsRelease
        && impl.released() && (region.trigger != Trigger::release && region.trigger != Trigger::release_key) ) {
        ModMatrix& modMatrix = impl.resources_.getModMatrix();
        modMatrix.cancelRelease(impl.id_, impl.region_->getId(), delay);
    }
}

void Voice::registerPitchWheel(int delay, float pitch) noexcept
{
    Impl& impl = *impl_;
    if (impl.state_ != State::playing)
        return;
    UNUSED(delay);
    UNUSED(pitch);
}

void Voice::registerAftertouch(int delay, float aftertouch) noexcept
{
    // TODO
    UNUSED(delay);
    UNUSED(aftertouch);
}

void Voice::registerPolyAftertouch(int delay, int noteNumber, float aftertouch) noexcept
{
    Impl& impl = *impl_;
    if (impl.state_ != State::playing)
        return;

    if (!(impl.triggerEvent_.type == TriggerEventType::NoteOn || impl.triggerEvent_.type == TriggerEventType::NoteOff)
        || impl.triggerEvent_.number != noteNumber)
        return;

    // TODO
    UNUSED(delay);
    UNUSED(aftertouch);
}

void Voice::registerTempo(int delay, float secondsPerQuarter) noexcept
{
    // TODO
    UNUSED(delay);
    UNUSED(secondsPerQuarter);
}

void Voice::setSampleRate(float sampleRate) noexcept
{
    Impl& impl = *impl_;
    impl.sampleRate_ = sampleRate;
    impl.gainSmoother_.setSmoothing(config::gainSmoothing, sampleRate);
    impl.xfadeSmoother_.setSmoothing(config::xfadeSmoothing, sampleRate);

    for (WavetableOscillator& osc : impl.waveOscillators_)
        osc.init(sampleRate);

    for (auto& eg : impl.flexEGs_)
        eg->setSampleRate(sampleRate);

    for (auto& lfo : impl.lfos_)
        lfo->setSampleRate(sampleRate);
    if (auto* lfo = impl.lfoAmplitude_.get())
        lfo->setSampleRate(sampleRate);
    if (auto* lfo = impl.lfoPitch_.get())
        lfo->setSampleRate(sampleRate);
    if (auto* lfo = impl.lfoFilter_.get())
        lfo->setSampleRate(sampleRate);

    for (auto& filter : impl.filters_)
        filter.setSampleRate(sampleRate);

    for (auto& eq : impl.equalizers_)
        eq.setSampleRate(sampleRate);

    impl.powerFollower_.setSampleRate(sampleRate);
}

void Voice::setSamplesPerBlock(int samplesPerBlock) noexcept
{
    Impl& impl = *impl_;
    impl.samplesPerBlock_ = samplesPerBlock;
    impl.powerFollower_.setSamplesPerBlock(samplesPerBlock);
}

void Voice::renderBlock(AudioSpan<float, 2> buffer) noexcept
{
    Impl& impl = *impl_;
    ASSERT(static_cast<int>(buffer.getNumFrames()) <= impl.samplesPerBlock_);
    buffer.fill(0.0f);

    const Region* region = impl.region_;
    if (region == nullptr || region->disabled())
        return;

    const auto delay = min(static_cast<size_t>(impl.initialDelay_), buffer.getNumFrames());
    auto delayed_buffer = buffer.subspan(delay);
    impl.initialDelay_ -= static_cast<int>(delay);

    { // Fill buffer with raw data
        ScopedTiming logger { impl.dataDuration_ };
        if (region->isOscillator())
            impl.fillWithGenerator(delayed_buffer);
        else
            impl.fillWithData(delayed_buffer);
    }

    if (region->isStereo()) {
        impl.ampStageStereo(buffer);
        impl.panStageStereo(buffer);
        impl.filterStageStereo(buffer);
    } else {
        impl.ampStageMono(buffer);
        impl.filterStageMono(buffer);
        impl.panStageMono(buffer);
    }

    if (!region->flexAmpEG) {
        if (!impl.egAmplitude_.isSmoothing())
            impl.switchState(State::cleanMeUp);
    }
    else {
        if (impl.flexEGs_[*region->flexAmpEG]->isFinished())
            impl.switchState(State::cleanMeUp);
    }

    impl.powerFollower_.process(buffer);

    impl.age_ += buffer.getNumFrames();
    if (impl.triggerDelay_) {
        // Should be OK but just in case;
        impl.age_ = min(impl.age_ - *impl.triggerDelay_, 0);
        impl.triggerDelay_ = absl::nullopt;
    }

#if 0
    ASSERT(!hasNanInf(buffer.getConstSpan(0)));
    ASSERT(!hasNanInf(buffer.getConstSpan(1)));
    SFIZZ_CHECK(isReasonableAudio(buffer.getConstSpan(0)));
    SFIZZ_CHECK(isReasonableAudio(buffer.getConstSpan(1)));
#endif
}

void Voice::Impl::resetCrossfades() noexcept
{
    float xfadeValue { 1.0f };
    const auto xfCurve = region_->crossfadeCCCurve;

    MidiState& midiState = resources_.getMidiState();

    for (const auto& mod : region_->crossfadeCCInRange) {
        const auto value = midiState.getCCValue(mod.cc);
        xfadeValue *= crossfadeIn(mod.data, value, xfCurve);
    }

    for (const auto& mod : region_->crossfadeCCOutRange) {
        const auto value = midiState.getCCValue(mod.cc);
        xfadeValue *= crossfadeOut(mod.data, value, xfCurve);
    }

    xfadeSmoother_.reset(xfadeValue);
}

void Voice::Impl::applyCrossfades(absl::Span<float> modulationSpan) noexcept
{
    const auto numSamples = modulationSpan.size();
    const auto xfCurve = region_->crossfadeCCCurve;

    MidiState& midiState = resources_.getMidiState();
    BufferPool& bufferPool = resources_.getBufferPool();

    auto tempSpan = bufferPool.getBuffer(numSamples);
    auto xfadeSpan = bufferPool.getBuffer(numSamples);

    if (!tempSpan || !xfadeSpan)
        return;

    fill<float>(*xfadeSpan, 1.0f);

    bool canShortcut = true;
    for (const auto& mod : region_->crossfadeCCInRange) {
        const auto& events = midiState.getCCEvents(mod.cc);
        canShortcut &= (events.size() == 1);
        linearEnvelope(events, *tempSpan, [&](float x) {
            return crossfadeIn(mod.data, x, xfCurve);
        });
        applyGain<float>(*tempSpan, *xfadeSpan);
    }

    for (const auto& mod : region_->crossfadeCCOutRange) {
        const auto& events = midiState.getCCEvents(mod.cc);
        canShortcut &= (events.size() == 1);
        linearEnvelope(events, *tempSpan, [&](float x) {
            return crossfadeOut(mod.data, x, xfCurve);
        });
        applyGain<float>(*tempSpan, *xfadeSpan);
    }

    xfadeSmoother_.process(*xfadeSpan, *xfadeSpan, canShortcut);
    applyGain<float>(*xfadeSpan, modulationSpan);
}


void Voice::Impl::amplitudeEnvelope(absl::Span<float> modulationSpan) noexcept
{
    const auto numSamples = modulationSpan.size();

    ModMatrix& mm = resources_.getModMatrix();

    // Amplitude EG
    absl::Span<const float> ampegOut(mm.getModulation(masterAmplitudeTarget_), numSamples);
    ASSERT(ampegOut.data());
    copy(ampegOut, modulationSpan);

    // Amplitude envelope
    applyGain1<float>(baseGain_, modulationSpan);
    if (float* mod = mm.getModulation(amplitudeTarget_)) {
        for (size_t i = 0; i < numSamples; ++i)
            modulationSpan[i] *= mod[i];
    }

    // Volume envelope
    applyGain1<float>(db2mag(baseVolumedB_), modulationSpan);
    if (float* mod = mm.getModulation(volumeTarget_)) {
        for (size_t i = 0; i < numSamples; ++i)
            modulationSpan[i] *= db2mag(mod[i]);
    }

    // Smooth the gain transitions
    gainSmoother_.process(modulationSpan, modulationSpan);
}

void Voice::Impl::ampStageMono(AudioSpan<float> buffer) noexcept
{
    ScopedTiming logger { amplitudeDuration_ };

    const auto numSamples = buffer.getNumFrames();
    const auto leftBuffer = buffer.getSpan(0);

    BufferPool& bufferPool = resources_.getBufferPool();

    auto modulationSpan = bufferPool.getBuffer(numSamples);
    if (!modulationSpan)
        return;

    amplitudeEnvelope(*modulationSpan);
    applyCrossfades(*modulationSpan);
    applyGain<float>(*modulationSpan, leftBuffer);
}

void Voice::Impl::ampStageStereo(AudioSpan<float> buffer) noexcept
{
    ScopedTiming logger { amplitudeDuration_ };

    BufferPool& bufferPool = resources_.getBufferPool();

    const auto numSamples = buffer.getNumFrames();
    auto modulationSpan = bufferPool.getBuffer(numSamples);
    if (!modulationSpan)
        return;

    amplitudeEnvelope(*modulationSpan);
    applyCrossfades(*modulationSpan);
    buffer.applyGain(*modulationSpan);
}

void Voice::Impl::panStageMono(AudioSpan<float> buffer) noexcept
{
    ScopedTiming logger { panningDuration_ };

    const auto numSamples = buffer.getNumFrames();
    const auto leftBuffer = buffer.getSpan(0);
    const auto rightBuffer = buffer.getSpan(1);

    BufferPool& bufferPool = resources_.getBufferPool();

    auto modulationSpan = bufferPool.getBuffer(numSamples);
    if (!modulationSpan)
        return;

    ModMatrix& mm = resources_.getModMatrix();

    // Prepare for stereo output
    copy<float>(leftBuffer, rightBuffer);

    // Apply panning
    fill(*modulationSpan, region_->pan);
    if (float* mod = mm.getModulation(panTarget_)) {
        for (size_t i = 0; i < numSamples; ++i)
            (*modulationSpan)[i] += mod[i];
    }
    pan(*modulationSpan, leftBuffer, rightBuffer);
}

void Voice::Impl::panStageStereo(AudioSpan<float> buffer) noexcept
{
    ScopedTiming logger { panningDuration_ };
    const auto numSamples = buffer.getNumFrames();
    const auto leftBuffer = buffer.getSpan(0);
    const auto rightBuffer = buffer.getSpan(1);

    BufferPool& bufferPool = resources_.getBufferPool();

    auto modulationSpan = bufferPool.getBuffer(numSamples);
    if (!modulationSpan)
        return;

    ModMatrix& mm = resources_.getModMatrix();

    // Apply panning
    fill(*modulationSpan, region_->pan);
    if (float* mod = mm.getModulation(panTarget_)) {
        for (size_t i = 0; i < numSamples; ++i)
            (*modulationSpan)[i] += mod[i];
    }
    pan(*modulationSpan, leftBuffer, rightBuffer);

    // Apply the width/position process
    fill(*modulationSpan, region_->width);
    if (float* mod = mm.getModulation(widthTarget_)) {
        for (size_t i = 0; i < numSamples; ++i)
            (*modulationSpan)[i] += mod[i];
    }
    width(*modulationSpan, leftBuffer, rightBuffer);

    fill(*modulationSpan, region_->position);
    if (float* mod = mm.getModulation(positionTarget_)) {
        for (size_t i = 0; i < numSamples; ++i)
            (*modulationSpan)[i] += mod[i];
    }
    pan(*modulationSpan, leftBuffer, rightBuffer);

    // add +3dB to compensate for the 2 pan stages (-3dB each stage)
    applyGain1(1.4125375446227544f, leftBuffer);
    applyGain1(1.4125375446227544f, rightBuffer);
}

void Voice::Impl::filterStageMono(AudioSpan<float> buffer) noexcept
{
    ScopedTiming logger { filterDuration_ };
    const auto numSamples = buffer.getNumFrames();
    const auto leftBuffer = buffer.getSpan(0);
    const float* inputChannel[1] { leftBuffer.data() };
    float* outputChannel[1] { leftBuffer.data() };
    for (unsigned i = 0; i < region_->filters.size(); ++i) {
        filters_[i].process(inputChannel, outputChannel, numSamples);
    }

    for (unsigned i = 0; i < region_->equalizers.size(); ++i) {
        equalizers_[i].process(inputChannel, outputChannel, numSamples);
    }
}

void Voice::Impl::filterStageStereo(AudioSpan<float> buffer) noexcept
{
    ScopedTiming logger { filterDuration_ };
    const auto numSamples = buffer.getNumFrames();
    const auto leftBuffer = buffer.getSpan(0);
    const auto rightBuffer = buffer.getSpan(1);

    const float* inputChannels[2] { leftBuffer.data(), rightBuffer.data() };
    float* outputChannels[2] { leftBuffer.data(), rightBuffer.data() };

    for (unsigned i = 0; i < region_->filters.size(); ++i) {
        filters_[i].process(inputChannels, outputChannels, numSamples);
    }

    for (unsigned i = 0; i < region_->equalizers.size(); ++i) {
        equalizers_[i].process(inputChannels, outputChannels, numSamples);
    }
}

void Voice::Impl::fillWithData(AudioSpan<float> buffer) noexcept
{
    const size_t numSamples = buffer.getNumFrames();
    if (numSamples == 0)
        return;

    if (!currentPromise_) {
        DBG("[Voice] Missing promise during fillWithData");
        return;
    }

    auto source = currentPromise_->getData();

    BufferPool& bufferPool = resources_.getBufferPool();
    const CurveSet& curves = resources_.getCurves();

    // calculate interpolation data
    //   indices: integral position in the source audio
    //   coeffs: fractional position normalized 0-1
    auto coeffs = bufferPool.getBuffer(numSamples);
    auto indices = bufferPool.getIndexBuffer(numSamples);
    if (!indices || !coeffs)
        return;
    {
        auto jumps = bufferPool.getBuffer(numSamples);
        if (!jumps)
            return;

        absl::Span<float> pitch = *jumps; // temporary
        pitchEnvelope(pitch);

        float baseRatio = pitchRatio_ * speedRatio_;
        for (size_t i = 0; i < numSamples; ++i)
            (*jumps)[i] = baseRatio * centsFactor(pitch[i]);

        // Take the first sample if the voice just started
        if (age_ == 0)
            jumps->front() = 0.0f;

        jumps->front() += floatPositionOffset_;
        cumsum<float>(*jumps, *jumps);
        sfzInterpolationCast<float>(*jumps, *indices, *coeffs);
        add1<int>(sourcePosition_, *indices);
    }

    // Update loop characteristics with the current CC state
    updateLoopInformation();
    const auto loop = this->loop_;

    // Looping logic
    const bool hasLoopSamples = static_cast<size_t>(loop.end) < source.getNumFrames();
    const bool loopCountReached = region_->loopCount && loop_.restarts >= *region_->loopCount;
    const bool loopContinuous = (region_->loopMode == LoopMode::loop_continuous);
    const bool loopSustain = (region_->loopMode == LoopMode::loop_sustain) && !released();
    const bool shouldLoop = hasLoopSamples && (loopSustain || loopContinuous) && !loopCountReached;

    /*
               loop start             loop end
                   v                      |
                  /|---------------|\     |
                /  |               |  \   |
              /    |               |    \ v
            /------|---------------|------\
            ^                      ^
        xfin start            xfout start
            <------>               <------>
           xfade size             xfade size
     */

    // loop crossfade partitioning
    absl::Span<int> partitionStarts;
    absl::Span<int> partitionTypes;
    unsigned numPartitions = 0;
    enum PartitionType { kPartitionNormal, kPartitionLoopXfade };

    SpanHolder<absl::Span<int>> partitionBuffers[2];
    if (shouldLoop) {
        for (auto& buf : partitionBuffers) {
            buf = bufferPool.getIndexBuffer(numSamples);
            if (!buf)
                return;
        }
        partitionStarts = *partitionBuffers[0];
        partitionTypes = *partitionBuffers[1];
        // Note: partitions will be alternance of Normal/Xfade
        //       computed along with index processing below
    } else {
        static const int starts[1] = { 0 };
        static const int types[1] = { kPartitionNormal };
        partitionStarts = absl::MakeSpan(const_cast<int*>(starts), 1);
        partitionTypes = absl::MakeSpan(const_cast<int*>(types), 1);
        numPartitions = 1;
    }

    const auto sampleEnd = min( int(sampleEnd_), int(currentPromise_->information.end), int(source.getNumFrames())) - 1;

    int blockRestarts { 0 };
    int oldIndex {};
    int oldPartitionType {};

    const auto addPartitionIfNecessary = [&] (unsigned blockIndex, int wrappedIndex, bool wrapped) {
        const bool xfading = wrappedIndex >= loop.start && wrappedIndex >= loop.xfOutStart;
        const int partitionType = xfading ? kPartitionLoopXfade : kPartitionNormal;

        // if looping or entering a different type, start a new partition
        bool start = blockIndex == 0 || wrapped || partitionType != oldPartitionType;
        if (start) {
            partitionStarts[numPartitions] = blockIndex;
            partitionTypes[numPartitions] = partitionType;
            ++numPartitions;
        }

        oldIndex = wrappedIndex;
        oldPartitionType = partitionType;
    };

    if (shouldLoop) {
        unsigned i = 0;
        while (i < numSamples) {
            int wrappedIndex = (*indices)[i] - loop.size * blockRestarts;
            if (wrappedIndex > loop.end) {
                wrappedIndex -= loop.size;
                blockRestarts += 1;
                loop_.restarts += 1;
            }
            (*indices)[i] = wrappedIndex;
            const bool wrapped = wrappedIndex < oldIndex;

            // identify the partition this index is in
            addPartitionIfNecessary(i, wrappedIndex, wrapped);
            i++;

            // Break if we reached the loop count
            if (wrapped && region_->loopCount && loop_.restarts >= *region_->loopCount)
                break;
        }

        while (i < numSamples) { // In case we released within the block, continue as if it were a one-shot
            (*indices)[i] -= loop.size * blockRestarts;
            if ((*indices)[i] >= sampleEnd) {
                fill<int>(indices->subspan(i), sampleEnd);
                fill<float>(coeffs->subspan(i), 0x1.fffffep-1);
                break;
            }
            i++;
        }
    } else { // One shots and loops that have released (for loop sustain) or ended (with loop counts)
        for (unsigned i = 0; i < numSamples; ++i) {
            (*indices)[i] -= sampleSize_ * blockRestarts;

            if ((*indices)[i] >= sampleEnd) {
                if (region_->sampleCount && count_ < *region_->sampleCount && !region_->shouldLoop()) {
                    (*indices)[i] -= sampleSize_;
                    blockRestarts += 1;
                    count_ += 1;
                    continue;
                }

                off(int(i), true);
                fill<int>(indices->subspan(i), sampleEnd);
                fill<float>(coeffs->subspan(i), 0x1.fffffep-1);
                break;
            }
        }
    }

    // interpolation processing
    const int quality = getCurrentSampleQuality();

    for (unsigned ptNo = 0; ptNo < numPartitions; ++ptNo) {
        // current partition
        const int ptType = partitionTypes[ptNo];
        const unsigned ptStart = partitionStarts[ptNo];
        const unsigned ptNextStart = (ptNo + 1 < numPartitions) ? partitionStarts[ptNo + 1] : numSamples;
        const unsigned ptSize = ptNextStart - ptStart;

        // partition spans
        AudioSpan<float> ptBuffer = buffer.subspan(ptStart, ptSize);
        absl::Span<const int> ptIndices = indices->subspan(ptStart, ptSize);
        absl::Span<const float> ptCoeffs = coeffs->subspan(ptStart, ptSize);

        fillInterpolatedWithQuality<false>(
            source, ptBuffer, ptIndices, ptCoeffs, {}, quality);

        if (ptType == kPartitionLoopXfade) {
            auto xfTemp1 = bufferPool.getBuffer(numSamples);
            auto xfTemp2 = bufferPool.getBuffer(numSamples);
            auto xfIndicesTemp = bufferPool.getIndexBuffer(numSamples);
            if (!xfTemp1 || !xfTemp2 || !xfIndicesTemp)
                return;

            absl::Span<float> xfCurvePos = xfTemp1->first(ptSize);

            // compute crossfade positions
            for (unsigned i = 0; i < ptSize; ++i) {
                float pos = float(ptIndices[i]) + ptCoeffs[i];
                xfCurvePos[i] = (pos - float(loop.xfOutStart)) / float(loop.xfSize);
            }

            //----------------------------------------------------------------//
            // Crossfade Out
            //   -> fade out signal nearing the loop end
            {
                // compute out curve
                absl::Span<float> xfCurve = xfTemp2->first(ptSize);
                IF_CONSTEXPR (config::loopXfadeCurve == 2) {
                    const Curve& xfIn = getSCurve();
                    for (unsigned i = 0; i < ptSize; ++i)
                        xfCurve[i] = xfIn.evalNormalized(1.0f - xfCurvePos[i]);
                }
                else IF_CONSTEXPR (config::loopXfadeCurve == 1) {
                    const Curve& xfOut = curves.getCurve(6);
                    for (unsigned i = 0; i < ptSize; ++i)
                        xfCurve[i] = xfOut.evalNormalized(xfCurvePos[i]);
                }
                else IF_CONSTEXPR (config::loopXfadeCurve == 0) {
                    // TODO(jpc) vectorize this
                    for (unsigned i = 0; i < ptSize; ++i)
                        xfCurve[i] = clamp(1.0f - xfCurvePos[i], 0.0f, 1.0f);
                }
                // apply out curve
                // (scalar fallback: buffer and curve not aligned)
                size_t numChannels = ptBuffer.getNumChannels();
                for (size_t c = 0; c < numChannels; ++c) {
                    absl::Span<float> channel = ptBuffer.getSpan(c);
                    for (unsigned i = 0; i < ptSize; ++i)
                        channel[i] *= xfCurve[i];
                }
            }
            //----------------------------------------------------------------//
            // Crossfade In
            //   -> fade in signal preceding the loop start
            {
                // compute indices of the crossfade input segment
                absl::Span<int> xfInIndices = xfIndicesTemp->first(ptSize);
                absl::c_copy(ptIndices, xfInIndices.begin());
                subtract1(loop.xfOutStart - loop.xfInStart, xfInIndices);

                // disregard the segment whose indices have been pushed
                // into the negatives, take these virtually as zeroes.
                unsigned applyOffset = 0;
                while (applyOffset < ptSize && xfInIndices[applyOffset] < 0)
                    ++applyOffset;
                unsigned applySize = ptSize - applyOffset;

                // offset the indices and coeffs
                xfInIndices = xfInIndices.subspan(applyOffset);
                absl::Span<const float> xfInCoeffs = ptCoeffs.subspan(applyOffset);
                // offset the curve positions
                absl::Span<float> xfInCurvePos = xfCurvePos.subspan(applyOffset);
                // offset the output buffer
                AudioSpan<float> xfInBuffer = ptBuffer.subspan(applyOffset);

                // compute in curve
                absl::Span<float> xfCurve = xfTemp2->first(applySize);
                IF_CONSTEXPR (config::loopXfadeCurve == 2) {
                    const Curve& xfIn = getSCurve();
                    for (unsigned i = 0; i < applySize; ++i)
                        xfCurve[i] = xfIn.evalNormalized(xfInCurvePos[i]);
                }
                else IF_CONSTEXPR (config::loopXfadeCurve == 1) {
                    const Curve& xfIn = curves.getCurve(5);
                    for (unsigned i = 0; i < applySize; ++i)
                        xfCurve[i] = xfIn.evalNormalized(xfInCurvePos[i]);
                }
                else IF_CONSTEXPR (config::loopXfadeCurve == 0) {
                    // TODO(jpc) vectorize this
                    for (unsigned i = 0; i < applySize; ++i)
                        xfCurve[i] = clamp(xfInCurvePos[i], 0.0f, 1.0f);
                }
                // apply in curve
                fillInterpolatedWithQuality<true>(
                    source, xfInBuffer, xfInIndices, xfInCoeffs, xfCurve, quality);
            }
        }
    }

    sourcePosition_ = indices->back();
    floatPositionOffset_ = coeffs->back();

#if 1
    ASSERT(!hasNanInf(buffer.getConstSpan(0)));
    ASSERT(!hasNanInf(buffer.getConstSpan(1)));
    SFIZZ_CHECK(isReasonableAudio(buffer.getConstSpan(0)));
    SFIZZ_CHECK(isReasonableAudio(buffer.getConstSpan(1)));
#endif
}

template <InterpolatorModel M, bool Adding>
void Voice::Impl::fillInterpolated(
    const AudioSpan<const float>& source, const AudioSpan<float>& dest,
    absl::Span<const int> indices, absl::Span<const float> coeffs,
    absl::Span<const float> addingGains)
{
    auto* ind = indices.data();
    auto* coeff = coeffs.data();
    auto* addingGain = addingGains.data();
    auto leftSource = source.getConstSpan(0);
    auto left = dest.getChannel(0);
    if (source.getNumChannels() == 1) {
        while (ind < indices.end()) {
            auto output = interpolate<M>(&leftSource[*ind], *coeff);
            IF_CONSTEXPR(Adding) {
                float g = *addingGain++;
                *left += g * output;
            }
            else
                *left = output;
            incrementAll(ind, left, coeff);
        }
    } else {
        auto right = dest.getChannel(1);
        auto rightSource = source.getConstSpan(1);
        while (ind < indices.end()) {
            auto leftOutput = interpolate<M>(&leftSource[*ind], *coeff);
            auto rightOutput = interpolate<M>(&rightSource[*ind], *coeff);
            IF_CONSTEXPR(Adding) {
                float g = *addingGain++;
                *left += g * leftOutput;
                *right += g * rightOutput;
            }
            else {
                *left = leftOutput;
                *right = rightOutput;
            }
            incrementAll(ind, left, right, coeff);
        }
    }
}

template <bool Adding>
void Voice::Impl::fillInterpolatedWithQuality(
    const AudioSpan<const float>& source, const AudioSpan<float>& dest,
    absl::Span<const int> indices, absl::Span<const float> coeffs,
    absl::Span<const float> addingGains, int quality)
{
    switch (clamp(quality, 0, 10)) {
    case 0:
        {
            constexpr auto itp = kInterpolatorNearest;
            fillInterpolated<itp, Adding>(source, dest, indices, coeffs, addingGains);
        }
        break;
    case 1:
        {
            constexpr auto itp = kInterpolatorLinear;
            fillInterpolated<itp, Adding>(source, dest, indices, coeffs, addingGains);
        }
        break;
    case 2:
        {
#if 0
            // B-spline response has faster decay of aliasing, but not zero-crossings at integer positions
            constexpr auto itp = kInterpolatorBspline3;
#else
            // Hermite polynomial, has less pass-band attenuation
            constexpr auto itp = kInterpolatorHermite3;
#endif
            fillInterpolated<itp, Adding>(source, dest, indices, coeffs, addingGains);
        }
        break;
    case 3:
        {
            constexpr auto itp = kInterpolatorSinc8;
            fillInterpolated<itp, Adding>(source, dest, indices, coeffs, addingGains);
        }
        break;
    case 4:
        {
            constexpr auto itp = kInterpolatorSinc12;
            fillInterpolated<itp, Adding>(source, dest, indices, coeffs, addingGains);
        }
        break;
    case 5:
        {
            constexpr auto itp = kInterpolatorSinc16;
            fillInterpolated<itp, Adding>(source, dest, indices, coeffs, addingGains);
        }
        break;
    case 6:
        {
            constexpr auto itp = kInterpolatorSinc24;
            fillInterpolated<itp, Adding>(source, dest, indices, coeffs, addingGains);
        }
        break;
    case 7:
        {
            constexpr auto itp = kInterpolatorSinc36;
            fillInterpolated<itp, Adding>(source, dest, indices, coeffs, addingGains);
        }
        break;
    case 8:
        {
            constexpr auto itp = kInterpolatorSinc48;
            fillInterpolated<itp, Adding>(source, dest, indices, coeffs, addingGains);
        }
        break;
    case 9:
        {
            constexpr auto itp = kInterpolatorSinc60;
            fillInterpolated<itp, Adding>(source, dest, indices, coeffs, addingGains);
        }
        break;
    case 10:
        {
            constexpr auto itp = kInterpolatorSinc72;
            fillInterpolated<itp, Adding>(source, dest, indices, coeffs, addingGains);
        }
        break;
    }
}

const Curve& Voice::Impl::getSCurve()
{
    static const Curve curve = []() -> Curve {
        constexpr unsigned N = Curve::NumValues;
        float values[N];
        for (unsigned i = 0; i < N; ++i) {
            double x = i / double(N - 1);
            values[i] = float((1.0 - std::cos(M_PI * x)) * 0.5);
        }
        return Curve::buildFromPoints(values);
    }();
    return curve;
}

void Voice::Impl::fillWithGenerator(AudioSpan<float> buffer) noexcept
{
    const auto leftSpan = buffer.getSpan(0);
    const auto rightSpan  = buffer.getSpan(1);

    if (region_->sampleId->filename() == "*noise") {
        auto gen = [&]() {
            return uniformNoiseDist_(Random::randomGenerator);
        };
        absl::c_generate(leftSpan, gen);
        absl::c_generate(rightSpan, gen);
    } else if (region_->sampleId->filename() == "*gnoise") {
        // You need to wrap in a lambda, otherwise generate will
        // make a copy of the gaussian distribution *along with its state*
        // leading to periodic behavior....
        auto gen = [&]() {
            return gaussianNoiseDist_();
        };
        absl::c_generate(leftSpan, gen);
        absl::c_generate(rightSpan, gen);
    } else {
        const size_t numFrames = buffer.getNumFrames();

        BufferPool& bufferPool = resources_.getBufferPool();
        ModMatrix& modMatrix = resources_.getModMatrix();

        auto frequencies = bufferPool.getBuffer(numFrames);
        if (!frequencies)
            return;

        absl::Span<float> pitch = *frequencies; // temporary
        pitchEnvelope(pitch);

        const float keycenterFrequency = midiNoteFrequency(pitchKeycenter_);
        const float baseRatio = pitchRatio_ * keycenterFrequency;

        for (size_t i = 0; i < numFrames; ++i)
            (*frequencies)[i] = baseRatio * centsFactor(pitch[i]);

        auto detuneSpan = bufferPool.getBuffer(numFrames);
        if (!detuneSpan)
            return;

        const int oscillatorMode = region_->oscillatorMode;
        const int oscillatorMulti = region_->oscillatorMulti;
        const int quality = getCurrentOscillatorQuality();

        if (oscillatorMode <= 0 && oscillatorMulti < 2) {
            // single oscillator
            auto tempSpan = bufferPool.getBuffer(numFrames);
            if (!tempSpan)
                return;

            WavetableOscillator& osc = waveOscillators_[0];
            osc.setQuality(quality);
            fill(*detuneSpan, 1.0f);
            osc.processModulated(frequencies->data(), detuneSpan->data(), tempSpan->data(), buffer.getNumFrames());
            copy<float>(*tempSpan, leftSpan);
            copy<float>(*tempSpan, rightSpan);
        }
        else if (oscillatorMode <= 0 && oscillatorMulti >= 3) {
            // unison oscillator
            auto tempSpan = bufferPool.getBuffer(numFrames);
            auto tempLeftSpan = bufferPool.getBuffer(numFrames);
            auto tempRightSpan = bufferPool.getBuffer(numFrames);
            if (!tempSpan || !tempLeftSpan || !tempRightSpan)
                return;

            const float* detuneMod = modMatrix.getModulation(oscillatorDetuneTarget_);
            for (unsigned u = 0, uSize = waveUnisonSize_; u < uSize; ++u) {
                WavetableOscillator& osc = waveOscillators_[u];
                osc.setQuality(quality);
                if (!detuneMod)
                    fill(*detuneSpan, waveDetuneRatio_[u]);
                else {
                    for (size_t i = 0; i < numFrames; ++i)
                        (*detuneSpan)[i] = centsFactor(detuneMod[i]);
                    applyGain1(waveDetuneRatio_[u], *detuneSpan);
                }
                osc.processModulated(frequencies->data(), detuneSpan->data(), tempSpan->data(), numFrames);
                if (u == 0) {
                    applyGain1<float>(waveLeftGain_[u], *tempSpan, *tempLeftSpan);
                    applyGain1<float>(waveRightGain_[u], *tempSpan, *tempRightSpan);
                }
                else {
                    multiplyAdd1<float>(waveLeftGain_[u], *tempSpan, *tempLeftSpan);
                    multiplyAdd1<float>(waveRightGain_[u], *tempSpan, *tempRightSpan);
                }
            }

            copy<float>(*tempLeftSpan, leftSpan);
            copy<float>(*tempRightSpan, rightSpan);
        }
        else {
            // modulated oscillator
            auto tempSpan = bufferPool.getBuffer(numFrames);
            if (!tempSpan)
                return;

            WavetableOscillator& oscCar = waveOscillators_[0];
            WavetableOscillator& oscMod = waveOscillators_[1];
            oscCar.setQuality(quality);
            oscMod.setQuality(quality);

            // compute the modulator
            auto modulatorSpan = bufferPool.getBuffer(numFrames);
            if (!modulatorSpan)
                return;

            const float* detuneMod = modMatrix.getModulation(oscillatorDetuneTarget_);
            if (!detuneMod)
                fill(*detuneSpan, waveDetuneRatio_[1]);
            else {
                for (size_t i = 0; i < numFrames; ++i)
                    (*detuneSpan)[i] = centsFactor(detuneMod[i]);
                applyGain1(waveDetuneRatio_[1], *detuneSpan);
            }

            oscMod.processModulated(frequencies->data(), detuneSpan->data(), modulatorSpan->data(), numFrames);

            // scale the modulator
            const float oscillatorModDepth = region_->oscillatorModDepth;
            if (oscillatorModDepth != 1.0f)
                applyGain1(oscillatorModDepth, *modulatorSpan);
            const float* modDepthMod = modMatrix.getModulation(oscillatorModDepthTarget_);
            if (modDepthMod)
                applyGain(absl::MakeConstSpan(modDepthMod, numFrames), *modulatorSpan);

            // compute carrier×modulator
            switch (region_->oscillatorMode) {
            case 0: // RM synthesis
            default:
                fill(*detuneSpan, 1.0f);
                oscCar.processModulated(frequencies->data(), detuneSpan->data(), tempSpan->data(), buffer.getNumFrames());
                applyGain<float>(*modulatorSpan, *tempSpan);
                break;

            case 1: // PM synthesis
                // Note(jpc): not implemented, just do FM instead
                goto fm_synthesis;
                break;

            case 2: // FM synthesis
            fm_synthesis:
                fill(*detuneSpan, 1.0f);
                multiplyAdd<float>(*modulatorSpan, *frequencies, *frequencies);
                oscCar.processModulated(frequencies->data(), detuneSpan->data(), tempSpan->data(), buffer.getNumFrames());
                break;
            }

            copy<float>(*tempSpan, leftSpan);
            copy<float>(*tempSpan, rightSpan);
        }
    }

#if 0
    ASSERT(!hasNanInf(buffer.getConstSpan(0)));
    ASSERT(!hasNanInf(buffer.getConstSpan(1)));
    SFIZZ_CHECK(isReasonableAudio(buffer.getConstSpan(0)));
    SFIZZ_CHECK(isReasonableAudio(buffer.getConstSpan(1)));
#endif
}

bool Voice::released() const noexcept
{
    Impl& impl = *impl_;
    return impl.released();
}

bool Voice::Impl::released() const noexcept
{
    if (!region_ || state_ != State::playing)
        return true;


    if (!region_->flexAmpEG)
        return egAmplitude_.isReleased();
    else
        return flexEGs_[*region_->flexAmpEG]->isReleased();
}

bool Voice::checkOffGroup(const Region* other, int delay, int noteNumber) noexcept
{
    Impl& impl = *impl_;
    const Layer* layer = impl.layer_;
    const Region* region = impl.region_;
    if (region == nullptr || other == nullptr)
        return false;

    if (impl.offed_)
        return false;

    if ((impl.triggerEvent_.type == TriggerEventType::NoteOn
            ||  impl.triggerEvent_.type == TriggerEventType::CC)
        && region->offBy && *region->offBy == other->group
        && (region->group != other->group || !layer->ccSwitched_.all() || noteNumber != impl.triggerEvent_.number)) {
        off(delay);
        return true;
    }

    return false;
}

void Voice::reset() noexcept
{
    Impl& impl = *impl_;
    impl.switchState(State::idle);
    impl.layer_ = nullptr;
    impl.region_ = nullptr;
    impl.currentPromise_.reset();
    impl.sourcePosition_ = 0;
    impl.age_ = 0;
    impl.count_ = 1;
    impl.floatPositionOffset_ = 0.0f;
    impl.noteIsOff_ = false;
    impl.sostenutoState_ = Impl::SostenutoState::Up;
    impl.offed_ = false;

    impl.resetLoopInformation();

    impl.powerFollower_.clear();

    for (auto& filter : impl.filters_)
        filter.reset();

    for (auto& eq : impl.equalizers_)
        eq.reset();

    removeVoiceFromRing();
}

void Voice::Impl::resetLoopInformation() noexcept
{
    loop_.start = 0;
    loop_.end = 0;
    loop_.size = 0;
    loop_.xfSize = 0;
    loop_.xfOutStart = 0;
    loop_.xfInStart = 0;
    loop_.restarts = 0;
}

void Voice::Impl::updateLoopInformation() noexcept
{
    if (!region_ || !currentPromise_)
        return;

    if (!region_->shouldLoop())
        return;

    const Region& region = *region_;
    MidiState& midiState = resources_.getMidiState();
    const FileInformation& info = currentPromise_->information;
    const double rate = info.sampleRate;

    loop_.start = static_cast<int>(loopStart(region, midiState));
    loop_.end = max(static_cast<int>(loopEnd(region, midiState)), loop_.start);
    loop_.size = loop_.end + 1 - loop_.start;
    loop_.xfSize = static_cast<int>(lroundPositive(region.loopCrossfade * rate));
    // Clamp the crossfade to the part available before the loop starts
    loop_.xfSize = min(loop_.start, loop_.xfSize);
    loop_.xfOutStart = loop_.end + 1 - loop_.xfSize;
    loop_.xfInStart = loop_.start - loop_.xfSize;
}

void Voice::setNextSisterVoice(Voice* voice) noexcept
{
    // Should never be null
    ASSERT(voice);
    nextSisterVoice_ = voice;
}

void Voice::setPreviousSisterVoice(Voice* voice) noexcept
{
    // Should never be null
    ASSERT(voice);
    previousSisterVoice_ = voice;
}

void Voice::removeVoiceFromRing() noexcept
{
    previousSisterVoice_->setNextSisterVoice(nextSisterVoice_);
    nextSisterVoice_->setPreviousSisterVoice(previousSisterVoice_);
    previousSisterVoice_ = this;
    nextSisterVoice_ = this;
}

float Voice::getAveragePower() const noexcept
{
    Impl& impl = *impl_;
    if (impl.followPower_)
        return impl.powerFollower_.getAveragePower();
    else
        return 0.0f;
}

bool Voice::offedOrFree() const noexcept
{
    Impl& impl = *impl_;
    if (impl.state_ != State::playing)
        return true;

    return impl.offed_;
}

void Voice::setMaxFiltersPerVoice(size_t numFilters)
{
    Impl& impl = *impl_;
    if (numFilters == impl.filters_.size())
        return;

    impl.filters_.clear();
    for (unsigned i = 0; i < numFilters; ++i)
        impl.filters_.emplace_back(impl.resources_);
}

void Voice::setMaxEQsPerVoice(size_t numFilters)
{
    Impl& impl = *impl_;
    if (numFilters == impl.equalizers_.size())
        return;

    impl.equalizers_.clear();
    for (unsigned i = 0; i < numFilters; ++i)
        impl.equalizers_.emplace_back(impl.resources_);
}

void Voice::setMaxLFOsPerVoice(size_t numLFOs)
{
    Impl& impl = *impl_;
    Resources& resources = impl.resources_;

    impl.lfos_.resize(numLFOs);

    for (size_t i = 0; i < numLFOs; ++i) {
        auto lfo = absl::make_unique<LFO>(resources);
        lfo->setSampleRate(impl.sampleRate_);
        impl.lfos_[i] = std::move(lfo);
    }
}

void Voice::setMaxFlexEGsPerVoice(size_t numFlexEGs)
{
    Impl& impl = *impl_;
    Resources& resources = impl.resources_;

    impl.flexEGs_.resize(numFlexEGs);

    for (size_t i = 0; i < numFlexEGs; ++i) {
        auto eg = absl::make_unique<FlexEnvelope>(resources);
        eg->setSampleRate(impl.sampleRate_);
        impl.flexEGs_[i] = std::move(eg);
    }
}

void Voice::setPitchEGEnabledPerVoice(bool havePitchEG)
{
    Impl& impl = *impl_;
    if (havePitchEG)
        impl.egPitch_.reset(new ADSREnvelope(impl.resources_.getMidiState()));
    else
        impl.egPitch_.reset();
}

void Voice::setFilterEGEnabledPerVoice(bool haveFilterEG)
{
    Impl& impl = *impl_;
    if (haveFilterEG)
        impl.egFilter_.reset(new ADSREnvelope(impl.resources_.getMidiState()));
    else
        impl.egFilter_.reset();
}

void Voice::setAmplitudeLFOEnabledPerVoice(bool haveAmplitudeLFO)
{
    Impl& impl = *impl_;
    Resources& res = impl.resources_;
    if (haveAmplitudeLFO) {
        LFO* lfo = new LFO(res);
        impl.lfoAmplitude_.reset(lfo);
        lfo->setSampleRate(impl.sampleRate_);
    }
    else
        impl.lfoAmplitude_.reset();
}

void Voice::setPitchLFOEnabledPerVoice(bool havePitchLFO)
{
    Impl& impl = *impl_;
    Resources& res = impl.resources_;
    if (havePitchLFO) {
        LFO* lfo = new LFO(res);
        impl.lfoPitch_.reset(lfo);
        lfo->setSampleRate(impl.sampleRate_);
    }
    else
        impl.lfoPitch_.reset();
}

void Voice::setFilterLFOEnabledPerVoice(bool haveFilterLFO)
{
    Impl& impl = *impl_;
    Resources& res = impl.resources_;
    if (haveFilterLFO) {
        LFO* lfo = new LFO(res);
        impl.lfoFilter_.reset(lfo);
        lfo->setSampleRate(impl.sampleRate_);
    }
    else
        impl.lfoFilter_.reset();
}

void Voice::Impl::setupOscillatorUnison()
{
    const int m = region_->oscillatorMulti;
    const float d = region_->oscillatorDetune;

    // 3-9: unison mode, 1: normal/RM, 2: PM/FM
    if (m < 3 || region_->oscillatorMode > 0) {
        waveUnisonSize_ = 1;
        // carrier
        waveDetuneRatio_[0] = 1.0;
        waveLeftGain_[0] = 1.0;
        waveRightGain_[0] = 1.0;
        // modulator
        const float modDepth = region_->oscillatorModDepth;
        waveDetuneRatio_[1] = centsFactor(d);
        waveLeftGain_[1] = modDepth;
        waveRightGain_[1] = modDepth;
        return;
    }

    // oscillator count, aka. unison size
    waveUnisonSize_ = m;

    // detune (cents)
    float detunes[config::oscillatorsPerVoice];
    detunes[0] = 0.0;
    detunes[1] = -d;
    detunes[2] = +d;
    for (int i = 3; i < m; ++i) {
        int n = (i - 1) / 2;
        detunes[i] = d * ((i & 1) ? -0.25f : +0.25f) * float(n);
    }

    // detune (ratio)
    for (int i = 0; i < m; ++i)
        waveDetuneRatio_[i] = centsFactor(detunes[i]);

    // gains
    waveLeftGain_[0] = 0.0;
    waveRightGain_[m - 1] = 0.0;
    for (int i = 0; i < m - 1; ++i) {
        float g = 1.0f - float(i) / float(m - 1);
        waveLeftGain_[m - 1 - i] = g;
        waveRightGain_[i] = g;
    }

#if 0
    fprintf(stderr, "\n");
    fprintf(stderr, "# Left:\n");
    for (int i = m - 1; i >= 0; --i) {
        if (waveLeftGain[i] != 0)
            fprintf(stderr, "[%d] %10g cents, %10g dB\n", i, detunes[i], 20.0f * std::log10(waveLeftGain[i]));
    }
    fprintf(stderr, "\n");
    fprintf(stderr, "# Right:\n");
    for (int i = 0; i < m; ++i) {
        if (waveRightGain[i] != 0)
            fprintf(stderr, "[%d] %10g cents, %10g dB\n", i, detunes[i], 20.0f * std::log10(waveRightGain[i]));
    }
#endif
}

void Voice::Impl::switchState(State s)
{
    if (s != state_) {
        state_ = s;
        if (stateListener_)
            stateListener_->onVoiceStateChanging(id_, s);
    }
}

void Voice::Impl::pitchEnvelope(absl::Span<float> pitchSpan) noexcept
{
    const size_t numFrames = pitchSpan.size();

    const MidiState& midiState = resources_.getMidiState();
    const EventVector& events = midiState.getPitchEvents();
    const auto bendLambda = [this](float bend) {
        return region_->getBendInCents(bend);
    };

    if (region_->bendStep > 1.0f)
        linearEnvelope(events, pitchSpan, bendLambda, region_->bendStep);
    else
        linearEnvelope(events, pitchSpan, bendLambda);
    bendSmoother_.process(pitchSpan, pitchSpan);

    ModMatrix& mm = resources_.getModMatrix();

    if (float* mod = mm.getModulation(pitchTarget_))
        add<float>(absl::MakeSpan(mod, numFrames), pitchSpan);
}

void Voice::Impl::resetSmoothers() noexcept
{
    bendSmoother_.reset(0.0f);
    gainSmoother_.reset(0.0f);
}

void Voice::Impl::saveModulationTargets(const Region* region) noexcept
{
    ModMatrix& mm = resources_.getModMatrix();
    masterAmplitudeTarget_ = mm.findTarget(ModKey::createNXYZ(ModId::MasterAmplitude, region->getId()));
    amplitudeTarget_ = mm.findTarget(ModKey::createNXYZ(ModId::Amplitude, region->getId()));
    volumeTarget_ = mm.findTarget(ModKey::createNXYZ(ModId::Volume, region->getId()));
    panTarget_ = mm.findTarget(ModKey::createNXYZ(ModId::Pan, region->getId()));
    positionTarget_ = mm.findTarget(ModKey::createNXYZ(ModId::Position, region->getId()));
    widthTarget_ = mm.findTarget(ModKey::createNXYZ(ModId::Width, region->getId()));
    pitchTarget_ = mm.findTarget(ModKey::createNXYZ(ModId::Pitch, region->getId()));
    oscillatorDetuneTarget_ = mm.findTarget(ModKey::createNXYZ(ModId::OscillatorDetune, region->getId()));
    oscillatorModDepthTarget_ = mm.findTarget(ModKey::createNXYZ(ModId::OscillatorModDepth, region->getId()));
}

void Voice::enablePowerFollower() noexcept
{
    Impl& impl = *impl_;
    impl.followPower_ = true;
    impl.powerFollower_.clear();
}

void Voice::disablePowerFollower() noexcept
{
    Impl& impl = *impl_;
    impl.followPower_ = false;
}

float Voice::getSampleRate() const noexcept
{
    Impl& impl = *impl_;
    return impl.sampleRate_;
}

int Voice::getSamplesPerBlock() const noexcept
{
    Impl& impl = *impl_;
    return impl.samplesPerBlock_;
}

bool Voice::toBeCleanedUp() const
{
    Impl& impl = *impl_;
    return impl.state_ == State::cleanMeUp;
}

void Voice::setStateListener(StateListener *l) noexcept
{
    Impl& impl = *impl_;
    impl.stateListener_ = l;
}

NumericId<Voice> Voice::getId() const noexcept
{
    Impl& impl = *impl_;
    return impl.id_;
}

const TriggerEvent& Voice::getTriggerEvent() const noexcept
{
    Impl& impl = *impl_;
    return impl.triggerEvent_;
}

const Region* Voice::getRegion() const noexcept
{
    Impl& impl = *impl_;
    return impl.region_;
}

int Voice::getRemainingDelay() const noexcept
{
    Impl& impl = *impl_;
    return impl.initialDelay_;
}

int Voice::getSourcePosition() const noexcept
{
    Impl& impl = *impl_;
    return impl.sourcePosition_;
}

LFO* Voice::getLFO(size_t index)
{
    Impl& impl = *impl_;
    return impl.lfos_[index].get();
}

FlexEnvelope* Voice::getFlexEG(size_t index)
{
    Impl& impl = *impl_;
    return impl.flexEGs_[index].get();
}

int Voice::getAge() const noexcept
{
    Impl& impl = *impl_;
    return impl.age_;
}

double Voice::getLastDataDuration() const noexcept
{
    Impl& impl = *impl_;
    return impl.dataDuration_;
}

double Voice::getLastAmplitudeDuration() const noexcept
{
    Impl& impl = *impl_;
    return impl.amplitudeDuration_;
}

double Voice::getLastFilterDuration() const noexcept
{
    Impl& impl = *impl_;
    return impl.filterDuration_;
}

double Voice::getLastPanningDuration() const noexcept
{
    Impl& impl = *impl_;
    return impl.panningDuration_;
}

LFO* Voice::getAmplitudeLFO()
{
    Impl& impl = *impl_;
    return impl.lfoAmplitude_.get();
}

LFO* Voice::getPitchLFO()
{
    Impl& impl = *impl_;
    return impl.lfoPitch_.get();
}

LFO* Voice::getFilterLFO()
{
    Impl& impl = *impl_;
    return impl.lfoFilter_.get();
}

ADSREnvelope* Voice::getAmplitudeEG()
{
    Impl& impl = *impl_;
    return &impl.egAmplitude_;
}

ADSREnvelope* Voice::getPitchEG()
{
    Impl& impl = *impl_;
    return impl.egPitch_.get();
}

ADSREnvelope* Voice::getFilterEG()
{
    Impl& impl = *impl_;
    return impl.egFilter_.get();
}

const TriggerEvent& Voice::getTriggerEvent()
{
    Impl& impl = *impl_;
    return impl.triggerEvent_;
}

} // namespace sfz
