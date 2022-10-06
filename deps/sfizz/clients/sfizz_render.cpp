
#include "sfizz/Synth.h"
#include "sfizz/MathHelpers.h"
#include "sfizz/SfzHelpers.h"
#include "sfizz/SIMDHelpers.h"
#include "MidiHelpers.h"
#include <st_audiofile_libs.h>
#include <cxxopts.hpp>
#include <fmidi/fmidi.h>
#include <iostream>
#include <fstream>

#define LOG_ERROR(ostream) std::cerr  << ostream << '\n'
#define LOG_INFO(ostream) if (verbose) { std::cout << ostream << '\n'; }
#define ERROR_IF(check, ostream) if ((check)) { LOG_ERROR(ostream); std::exit(-1); }

struct CallbackData {
    sfz::Synth& synth;
    unsigned delay;
    bool finished;
};

void midiCallback(const fmidi_event_t * event, void * cbdata)
{
    auto data = reinterpret_cast<CallbackData*>(cbdata);

    if (event->type != fmidi_event_type::fmidi_event_message)
        return;

    switch (midi::status(event->data[0])) {
        case midi::noteOff:
            data->synth.noteOff(data->delay, event->data[1], event->data[2]);
            break;
        case midi::noteOn:
            data->synth.noteOn(data->delay, event->data[1], event->data[2]);
            break;
        case midi::polyphonicPressure:
            break;
        case midi::controlChange:
            data->synth.cc(data->delay, event->data[1], event->data[2]);
            break;
        case midi::programChange:
            break;
        case midi::channelPressure:
            break;
        case midi::pitchBend:
            data->synth.pitchWheel(data->delay, midi::buildAndCenterPitch(event->data[1], event->data[2]));
            break;
        case midi::systemMessage:
            break;
        }
}

void finishedCallback(void * cbdata)
{
    auto data = reinterpret_cast<CallbackData*>(cbdata);
    data->finished = true;
}

