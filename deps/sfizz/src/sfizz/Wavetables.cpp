// SPDX-License-Identifier: BSD-2-Clause

// This code is part of the sfizz library and is licensed under a BSD 2-clause
// license. You should have receive a LICENSE.md file along with the code.
// If not, contact the sfizz maintainers at https://github.com/sfztools/sfizz

#include "Wavetables.h"
#include "FilePool.h"
#include "Interpolators.h"
#include "MathHelpers.h"
#include "absl/meta/type_traits.h"
#include <kiss_fftr.h>

namespace sfz {

void WavetableOscillator::init(double sampleRate)
{
    _sampleInterval = 1.0 / sampleRate;
    _multi = WavetableMulti::getSilenceWavetable();
    clear();
}

void WavetableOscillator::clear()
{
    _phase = 0.0f;
}

void WavetableOscillator::setWavetable(const WavetableMulti* wave)
{
    _multi = wave ? wave : WavetableMulti::getSilenceWavetable();
}

void WavetableOscillator::setPhase(float phase)
{
    ASSERT(phase >= 0.0f && phase <= 1.0f);
    _phase = phase;
}

static float incrementAndWrap(float phase, float inc)
{
    phase += inc;
    phase -= static_cast<int>(phase);
    phase += phase < 0.0f; // in case of negative frequencies
    return phase;
}

template <InterpolatorModel M>
void WavetableOscillator::processSingle(float frequency, float detuneRatio, float* output, unsigned nframes)
{
    float phase = _phase;
    float phaseInc = frequency * (detuneRatio * _sampleInterval);

    const WavetableMulti& multi = *_multi;
    unsigned tableSize = multi.tableSize();
    absl::Span<const float> table = multi.getTableForFrequency(frequency);

    for (unsigned i = 0; i < nframes; ++i) {
        float position = phase * tableSize;
        unsigned index = static_cast<unsigned>(position);
        float frac = position - index;
        output[i] = interpolate<M>(&table[index], frac);

        phase = incrementAndWrap(phase, phaseInc);
    }

    _phase = phase;
}

template <InterpolatorModel M>
void WavetableOscillator::processModulatedSingle(const float* frequencies, const float* detuneRatios, float* output, unsigned nframes)
{
    float phase = _phase;
    float sampleInterval = _sampleInterval;

    const WavetableMulti& multi = *_multi;
    unsigned tableSize = multi.tableSize();

    for (unsigned i = 0; i < nframes; ++i) {
        float frequency = frequencies[i];
        float phaseInc = frequency * (detuneRatios[i] * sampleInterval);
        absl::Span<const float> table = multi.getTableForFrequency(frequency);

        float position = phase * tableSize;
        unsigned index = static_cast<unsigned>(position);
        float frac = position - index;
        output[i] = interpolate<M>(&table[index], frac);

        phase = incrementAndWrap(phase, phaseInc);
    }

    _phase = phase;
}

template <InterpolatorModel M>
void WavetableOscillator::processDual(float frequency, float detuneRatio, float* output, unsigned nframes)
{
    float phase = _phase;
    float phaseInc = frequency * (detuneRatio * _sampleInterval);

    const WavetableMulti& multi = *_multi;
    unsigned tableSize = multi.tableSize();
    WavetableMulti::DualTable dt = multi.getInterpolationPairForFrequency(frequency);

    for (unsigned i = 0; i < nframes; ++i) {
        float position = phase * tableSize;
        unsigned index = static_cast<unsigned>(position);
        float frac = position - index;
        output[i] =
            (1 - dt.delta) * interpolate<M>(&dt.table1[index], frac) +
            dt.delta * interpolate<M>(&dt.table2[index], frac);

        phase = incrementAndWrap(phase, phaseInc);
    }

    _phase = phase;
}

template <InterpolatorModel M>
void WavetableOscillator::processModulatedDual(const float* frequencies, const float* detuneRatios, float* output, unsigned nframes)
{
    float phase = _phase;
    float sampleInterval = _sampleInterval;

    const WavetableMulti& multi = *_multi;
    unsigned tableSize = multi.tableSize();

    for (unsigned i = 0; i < nframes; ++i) {
        float frequency = frequencies[i];
        float phaseInc = frequency * (detuneRatios[i] * sampleInterval);

        WavetableMulti::DualTable dt = multi.getInterpolationPairForFrequency(frequency);

        float position = phase * tableSize;
        unsigned index = static_cast<unsigned>(position);
        float frac = position - index;
        output[i] =
            (1 - dt.delta) * interpolate<M>(&dt.table1[index], frac) +
            dt.delta * interpolate<M>(&dt.table2[index], frac);

        phase = incrementAndWrap(phase, phaseInc);
    }

    _phase = phase;
}

void WavetableOscillator::process(float frequency, float detuneRatio, float* output, unsigned nframes)
{
    int quality = clamp(_quality, 0, 3);

    switch (quality) {
    case 0:
        processSingle<kInterpolatorNearest>(frequency, detuneRatio, output, nframes);
        break;
    case 1:
        processSingle<kInterpolatorLinear>(frequency, detuneRatio, output, nframes);
        break;
    case 2:
        processSingle<kInterpolatorHermite3>(frequency, detuneRatio, output, nframes);
        break;
    case 3:
        processDual<kInterpolatorHermite3>(frequency, detuneRatio, output, nframes);
        break;
    }
}

void WavetableOscillator::processModulated(const float* frequencies, const float* detuneRatios, float* output, unsigned nframes)
{
    int quality = clamp(_quality, 0, 3);

    switch (quality) {
    case 0:
        processModulatedSingle<kInterpolatorNearest>(frequencies, detuneRatios, output, nframes);
        break;
    case 1:
        processModulatedSingle<kInterpolatorLinear>(frequencies, detuneRatios, output, nframes);
        break;
    case 2:
        processModulatedSingle<kInterpolatorHermite3>(frequencies, detuneRatios, output, nframes);
        break;
    case 3:
        processModulatedDual<kInterpolatorHermite3>(frequencies, detuneRatios, output, nframes);
        break;
    }
}

//------------------------------------------------------------------------------
void HarmonicProfile::generate(
    absl::Span<float> table, double amplitude, double cutoff) const
{
    size_t size = table.size();

    typedef std::complex<kiss_fft_scalar> cpx;

    // allocate a spectrum of size N/2+1
    // bins are equispaced in frequency, with index N/2 being nyquist
    std::unique_ptr<cpx[]> spec(new cpx[size / 2 + 1]());

    kiss_fftr_cfg cfg = kiss_fftr_alloc(size, true, nullptr, nullptr);
    if (!cfg)
        throw std::bad_alloc();

    // bins need scaling and phase offset; this IFFT is a sum of cosines
    const std::complex<double> k = std::polar(amplitude * 0.5, M_PI / 2);

    // start filling at bin index 1; 1 is fundamental, 0 is DC
    for (size_t index = 1; index < size / 2 + 1; ++index) {
        if (index * (1.0 / size) > cutoff)
            break;

        std::complex<double> harmonic = getHarmonic(index);
        spec[index] = k * harmonic;
    }

    kiss_fftri(cfg, reinterpret_cast<kiss_fft_cpx*>(spec.get()), table.data());
    kiss_fftr_free(cfg);
}

class SineProfile : public HarmonicProfile {
public:
    std::complex<double> getHarmonic(size_t index) const
    {
        return (index == 1) ? 1.0 : 0.0;
    }
};

class TriangleProfile : public HarmonicProfile {
public:
    std::complex<double> getHarmonic(size_t index) const
    {
        if ((index & 1) == 0)
            return 0.0;

        bool s = (index >> 1) & 1;
        return std::polar<double>(
            (8 / (M_PI * M_PI)) * (1.0 / (index * index)),
            s ? 0.0 : M_PI);
    }
};

class SawProfile : public HarmonicProfile {
public:
    std::complex<double> getHarmonic(size_t index) const
    {
        if (index < 1)
            return 0.0;

        return std::polar(
            (2.0 / M_PI) / index,
            (index & 1) ? 0.0 : M_PI);
    }
};

class SquareProfile : public HarmonicProfile {
public:
    std::complex<double> getHarmonic(size_t index) const
    {
        if ((index & 1) == 0)
            return 0.0;

        return std::polar((4.0 / M_PI) / index, M_PI);
    }
};

static const SineProfile sineProfile;
static const TriangleProfile triangleProfile;
static const SawProfile sawProfile;
static const SquareProfile squareProfile;

const HarmonicProfile& HarmonicProfile::getSine()
{
    return sineProfile;
}

const HarmonicProfile& HarmonicProfile::getTriangle()
{
    return triangleProfile;
}

const HarmonicProfile& HarmonicProfile::getSaw()
{
    return sawProfile;
}

const HarmonicProfile& HarmonicProfile::getSquare()
{
    return squareProfile;
}

//------------------------------------------------------------------------------
constexpr unsigned MipmapRange::N;
constexpr float MipmapRange::F1;
constexpr float MipmapRange::FN;

const float MipmapRange::K = 1.0 / F1;
const float MipmapRange::LogB = std::log(FN / F1) / (N - 1);

const std::array<float, 1024> MipmapRange::FrequencyToIndex = []()
{
    std::array<float, 1024> table;

    for (unsigned i = 0; i < table.size() - 1; ++i) {
        float r = i * (1.0f / (table.size() - 1));
        float f = F1 + r * (FN - F1);
        table[i] = getExactIndexForFrequency(f);
    }
    // ensure the last element to be exact
    table[table.size() - 1] = N - 1;

    return table;
}();

float MipmapRange::getIndexForFrequency(float f)
{
    static constexpr unsigned tableSize = FrequencyToIndex.size();

    float pos = (f - F1) * ((tableSize - 1) / static_cast<float>(FN - F1));
    pos = clamp<float>(pos, 0, tableSize - 1);

    int index1 = static_cast<int>(pos);
    int index2 = std::min<int>(index1 + 1, tableSize - 1);
    float frac = pos - index1;

    return (1.0f - frac) * FrequencyToIndex[index1] +
        frac * FrequencyToIndex[index2];
}

float MipmapRange::getExactIndexForFrequency(float f)
{
    float t = (f < F1) ? 0.0f : (std::log(K * f) / LogB);
    return clamp<float>(t, 0, N - 1);
}

const std::array<float, MipmapRange::N + 1> MipmapRange::IndexToStartFrequency = []()
{
    std::array<float, N + 1> table;
    for (unsigned t = 0; t < N; ++t)
        table[t] = std::exp(t * LogB) / K;
    // end value for final table
    table[N] = 22050.0;

    return table;
}();

MipmapRange MipmapRange::getRangeForIndex(int o)
{
    o = clamp<int>(o, 0, N - 1);

    MipmapRange range;
    range.minFrequency = IndexToStartFrequency[o];
    range.maxFrequency = IndexToStartFrequency[o + 1];

    return range;
}

MipmapRange MipmapRange::getRangeForFrequency(float f)
{
    int index = static_cast<int>(getIndexForFrequency(f));
    return getRangeForIndex(index);
}

//------------------------------------------------------------------------------
constexpr unsigned WavetableMulti::_tableExtra;

WavetableMulti WavetableMulti::createForHarmonicProfile(
    const HarmonicProfile& hp, double amplitude, unsigned tableSize, double refSampleRate)
{
    WavetableMulti wm;
    constexpr unsigned numTables = WavetableMulti::numTables();

    wm.allocateStorage(tableSize);

    for (unsigned m = 0; m < numTables; ++m) {
        MipmapRange range = MipmapRange::getRangeForIndex(m);

        double freq = range.maxFrequency;

        // A spectrum S of fundamental F has: S[1]=F and S[N/2]=Fs'/2
        // which lets it generate frequency up to Fs'/2=F*N/2.
        // Therefore it's desired to cut harmonics at C=0.5*Fs/Fs'=0.5*Fs/(F*N).
        double cutoff = (0.5 * refSampleRate / tableSize) / freq;

        float* ptr = const_cast<float*>(wm.getTablePointer(m));
        absl::Span<float> table(ptr, tableSize);

        hp.generate(table, amplitude, cutoff);
    }

    wm.fillExtra();

    return wm;
}

const WavetableMulti* WavetableMulti::getSilenceWavetable()
{
    static WavetableMulti wm;
    static bool initialized { false };

    if (!initialized) {
        constexpr unsigned numTables = WavetableMulti::numTables();
        wm.allocateStorage(1);
        for (unsigned m = 0; m < numTables; ++m) {
            float* ptr = const_cast<float*>(wm.getTablePointer(m));
            *ptr = 0;
        }
        wm.fillExtra();
        initialized = true;
    }
    return &wm;
}

void WavetableMulti::allocateStorage(unsigned tableSize)
{
    _multiData.resize((tableSize + 2 * _tableExtra) * numTables());
    _tableSize = tableSize;
}

void WavetableMulti::fillExtra()
{
    unsigned tableSize = _tableSize;
    constexpr unsigned tableExtra = _tableExtra;
    constexpr unsigned numTables = WavetableMulti::numTables();

    for (unsigned m = 0; m < numTables; ++m) {
        float* beg = const_cast<float*>(getTablePointer(m));
        float* end = beg + tableSize;
        // fill right
        float* src = beg;
        float* dst = end;
        for (unsigned i = 0; i < tableExtra; ++i) {
            *dst++ = *src;
            src = (src + 1 != end) ? (src + 1) : beg;
        }
        // fill left
        src = end - 1;
        dst = beg - 1;
        for (unsigned i = 0; i < tableExtra; ++i) {
            *dst-- = *src;
            src = (src != beg) ? (src - 1) : (end - 1);
        }
    }
}

//------------------------------------------------------------------------------

/**
 * @brief Harmonic profile which takes its values from a table.
 */
class TabulatedHarmonicProfile : public HarmonicProfile {
public:
    explicit TabulatedHarmonicProfile(absl::Span<const std::complex<float>> harmonics)
        : _harmonics(harmonics)
    {
    }

