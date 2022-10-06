// SPDX-License-Identifier: BSD-2-Clause

// This code is part of the sfizz library and is licensed under a BSD 2-clause
// license. You should have receive a LICENSE.md file along with the code.
// If not, contact the sfizz maintainers at https://github.com/sfztools/sfizz

#include "Tuning.h"
#include "Tunings.h" // Surge tuning library
#include "utility/Debug.h"
#include <absl/types/optional.h>
#include <algorithm>
#include <fstream>
#include <sstream>
#include <array>
#include <cmath>

namespace sfz {

struct Tuning::Impl {
public:
    Impl() { updateKeysFractional12TET(); }

    const Tunings::Tuning& tuning() const { return tuning_; }

    float getKeyFractional12TET(int midiKey) const;

    int rootKey() const { return rootKey_; }
    float tuningFrequency() const { return tuningFrequency_; }

    void updateScale(const Tunings::Scale& scale, absl::optional<fs::path> sourceFile = {});
    bool shouldReloadScala();
    void updateRootKey(int rootKey);
    void updateTuningFrequency(float tuningFrequency);
    void reset();
private:
    void updateKeysFractional12TET();
    static Tunings::KeyboardMapping mappingFromParameters(int rootKey, float tuningFrequency);

private:
    static constexpr int defaultRootKey = 60;
    static constexpr float defaultTuningFrequency = 440.0;
    int rootKey_ = defaultRootKey;
    float tuningFrequency_ = defaultTuningFrequency;

    Tunings::Tuning tuning_ {
        Tunings::evenTemperament12NoteScale(),
        mappingFromParameters(defaultRootKey, defaultTuningFrequency)
    };

    absl::optional<fs::path> scalaFile_;
    fs::file_time_type modificationTime_ {};