int main(int argc, char** argv)
{
    cxxopts::Options options("sfizz-render", "Render a midi file through an SFZ file using the sfizz library.");

    unsigned blockSize { 1024 };
    int sampleRate { 48000 };
    bool verbose { false };
    bool help { false };
    bool useEOT { false };
    int quality { 2 };
    int polyphony { 64 };

    options.add_options()
        ("sfz", "SFZ file", cxxopts::value<std::string>())
        ("midi", "Input midi file", cxxopts::value<std::string>())
        ("wav", "Output wav file", cxxopts::value<std::string>())
        ("b,blocksize", "Block size for the sfizz callbacks", cxxopts::value(blockSize))
        ("s,samplerate", "Output sample rate", cxxopts::value(sampleRate))
        ("q,quality", "Resampling quality", cxxopts::value(quality))
        ("p,polyphony", "Polyphony max", cxxopts::value(polyphony))
        ("v,verbose", "Verbose output", cxxopts::value(verbose))
        ("log", "Produce logs", cxxopts::value<std::string>())
        ("use-eot", "End the rendering at the last End of Track Midi message", cxxopts::value(useEOT))
        ("h,help", "Show help", cxxopts::value(help))
    ;
    auto params = [&]() {
        try { return options.parse(argc, argv); }
        catch (std::exception& e) {
            LOG_ERROR(e.what());
            std::exit(-1);
        }
    }();

    if (help) {
        std::cout << options.help();
        std::exit(0);
    }

    ERROR_IF(params.count("sfz") != 1, "Please specify a single SFZ file using --sfz");
    ERROR_IF(params.count("wav") != 1, "Please specify a single WAV file using --wav");
    ERROR_IF(params.count("midi") != 1, "Please specify a single MIDI file using --midi");

    fs::path sfzPath  = fs::current_path() / params["sfz"].as<std::string>();
    fs::path outputPath  = fs::current_path() / params["wav"].as<std::string>();
    fs::path midiPath  = fs::current_path() / params["midi"].as<std::string>();

    ERROR_IF(!fs::exists(sfzPath) || !fs::is_regular_file(sfzPath),
                    "SFZ file " << sfzPath.string() << " does not exist or is not a regular file");
    ERROR_IF(!fs::exists(midiPath) || !fs::is_regular_file(midiPath),
            "MIDI file " << midiPath.string() << " does not exist or is not a regular file");

    if (fs::exists(outputPath)) {
        LOG_INFO("Output file " << outputPath.string() << " already exists and will be erased.");
    }

    LOG_INFO("SFZ file:    " << sfzPath.string());
    LOG_INFO("MIDI file:   " << midiPath.string());
    LOG_INFO("Output file: " << outputPath.string());
    LOG_INFO("Block size: " << blockSize);
    LOG_INFO("Sample rate: " << sampleRate);
    LOG_INFO("Polyphony Max: " << polyphony);

    sfz::Synth synth;
    synth.setSamplesPerBlock(blockSize);
    synth.setSampleRate(sampleRate);
    synth.setSampleQuality(sfz::Synth::ProcessMode::ProcessFreewheeling, quality);
    synth.setNumVoices(polyphony);
    synth.enableFreeWheeling();

    bool logging = params.count("log") > 0;
    std::string logFilename {};
    std::ofstream callbackLogFile {};
    if (logging) {
        logFilename = params["log"].as<std::string>();
        fs::path logPath{ fs::current_path() / logFilename };
        callbackLogFile.open(logPath.string());

        if (callbackLogFile.is_open()) {
            callbackLogFile << "Dispatch,RenderMethod,Data,Amplitude,Filters,Panning,Effects,NumVoices,NumSamples" << '\n';
        } else {
            logging = false;
            LOG_INFO("Error opening log file " << logPath.string() << "; logging will be disabled");
        }
    }

    auto writeLogLine = [&] {
        if (!logging)
            return;

        auto breakdown = synth.getCallbackBreakdown();
        auto numVoices = synth.getNumActiveVoices();
        callbackLogFile << breakdown.dispatch << ','
                        << breakdown.renderMethod << ','
                        << breakdown.data << ','
                        << breakdown.amplitude << ','
                        << breakdown.filters << ','
                        << breakdown.panning << ','
                        << breakdown.effects << ','
                        << numVoices << ','
                        << blockSize << '\n';
    };

    ERROR_IF(!synth.loadSfzFile(sfzPath), "There was an error loading the SFZ file.");
    LOG_INFO(synth.getNumRegions() << " regions in the SFZ.");

    fmidi_smf_u midiFile { fmidi_smf_file_read(midiPath.u8string().c_str()) };
    ERROR_IF(!midiFile, "Can't read " << midiPath);

    const auto* midiInfo = fmidi_smf_get_info(midiFile.get());
    ERROR_IF(!midiInfo, "Can't get info on the midi file");

    LOG_INFO( midiInfo->track_count << " tracks in the SMF.");

    if (useEOT) {
        LOG_INFO("-- Cutting the rendering at the last MIDI End of Track message");
    }

    drwav outputFile;
    drwav_data_format outputFormat {};
    outputFormat.container = drwav_container_riff;
    outputFormat.format = DR_WAVE_FORMAT_PCM;
    outputFormat.channels = 2;
    outputFormat.sampleRate = sampleRate;
    outputFormat.bitsPerSample = 16;

#if !defined(_WIN32)
    drwav_bool32 outputFileOk = drwav_init_file_write(&outputFile, outputPath.c_str(), &outputFormat, nullptr);
#else
    drwav_bool32 outputFileOk = drwav_init_file_write_w(&outputFile, outputPath.c_str(), &outputFormat, nullptr);
#endif
    ERROR_IF(!outputFileOk, "Error opening the wav file for writing");

    auto sampleRateDouble = static_cast<double>(sampleRate);
    const double increment { 1.0 / sampleRateDouble };
    uint64_t numFramesWritten { 0 };
    sfz::AudioBuffer<float> audioBuffer { 2, blockSize };
    sfz::Buffer<float> interleavedBuffer { 2 * blockSize };
    sfz::Buffer<int16_t> interleavedPcm { 2 * blockSize };

    fmidi_player_u midiPlayer { fmidi_player_new(midiFile.get()) };
    CallbackData callbackData { synth, 0, false };
    fmidi_player_event_callback(midiPlayer.get(), &midiCallback, &callbackData);
    fmidi_player_finish_callback(midiPlayer.get(), &finishedCallback, &callbackData);

    fmidi_player_start(midiPlayer.get());
    while (!callbackData.finished) {
        for (callbackData.delay = 0; callbackData.delay < blockSize && !callbackData.finished; callbackData.delay++)
            fmidi_player_tick(midiPlayer.get(), increment);
        synth.renderBlock(audioBuffer);
        sfz::writeInterleaved(audioBuffer.getConstSpan(0), audioBuffer.getConstSpan(1), absl::MakeSpan(interleavedBuffer));
        drwav_f32_to_s16(interleavedPcm.data(), interleavedBuffer.data(), 2 * blockSize);
        numFramesWritten += drwav_write_pcm_frames(&outputFile, blockSize, interleavedPcm.data());
        writeLogLine();
    }

    if (!useEOT) {
        auto averagePower = sfz::meanSquared<float>(interleavedBuffer);
        while (averagePower > 1e-12f) {
            synth.renderBlock(audioBuffer);
            sfz::writeInterleaved(audioBuffer.getConstSpan(0), audioBuffer.getConstSpan(1), absl::MakeSpan(interleavedBuffer));
            drwav_f32_to_s16(interleavedPcm.data(), interleavedBuffer.data(), 2 * blockSize);
            numFramesWritten += drwav_write_pcm_frames(&outputFile, blockSize, interleavedPcm.data());
            writeLogLine();
            averagePower = sfz::meanSquared<float>(interleavedBuffer);
        }
    }

    drwav_uninit(&outputFile);
    LOG_INFO("Wrote " << numFramesWritten << " frames of sound data in" << outputPath.string());

    return 0;
}
