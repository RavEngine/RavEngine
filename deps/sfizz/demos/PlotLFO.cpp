// SPDX-License-Identifier: BSD-2-Clause

// This code is part of the sfizz library and is licensed under a BSD 2-clause
// license. You should have receive a LICENSE.md file along with the code.
// If not, contact the sfizz maintainers at https://github.com/sfztools/sfizz

/**
   This program generates the data file of a LFO output recorded for a fixed
   duration. The file contains columns for each LFO in the SFZ region.
   The columns are: Time, Lfo1, ... LfoN
   One can use Gnuplot to display this data.
   Example:
     sfizz_plot_lfo file.sfz > lfo.dat
     gnuplot
     plot "lfo.dat" using 1:2 with lines
 */

#include "sfizz/Synth.h"
#include "sfizz/LFO.h"
#include "sfizz/Region.h"
#include "sfizz/LFODescription.h"
#include "sfizz/MathHelpers.h"
#include "cxxopts.hpp"
#include <absl/types/span.h>
#include <vector>
#include <iostream>
#include <fstream>
#include <cmath>
#ifdef _WIN32
#define ENABLE_SNDFILE_WINDOWS_PROTOTYPES 1
#endif
#include <sndfile.hh>

//==============================================================================

static double sampleRate = 1000.0; // sample rate used to compute
static double duration = 5.0; // length in seconds
static std::string outputFilename;
static bool saveFlac = false;

static std::vector<sfz::LFODescription> lfoDescriptionFromSfzFile(const fs::path &sfzPath, bool &success)
{
    sfz::Synth synth;

    if (!synth.loadSfzFile(sfzPath)) {
        std::cerr << "Cannot load the SFZ file.\n";
        success = false;
        return {};
    }

    if (synth.getNumRegions() != 1) {
        std::cerr << "The SFZ file must contain exactly one region.\n";
        success = false;
        return {};
    }

    success = true;
    return synth.getRegionView(0)->lfos;
}

/**
   Program which loads LFO configuration and generates plot data for the given duration.
 */
int main(int argc, char* argv[])
{
    cxxopts::Options options("sfizz_plot_lfo", "Compute LFO and generate plot data");

    options.add_options()
        ("s,samplerate", "Sample rate", cxxopts::value(sampleRate))
        ("d,duration", "Duration", cxxopts::value(duration))
        ("o,output", "Output file", cxxopts::value(outputFilename))
        ("F,flac", "Save output as FLAC", cxxopts::value(saveFlac))
        ("h,help", "Print usage")
    ;
    options.positional_help("sfz-file");

    fs::path sfzPath;

    try {
        cxxopts::ParseResult result = options.parse(argc, argv);
        options.parse_positional({ "sfz-file" });

        if (result.count("help")) {
            std::cout << options.help() << std::endl;
            return 0;
        }

        if (argc != 2) {
            std::cerr << "Please indicate the SFZ file to process.\n";
            return 1;
        }

        sfzPath = argv[1];
    }
    catch (cxxopts::OptionException& ex) {
        std::cerr << ex.what() << "\n";
        return 1;
    }

    bool success = false;
    const std::vector<sfz::LFODescription> desc = lfoDescriptionFromSfzFile(sfzPath, success);
    if (!success){
        std::cerr << "Could not extract LFO descriptions from SFZ file.\n";
        return 1;
    }

    if (sampleRate <= 0) {
        std::cerr << "The sample rate provided is invalid.\n";
        return 1;
    }

    constexpr size_t bufferSize = 1024;
    sfz::Resources resources;
    resources.setSamplesPerBlock(bufferSize);

    size_t numLfos = desc.size();
    std::vector<std::unique_ptr<sfz::LFO>> lfos(numLfos);

    for (size_t l = 0; l < numLfos; ++l) {
        sfz::LFO* lfo = new sfz::LFO(resources);
        lfos[l].reset(lfo);
        lfo->setSampleRate(sampleRate);
        lfo->configure(&desc[l]);
    }

    size_t numFrames = (size_t)std::ceil(sampleRate * duration);
    std::vector<float> outputMemory(numLfos * numFrames);

    for (size_t l = 0; l < numLfos; ++l) {
        lfos[l]->start(0);
    }

    std::vector<absl::Span<float>> lfoOutputs(numLfos);
    for (size_t l = 0; l < numLfos; ++l) {
        lfoOutputs[l] = absl::MakeSpan(&outputMemory[l * numFrames], numFrames);
        for (size_t i = 0, currentFrames; i < numFrames; i += currentFrames) {
            currentFrames = std::min(numFrames - i, bufferSize);
            lfos[l]->process(lfoOutputs[l].subspan(i, currentFrames));
        }
    }

    if (saveFlac) {
        if (outputFilename.empty()) {
            std::cerr << "Please indicate the audio file to save.\n";
            return 1;
        }

        fs::path outputPath = fs::u8path(outputFilename);
        SndfileHandle snd(
#ifndef _WIN32
            outputPath.c_str(),
#else
            outputPath.wstring().c_str(),
#endif
            SFM_WRITE, SF_FORMAT_FLAC|SF_FORMAT_PCM_16, numLfos, sampleRate);

        std::unique_ptr<float[]> frame(new float[numLfos]);
        size_t numClips = 0;

        for (size_t i = 0; i < numFrames; ++i) {
            for (size_t l = 0; l < numLfos; ++l) {
                float orig = lfoOutputs[l][i];
                float clamped = clamp(orig, -1.0f, 1.0f);
                numClips += clamped != orig;
                frame[l] = clamped;
            }
            snd.writef(frame.get(), 1);
        }
        snd.writeSync();

        if (snd.error()) {
            std::error_code ec;
            fs::remove(outputPath, ec);
            std::cerr << "Could not save audio to the output file.\n";
            return 1;
        }

        if (numClips > 0)
            std::cerr << "Warning: the audio output has been clipped on " << numClips << " frames.\n";
    }
    else {
        std::ostream* os = &std::cout;
        fs::ofstream of;

        fs::path outputPath;
        if (!outputFilename.empty()) {
            outputPath = fs::u8path(outputFilename);
            of.open(outputPath);
            os = &of;
        }

        for (size_t i = 0; i < numFrames; ++i) {
            *os << (i / sampleRate);
            for (size_t l = 0; l < numLfos; ++l)
                *os << ' ' << lfoOutputs[l][i];
            *os << '\n';
        }

        if (os == &of) {
            of.flush();
            if (!of) {
                std::error_code ec;
                fs::remove(outputPath, ec);
                std::cerr << "Could not save data to the output file.\n";
                return 1;
            }
        }
    }

    return 0;
}