    static constexpr int numKeys = Tunings::Tuning::N;
    static constexpr int keyOffset = 256; // Surge tuning has key range ±256
    std::array<float, numKeys> keysFractional12TET_;
};

void Tuning::Impl::reset()
{
    rootKey_ = defaultRootKey;
    tuningFrequency_ = defaultTuningFrequency;
    tuning_ = Tunings::Tuning(
        Tunings::evenTemperament12NoteScale(),
        mappingFromParameters(defaultRootKey, defaultTuningFrequency)
    );
    scalaFile_.reset();
    modificationTime_ = fs::file_time_type::min();
    updateKeysFractional12TET();
}

float Tuning::Impl::getKeyFractional12TET(int midiKey) const
{
    return keysFractional12TET_[std::max(0, std::min(numKeys - 1, midiKey + keyOffset))];
}

void Tuning::Impl::updateScale(const Tunings::Scale& scale, absl::optional<fs::path> sourceFile)
{
    tuning_ = Tunings::Tuning(scale, tuning_.keyboardMapping);
    updateKeysFractional12TET();

    scalaFile_ = sourceFile;

    if (sourceFile) {
        std::error_code ec;
        modificationTime_ = fs::last_write_time(*sourceFile, ec);
    }
}

bool Tuning::Impl::shouldReloadScala()
{
    if (!scalaFile_)
        return false;

    std::error_code ec;
    const auto newTime = fs::last_write_time(*scalaFile_, ec);
    if (newTime > modificationTime_) {
        DBG("Scala file changed!");
        modificationTime_ = newTime;
        return true;
    }

    return false;
}

void Tuning::Impl::updateRootKey(int rootKey)
{
    ASSERT(rootKey >= 0);
    rootKey = std::max(0, rootKey);

    if (rootKey_ == rootKey)
        return;

    tuning_ = Tunings::Tuning(tuning_.scale, mappingFromParameters(rootKey, tuningFrequency_));
    rootKey_ = rootKey;
    updateKeysFractional12TET();
}

void Tuning::Impl::updateTuningFrequency(float tuningFrequency)
{
    ASSERT(tuningFrequency >= 0);
    tuningFrequency = std::max(0.0f, tuningFrequency);

    if (tuningFrequency_ == tuningFrequency)
        return;

    tuning_ = Tunings::Tuning(tuning_.scale, mappingFromParameters(rootKey_, tuningFrequency));
    tuningFrequency_ = tuningFrequency;
    updateKeysFractional12TET();
}

void Tuning::Impl::updateKeysFractional12TET()
{
    // mapping of MIDI key to equal temperament key
    for (int key = 0; key < numKeys; ++key) {
        double freq = tuning_.frequencyForMidiNote(key - keyOffset);
        keysFractional12TET_[key] = 12.0 * std::log2(freq / 440.0) + 69.0;
    }
}

Tunings::KeyboardMapping Tuning::Impl::mappingFromParameters(int rootKey, float tuningFrequency)
{
#if 1
    // root note is the start of octave. like Scala
#else
    // root note is the start of next octave. like Sforzando
    rootKey = std::max(0, rootKey - 12);
#endif
    // fixed frequency of the root note
    const double rootFrequency = tuningFrequency * std::exp2((rootKey - 69) / 12.0);
    return Tunings::tuneNoteTo(rootKey, rootFrequency);
}

///
Tuning::Tuning()
    : impl_(new Impl)
{
}

Tuning::~Tuning()
{
}

bool Tuning::loadScalaFile(const fs::path& path)
{
    Tunings::Scale scl;
    fs::ifstream stream(path);

    if (stream.bad()) {
        DBG("Cannot open scale file: " << path);
        goto failure;
    }

    try {
        scl = Tunings::readSCLStream(stream);
    }
    catch (Tunings::TuningError& error) {
        DBG("Tuning: " << error.what());
        goto failure;
    }

    if (scl.count <= 0) {
        DBG("The scale file is empty: " << path);
        goto failure;
    }

    impl_->updateScale(scl, path);
    return true;

failure:
    loadEqualTemperamentScale();
    return false;
}

bool Tuning::loadScalaString(const std::string& text)
{
    Tunings::Scale scl;
    std::istringstream stream(text);

    try {
        scl = Tunings::readSCLStream(stream);
    }
    catch (Tunings::TuningError& error) {
        DBG("Tuning: " << error.what());
        goto failure;
    }

    if (scl.count <= 0) {
        DBG("Error loading scala string: " << text);
        goto failure;
    }

    impl_->updateScale(scl);
    return true;

failure:
    loadEqualTemperamentScale();
    return false;
}

void Tuning::setScalaRootKey(int rootKey)
{
    impl_->updateRootKey(rootKey);
}

int Tuning::getScalaRootKey() const
{
    return impl_->rootKey();
}

void Tuning::setTuningFrequency(float frequency)
{
    impl_->updateTuningFrequency(frequency);
}

float Tuning::getTuningFrequency() const
{
    return impl_->tuningFrequency();
}

void Tuning::loadEqualTemperamentScale()
{
    impl_->updateScale(Tunings::evenTemperament12NoteScale());
}

float Tuning::getFrequencyOfKey(int midiKey) const
{
    return impl_->tuning().frequencyForMidiNote(midiKey);
}

float Tuning::getKeyFractional12TET(int midiKey)
{
    return impl_->getKeyFractional12TET(midiKey);
}

bool Tuning::shouldReloadScala()
{
    return impl_->shouldReloadScala();
}

///
float StretchTuning::getRatioForIntegralKey(int key) const noexcept
{
    return keyDetuneRatio_[std::max(0, std::min(127, key))];
}

float StretchTuning::getRatioForFractionalKey(float key) const noexcept
{
    int index1 = static_cast<int>(key);
    float mu = key - index1;

    index1 = std::max(0, std::min(127, index1));
    int index2 = std::min(127, index1 + 1);

    return keyDetuneRatio_[index1] * (1 - mu) + keyDetuneRatio_[index2] * mu;
}

StretchTuning StretchTuning::createFromDetuneRatios(const float detune[128])
{
    StretchTuning t;
    std::copy(detune, detune + 128, t.keyDetuneRatio_);
    return t;
}

StretchTuning StretchTuning::createRailsbackFromRatio(float stretch)
{
    float data[128];

    static constexpr float railsback21[128] = {
        #include "sfizz/railsback/2-1.h"
    };
    static constexpr float railsback41[128] = {
        #include "sfizz/railsback/4-1.h"
    };
    static constexpr float railsback42[128] = {
        #include "sfizz/railsback/4-2.h"
    };

    // known curves and their matching knob positions
    const float* curves[] = { railsback21, railsback41, railsback42 };
    const float points[] = { 0.25f, 0.5f, 1.0f };
    constexpr int num_curves = sizeof(curves) / sizeof(decltype(curves[0]));

    //
    int index = -1;
    while (index + 1 < num_curves && stretch >= points[index + 1])
        ++index;

    //
    if (index < 0) {
        float mu = std::max(0.0f, stretch / points[0]);
        const float* c = curves[0];
        for (int i = 0; i < 128; ++i)
            data[i] = mu * c[i] + (1 - mu);
    }
    else if (index + 1 < num_curves) {
        float mu = (stretch - points[index]) / (points[index + 1] - points[index]);
        const float* c1 = curves[index];
        const float* c2 = curves[index + 1];
        for (int i = 0; i < 128; ++i)
            data[i] = mu * c2[i] + (1 - mu) * c1[i];
    }
    else {
        const float* c = curves[num_curves - 1];
        for (int i = 0; i < 128; ++i)
            data[i] = c[i];
    }

    //
    return createFromDetuneRatios(data);
}

} // namespace sfz
