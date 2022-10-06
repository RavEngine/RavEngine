// SPDX-License-Identifier: BSD-2-Clause

// This code is part of the sfizz library and is licensed under a BSD 2-clause
// license. You should have receive a LICENSE.md file along with the code.
// If not, contact the sfizz maintainers at https://github.com/sfztools/sfizz

#pragma once
#include "Config.h"
#include "Buffer.h"
#include "MathHelpers.h"
#include "utility/LeakDetector.h"
#include <absl/types/span.h>
#include <absl/container/flat_hash_map.h>
#include <array>
#include <memory>
#include <complex>

namespace sfz {
class FilePool;

class WavetableMulti;

enum InterpolatorModel : int;

/**
   An oscillator based on wavetables
 */
class WavetableOscillator {
public:
    /**
       Initialize with the given sample rate.
       Run it once after instantiating.
     */
    void init(double sampleRate);

    /**
       Reset the oscillation to the initial phase.
     */
    void clear();

    /**
       Set the wavetable to generate with this oscillator.
     */
    void setWavetable(const WavetableMulti* wave);

    /**
       Set the current phase of this oscillator, between 0 and 1 excluded.
     */
    void setPhase(float phase);

    /**
       Set the quality of this oscillator. (cf. `oscillator_quality`)

       0: nearest
       1: linear
       2: high
       3: dual-high
     */
    void setQuality(int q) { _quality = q; }

    /**
       Get the quality of this oscillator. (cf. `oscillator_quality`)
    */
    int quality() const { return _quality; }

    /**
       Compute a cycle of the oscillator, with constant frequency.
     */
    void process(float frequency, float detuneRatio, float* output, unsigned nframes);

    /**
       Compute a cycle of the oscillator, with varying frequency.
     */
    void processModulated(const float* frequencies, const float* detuneRatios, float* output, unsigned nframes);

private:
    // single-table interpolation
    template <InterpolatorModel M>
    void processSingle(float frequency, float detuneRatio, float* output, unsigned nframes);
    template <InterpolatorModel M>
    void processModulatedSingle(const float* frequencies, const float* detuneRatios, float* output, unsigned nframes);

    // dual-table interpolation
    template <InterpolatorModel M>
    void processDual(float frequency, float detuneRatio, float* output, unsigned nframes);
    template <InterpolatorModel M>
    void processModulatedDual(const float* frequencies, const float* detuneRatios, float* output, unsigned nframes);

private:
    float _phase = 0.0f;
    float _sampleInterval = 0.0f;
    const WavetableMulti* _multi = nullptr;
    int _quality = 1;
    LEAK_DETECTOR(WavetableOscillator);
};

/**
   A description of the harmonics of a particular wave form
 */
class HarmonicProfile {
public:
    virtual ~HarmonicProfile() {}

    static const HarmonicProfile& getSine();
    static const HarmonicProfile& getTriangle();
    static const HarmonicProfile& getSaw();
    static const HarmonicProfile& getSquare();

    /**
       @brief Get the value at the given index of the frequency spectrum.

       The modulus and the argument of the complex number are equal to the
       amplitude and the phase of the harmonic component.
     */
    virtual std::complex<double> getHarmonic(size_t index) const = 0;

    /**
       @brief Generate a period of the waveform and store it in the table.

       Do not generate harmonics above cutoff, which is expressed as Fc/Fs.
     */
    void generate(absl::Span<float> table, double amplitude, double cutoff) const;
};

/**
   A helper to select ranges of a mip-mapped wave, according to the
   frequency of an oscillator.

   The ranges are identified by octave numbers; not octaves in a musical sense,
   but as logarithmic divisions of the frequency range.
 */
class MipmapRange {
public:
    float minFrequency = 0;
    float maxFrequency = 0;

    // number of tables in the mipmap
    static constexpr unsigned N = 24;
    // start frequency of the first table in the mipmap
    static constexpr float F1 = 20.0;
    // start frequency of the last table in the mipmap
    static constexpr float FN = 12000.0;

    static float getIndexForFrequency(float f);
    static float getExactIndexForFrequency(float f);
    static MipmapRange getRangeForIndex(int o);
    static MipmapRange getRangeForFrequency(float f);