    std::complex<double> getHarmonic(size_t index) const override
    {
        if (index >= _harmonics.size())
            return {};

        return _harmonics[index];
    }

private:
    absl::Span<const std::complex<float>> _harmonics;
};

//------------------------------------------------------------------------------

WavetablePool::WavetablePool()
{
    getWaveSin();
    getWaveTriangle();
    getWaveSaw();
    getWaveSquare();
}

const WavetableMulti* WavetablePool::getWaveSin()
{
    static auto wave = WavetableMulti::createForHarmonicProfile(
            HarmonicProfile::getSine(), config::amplitudeSine);
    return &wave;
}
const WavetableMulti* WavetablePool::getWaveTriangle()
{
    static auto wave = WavetableMulti::createForHarmonicProfile(
            HarmonicProfile::getTriangle(), config::amplitudeTriangle);
    return &wave;
}
const WavetableMulti* WavetablePool::getWaveSaw()
{
    static auto wave = WavetableMulti::createForHarmonicProfile(
            HarmonicProfile::getSaw(), config::amplitudeSaw);
    return &wave;
}
const WavetableMulti* WavetablePool::getWaveSquare()
{
    static auto wave = WavetableMulti::createForHarmonicProfile(
            HarmonicProfile::getSquare(), config::amplitudeSquare);
    return &wave;
}

const WavetableMulti* WavetablePool::getFileWave(const std::string& filename)
{
    auto it = _fileWaves.find(filename);
    if (it == _fileWaves.end())
        return nullptr;

    return it->second.get();
}

void WavetablePool::clearFileWaves()
{
    _fileWaves.clear();
}

bool WavetablePool::createFileWave(FilePool& filePool, const std::string& filename)
{
    if (_fileWaves.contains(filename))
        return true;

    auto fileHandle = filePool.loadFile(FileId(filename));
    if (!fileHandle)
        return false;

    if (fileHandle->information.numChannels > 1)
        DBG("[sfizz] Only the first channel of " << filename << " will be used to create the wavetable");

    auto audioData = fileHandle->preloadedData.getConstSpan(0);

    // an even size is required for FFT
    static_assert(absl::remove_reference_t<decltype(fileHandle->preloadedData)>::PaddingRight > 0,
                  "Right padding is required on the audio file buffer");
    if (audioData.size() & 1)
        audioData = absl::MakeConstSpan(audioData.data(), audioData.size() + 1);

    size_t fftSize = audioData.size();
    size_t specSize = fftSize / 2 + 1;

    typedef std::complex<kiss_fft_scalar> cpx;
    std::unique_ptr<cpx[]> spec { new cpx[specSize] };

    kiss_fftr_cfg cfg = kiss_fftr_alloc(fftSize, false, nullptr, nullptr);
    if (!cfg)
        throw std::bad_alloc();

    kiss_fftr(cfg, audioData.data(), reinterpret_cast<kiss_fft_cpx*>(spec.get()));
    kiss_fftr_free(cfg);

    // scale transform, and normalize amplitude and phase
    const std::complex<double> k = std::polar(2.0 / fftSize, -M_PI / 2);
    for (size_t i = 0; i < specSize; ++i)
        spec[i] *= k;

    TabulatedHarmonicProfile hp {
        absl::Span<const std::complex<float>> { spec.get(), specSize }
    };

    auto wave = std::make_shared<WavetableMulti>(
        WavetableMulti::createForHarmonicProfile(hp, 1.0));

    _fileWaves[filename] = wave;
    return true;
}

} // namespace sfz
