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
    float frequency { 50.0f };
    float bandwidth { 1.0f };

    options.add_options()
        ("g,gain", "Gain", cxxopts::value(gain))
        ("f,frequency", "Frequency", cxxopts::value(frequency))
        ("b,bandwidth", "Bandwidth", cxxopts::value(bandwidth))
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
    std::cout << "Frequency: " << frequency << '\n';
    std::cout << "Bandwidth: " << bandwidth << '\n';

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
    sfz::FilterEq eq;
    eq.init(sndfile.samplerate());
    float* in [1] = { left.data() };
    float* out [1] = { output.data() };
    eq.process(in, out, frequency, bandwidth, gain, numFrames);

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
