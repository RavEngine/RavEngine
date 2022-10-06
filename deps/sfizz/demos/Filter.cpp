#include "sfizz/SfzFilter.h"
#include "sfizz/Buffer.h"
#include "sfizz/SIMDHelpers.h"
#include "sfizz/utility/StringViewHelpers.h"
#include <sndfile.hh>
#include <cxxopts.hpp>
#include <ghc/fs_std.hpp>
#include <string>
#include <iostream>

int main(int argc, char** argv)
{
    (void)argc;

    cxxopts::Options options("test", "A brief description");

    float gain { 0.0f };
    float cutoff { 50.0f };
    float resonance { 1.0f };
    std::string filterType { };

    options.add_options()
        ("g,gain", "Gain", cxxopts::value(gain))
        ("c,cutoff", "Cutoff", cxxopts::value(cutoff))
        ("r,resonance", "Resonance", cxxopts::value(resonance))
        ("q", "Q factor", cxxopts::value(resonance))
        ("h,help", "Print usage")
    ;

    auto result = options.parse(argc, argv);
    // options.parse_positional({ "filename" });

    if (result.count("help"))
    {
        std::cout << options.help() << std::endl;
        return 0;
    }

    if (argc == 1) {
        std::cout << "Need a file name" << '\n';
        return -1;
    }

    std::cout << "File: " << argv[1]  << '\n';
    std::cout << "Gain: " << gain << '\n';
    std::cout << "Cutoff: " << cutoff << '\n';
    std::cout << "Filter type: " << filterType << '\n';
    std::cout << "Resonance: " << resonance << '\n';

    auto path = fs::current_path() / argv[1];
    if (!fs::exists(path)) {
        std::cout << "Can't find " << argv[1] << '\n';
        return -1;
    }

    std::cout << "Opening " << path.native() << '\n';
    SndfileHandle sndfile { path.native() };
    if (sndfile.error()) {
        std::cout << "Input file error" << '\n';
        std::cout << sndfile.strError() << '\n';
        return -1;
    }

    auto numFrames = static_cast<size_t>(sndfile.frames());

    sfz::Buffer<float> left { numFrames };

    if (sndfile.channels() == 2) {
        sfz::Buffer<float> buffer { numFrames * 2 };
        sfz::Buffer<float> right { numFrames };
        sndfile.readf(buffer.data(), numFrames * 2 );
        sfz::readInterleaved(buffer, absl::MakeSpan(left), absl::MakeSpan(right));
    } else if (sndfile.channels() == 1) {
        sndfile.readf(left.data(), numFrames);
    } else {
        std::cout << "Unhandled number of channels:" << sndfile.channels() << '\n';
    }

    sfz::Buffer<float> output { numFrames };
    sfz::Filter filter;
    filter.init(sndfile.samplerate());

    switch(hash(filterType)) {
        case hash("Apf1p"): filter.setType(sfz::FilterType::kFilterApf1p); break;
        case hash("Bpf1p"): filter.setType(sfz::FilterType::kFilterBpf1p); break;
        case hash("Bpf2p"): filter.setType(sfz::FilterType::kFilterBpf2p); break;
        case hash("Bpf4p"): filter.setType(sfz::FilterType::kFilterBpf4p); break;
        case hash("Bpf6p"): filter.setType(sfz::FilterType::kFilterBpf6p); break;
        case hash("Brf1p"): filter.setType(sfz::FilterType::kFilterBrf1p); break;
        case hash("Brf2p"): filter.setType(sfz::FilterType::kFilterBrf2p); break;
        case hash("Hpf1p"): filter.setType(sfz::FilterType::kFilterHpf1p); break;
        case hash("Hpf2p"): filter.setType(sfz::FilterType::kFilterHpf2p); break;
        case hash("Hpf4p"): filter.setType(sfz::FilterType::kFilterHpf4p); break;
        case hash("Hpf6p"): filter.setType(sfz::FilterType::kFilterHpf6p); break;
        case hash("Lpf1p"): filter.setType(sfz::FilterType::kFilterLpf1p); break;
        case hash("Lpf2p"): filter.setType(sfz::FilterType::kFilterLpf2p); break;
        case hash("Lpf4p"): filter.setType(sfz::FilterType::kFilterLpf4p); break;
        case hash("Lpf6p"): filter.setType(sfz::FilterType::kFilterLpf6p); break;
        case hash("Pink"): filter.setType(sfz::FilterType::kFilterPink); break;
        case hash("Lpf2pSv"): filter.setType(sfz::FilterType::kFilterLpf2pSv); break;
        case hash("Hpf2pSv"): filter.setType(sfz::FilterType::kFilterHpf2pSv); break;
        case hash("Bpf2pSv"): filter.setType(sfz::FilterType::kFilterBpf2pSv); break;
        case hash("Brf2pSv"): filter.setType(sfz::FilterType::kFilterBrf2pSv); break;
        case hash("Lsh"): filter.setType(sfz::FilterType::kFilterLsh); break;
        case hash("Hsh"): filter.setType(sfz::FilterType::kFilterHsh); break;
        case hash("Peq"): filter.setType(sfz::FilterType::kFilterPeq); break;
        default:
            std::cout << "Unknown filter type" << '\n';
            return -1;
    }

    float* in [1] = { left.data() };
    float* out [1] = { output.data() };
    filter.process(in, out, cutoff, resonance, gain, numFrames);

    auto outputFile = fs::current_path() / path.stem().concat("_processed").concat(path.extension());
    SndfileHandle outSndfile { outputFile.native(), SFM_WRITE, sndfile.format(), 1, sndfile.samplerate() };
    if (outSndfile.error() != 0 && outSndfile.error() != 2) {
        std::cout << "Output file error: " << outSndfile.error() << '\n';
        std::cout << outSndfile.strError() << '\n';
        return -1;
    }
    std::cout << "Writing to " << outputFile.native() << '\n';
    outSndfile.writef(output.data(), output.size());

    return 0;
}