    // the frequency mapping of the mipmap is defined by formula:
    //     T(f) = log(k*f)/log(b)
    // - T is the table number, converted to index by rounding down
    // - f is the oscillation frequency
    // - k and b are adjustment parameters according to constant parameters
    //     k = 1/F1
    //     b = exp(log(FN/F1)/(N-1))

    static const float K;
    static const float LogB;

    static const std::array<float, 1024> FrequencyToIndex;
    static const std::array<float, N + 1> IndexToStartFrequency;
};

/**
   Multisample of a wavetable, which is a collection of FFT-filtered mipmaps
   adapted for various playback frequencies.
 */
class WavetableMulti {
public:
    // number of elements in each table
    unsigned tableSize() const { return _tableSize; }

    // number of tables in the multisample
    static constexpr unsigned numTables() { return MipmapRange::N; }

    // get the N-th table in the multisample
    absl::Span<const float> getTable(unsigned index) const
    {
        return { getTablePointer(index), _tableSize };
    }

    // get the table which is adequate for a given playback frequency
    absl::Span<const float> getTableForFrequency(float freq) const
    {
        return getTable(MipmapRange::getIndexForFrequency(freq));
    }

    // adjacent tables with interpolation factor between them
    struct DualTable {
        const float* table1;
        const float* table2;
        float delta;
    };

    // get the pair of tables at the fractional multisample position (range checked)
    DualTable getInterpolationPair(float position) const
    {
        DualTable dt;
        int index = static_cast<int>(position);
        dt.delta = position - index;
        dt.table1 = getTablePointer(clamp<int>(index, 0, MipmapRange::N - 1));
        dt.table2 = getTablePointer(clamp<int>(index + 1, 0, MipmapRange::N - 1));
        return dt;
    }

    // get the pair of tables for the given playback frequency (range checked)
    DualTable getInterpolationPairForFrequency(float freq) const
    {
        float position = MipmapRange::getIndexForFrequency(freq);
        return getInterpolationPair(position);
    }

    // create a multisample according to a given harmonic profile
    // the reference sample rate is the minimum value accepted by the DSP
    // system (most defavorable wrt. aliasing)
    static WavetableMulti createForHarmonicProfile(
        const HarmonicProfile& hp, double amplitude,
        unsigned tableSize = config::tableSize,
        double refSampleRate = config::tableRefSampleRate);

    // get a tiny silent wavetable with null content for use with oscillators
    static const WavetableMulti* getSilenceWavetable();

private:
    // get a pointer to the beginning of the N-th table
    const float* getTablePointer(unsigned index) const
    {
        return _multiData.data() + index * (_tableSize + 2 * _tableExtra) + _tableExtra;
    }

    // allocate the internal data for tables of the given size
    void allocateStorage(unsigned tableSize);

    // fill extra data at table ends with repetitions of the first samples
    void fillExtra();

    // length of each individual table of the multisample
    unsigned _tableSize = 0;

    // number X of extra elements, for safe interpolations up to X-th order.
    static constexpr unsigned _tableExtra = 4;

    // internal storage, having `multiSize` rows and `tableSize` columns.
    sfz::Buffer<float> _multiData;
    LEAK_DETECTOR(WavetableMulti);
};

/**
 * @brief Holds predefined and loaded wavetables.
 *
 */
struct WavetablePool {
    WavetablePool();

    /**
     * @brief Get a file wave. Return a silent table if the wave does not exist yet.
     * Use createFileWave to preload file waves before calling this function.
     * This function is real-time safe.
     *
     * @param filename the name of the file wave
     * @return the wavetable, or a silent table
     */
    const WavetableMulti* getFileWave(const std::string& filename);
    /**
     * @brief Load a file wave from the filepool and use it to create a wavetable.
     * This function is not real-time safe.
     *
     * @param filePool the file pool to use to load the file
     * @param filename the file name to load
     * @return true if the wavetable was correctly created (or existed already)
     */
    bool createFileWave(FilePool& filePool, const std::string& filename);
    /**
     * @brief Removes all the stored file waves from the wavetable pool.
     */
    void clearFileWaves();

    static const WavetableMulti* getWaveSin();
    static const WavetableMulti* getWaveTriangle();
    static const WavetableMulti* getWaveSaw();
    static const WavetableMulti* getWaveSquare();

private:
    absl::flat_hash_map<std::string, std::shared_ptr<WavetableMulti>> _fileWaves;
};

} // namespace sfz
