// Copyright (c) 2019, Paul Ferrand
// All rights reserved.

// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are met:

// 1. Redistributions of source code must retain the above copyright notice, this
//    list of conditions and the following disclaimer.
// 2. Redistributions in binary form must reproduce the above copyright notice,
//    this list of conditions and the following disclaimer in the documentation
//    and/or other materials provided with the distribution.

// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
// ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
// WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
// DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR
// ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
// (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
// LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
// ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
// (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
// SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

#include "sfizz.hpp"
#include "sfizz/import/sfizz_import.h"
#include "MidiHelpers.h"
#include <absl/flags/parse.h>
#include <absl/flags/flag.h>
#include <absl/types/span.h>
#include <SpinMutex.h>
#include <atomic>
#include <cstddef>
#include <ios>
#include <iostream>
#include <jack/jack.h>
#include <jack/midiport.h>
#include <jack/types.h>
#include <ostream>
#include <signal.h>
#include <string_view>
#include <chrono>
#include <thread>
#include <mutex>
#include <algorithm>
#include <vector>

sfz::Sfizz synth;

static jack_port_t* midiInputPort;
static jack_port_t* outputPort1;
static jack_port_t* outputPort2;
static jack_client_t* client;
static SpinMutex processMutex;

int process(jack_nframes_t numFrames, void* arg)
{
    auto* synth = reinterpret_cast<sfz::Sfizz*>(arg);

    auto* buffer = jack_port_get_buffer(midiInputPort, numFrames);
    assert(buffer);

    auto* leftOutput = reinterpret_cast<float*>(jack_port_get_buffer(outputPort1, numFrames));
    auto* rightOutput = reinterpret_cast<float*>(jack_port_get_buffer(outputPort2, numFrames));

    std::unique_lock<SpinMutex> lock { processMutex, std::try_to_lock };
    if (!lock.owns_lock()) {
        std::fill_n(leftOutput, numFrames, 0.0f);
        std::fill_n(rightOutput, numFrames, 0.0f);
        return 0;
    }

    auto numMidiEvents = jack_midi_get_event_count(buffer);
    jack_midi_event_t event;

    // Midi dispatching
    for (uint32_t i = 0; i < numMidiEvents; ++i) {
        if (jack_midi_event_get(&event, buffer, i) != 0)
            continue;

        if (event.size == 0)
            continue;

        switch (midi::status(event.buffer[0])) {
        case midi::noteOff: noteoff:
            synth->noteOff(event.time, event.buffer[1], event.buffer[2]);
            break;
        case midi::noteOn:
            if (event.buffer[2] == 0)
                goto noteoff;
            synth->noteOn(event.time, event.buffer[1], event.buffer[2]);
            break;
        case midi::polyphonicPressure:
            synth->polyAftertouch(event.time, event.buffer[1], event.buffer[2]);
            break;
        case midi::controlChange:
            synth->cc(event.time, event.buffer[1], event.buffer[2]);
            break;
        case midi::programChange:
            // Not implemented
            break;
        case midi::channelPressure:
            synth->channelAftertouch(event.time, event.buffer[1]);
            break;
        case midi::pitchBend:
            synth->pitchWheel(event.time, midi::buildAndCenterPitch(event.buffer[1], event.buffer[2]));
            break;
        case midi::systemMessage:
            // Not implemented
            break;
        }
    }

    float* stereoOutput[] = { leftOutput, rightOutput };
    synth->renderBlock(stereoOutput, numFrames);

    return 0;
}

int sampleBlockChanged(jack_nframes_t nframes, void* arg)
{
    if (arg == nullptr)
        return 0;

    auto* synth = reinterpret_cast<sfz::Sfizz*>(arg);
    // DBG("Sample per block changed to " << nframes);
    std::lock_guard<SpinMutex> lock { processMutex };
    synth->setSamplesPerBlock(nframes);
    return 0;
}

int sampleRateChanged(jack_nframes_t nframes, void* arg)
{
    if (arg == nullptr)
        return 0;

    auto* synth = reinterpret_cast<sfz::Sfizz*>(arg);
    // DBG("Sample rate changed to " << nframes);
    std::lock_guard<SpinMutex> lock { processMutex };
    synth->setSampleRate(nframes);
    return 0;
}

static volatile sig_atomic_t shouldClose { false };

static void done(int sig)
{
    std::cout << "Signal received" << '\n';
    shouldClose = true;
    (void)sig;
    // if (client != nullptr)

    // exit(0);
}

bool loadInstrument(const char* fpath)
{
    const char* importFormat = nullptr;
    if (!sfizz_load_or_import_file(synth.handle(), fpath, &importFormat)) {
        std::cout << "Could not load the instrument file: " << fpath << '\n';
        return false;
    }

    std::cout << "Instrument loaded: " << fpath << '\n';
    std::cout << "===========================" << '\n';
    std::cout << "Total:" << '\n';
    std::cout << "\tMasters: " << synth.getNumMasters() << '\n';
    std::cout << "\tGroups: " << synth.getNumGroups() << '\n';
    std::cout << "\tRegions: " << synth.getNumRegions() << '\n';
    std::cout << "\tCurves: " << synth.getNumCurves() << '\n';
    std::cout << "\tPreloadedSamples: " << synth.getNumPreloadedSamples() << '\n';
#if 0 // not currently in public API
    std::cout << "===========================" << '\n';
    std::cout << "Included files:" << '\n';
    for (auto& file : synth.getParser().getIncludedFiles())
        std::cout << '\t' << file << '\n';
    std::cout << "===========================" << '\n';
    std::cout << "Defines:" << '\n';
    for (auto& define : synth.getParser().getDefines())
        std::cout << '\t' << define.first << '=' << define.second << '\n';
#endif
    std::cout << "===========================" << '\n';
    std::cout << "Unknown opcodes:";
    for (auto& opcode : synth.getUnknownOpcodes())
        std::cout << opcode << ',';
    std::cout << '\n';
    if (importFormat) {
        std::cout << "===========================" << '\n';
        std::cout << "Import format: " << importFormat << '\n';
    }
    // std::cout << std::flush;

    return true;
}

std::vector<std::string> stringTokenize(const std::string& str)
{
    std::vector<std::string> tokens;
    std::string part = "";
    for (size_t i = 0; i < str.length(); i++) {
        char c = str[i];
        if (c == ' ' && part != "") {
            tokens.push_back(part);
            part = "";
        } else if (c == '\"') {
            i++;
            while (str[i] != '\"') {
                part += str[i];
                i++;
            }
            tokens.push_back(part);
            part = "";
        } else {
            part += c;
        }
    }
    if (part != "") {
        tokens.push_back(part);
    }
    return tokens;
}

void cliThreadProc()
{
    while (!shouldClose) {
        std::cout << "\n> ";

        std::string command;
        std::getline(std::cin, command);
        std::size_t pos = command.find(" ");
        std::string kw = command.substr(0, pos);
        std::string args = command.substr(pos + 1);
        std::vector<std::string> tokens = stringTokenize(args);

        if (kw == "load_instrument") {
            try {
                std::lock_guard<SpinMutex> lock { processMutex };
                loadInstrument(tokens[0].c_str());
            } catch (...) {
                std::cout << "ERROR: Can't load instrument!\n";
            }
        } else if (kw == "set_oversampling") {
            try {
                std::lock_guard<SpinMutex> lock { processMutex };
                synth.setOversamplingFactor(stoi(args));
            } catch (...) {
                std::cout << "ERROR: Can't set oversampling!\n";
            }
        } else if (kw == "set_preload_size") {
            try {
                std::lock_guard<SpinMutex> lock { processMutex };
                synth.setPreloadSize(stoi(args));
            } catch (...) {
                std::cout << "ERROR: Can't set preload size!\n";
            }
        } else if (kw == "set_voices") {
            try {
                std::lock_guard<SpinMutex> lock { processMutex };
                synth.setNumVoices(stoi(args));
            } catch (...) {
                std::cout << "ERROR: Can't set num of voices!\n";
            }
        } else if (kw == "quit") {
            shouldClose = true;
        } else if (kw.size() > 0) {
            std::cout << "ERROR: Unknown command '" << kw << "'!\n";
        }
    }
}

ABSL_FLAG(std::string, client_name, "sfizz", "Jack client name");
ABSL_FLAG(std::string, oversampling, "1x", "Internal oversampling factor (value values are x1, x2, x4, x8)");
ABSL_FLAG(uint32_t, preload_size, 8192, "Preloaded size");
ABSL_FLAG(uint32_t, num_voices, 32, "Num of voices");
ABSL_FLAG(bool, jack_autoconnect, false, "Autoconnect audio output");
ABSL_FLAG(bool, state, false, "Output the synth state in the jack loop");

int main(int argc, char** argv)
{
    auto arguments = absl::ParseCommandLine(argc, argv);

    auto filesToParse = absl::MakeConstSpan(arguments).subspan(1);
    const std::string clientName = absl::GetFlag(FLAGS_client_name);
    const std::string oversampling = absl::GetFlag(FLAGS_oversampling);
    const uint32_t preload_size = absl::GetFlag(FLAGS_preload_size);
    const uint32_t num_voices = absl::GetFlag(FLAGS_num_voices);
    const bool jack_autoconnect = absl::GetFlag(FLAGS_jack_autoconnect);
    const bool verboseState = absl::GetFlag(FLAGS_state);

    std::cout << "Flags" << '\n';
    std::cout << "- Client name: " << clientName << '\n';
    std::cout << "- Oversampling: " << oversampling << '\n';
    std::cout << "- Preloaded size: " << preload_size << '\n';
    std::cout << "- Num of voices: " << num_voices << '\n';
    std::cout << "- Audio Autoconnect: " << jack_autoconnect << '\n';
    std::cout << "- Verbose State: " << verboseState << '\n';

    const auto factor = [&]() {
        if (oversampling == "x1") return 1;
        if (oversampling == "x2") return 2;
        if (oversampling == "x4") return 4;
        if (oversampling == "x8") return 8;
        return 1;
    }();

    std::cout << "Positional arguments:";
    for (auto& file : filesToParse)
        std::cout << " " << file << ',';
    std::cout << '\n';

    synth.setOversamplingFactor(factor);
    synth.setPreloadSize(preload_size);
    synth.setNumVoices(num_voices);

    jack_status_t status;
    client = jack_client_open(clientName.c_str(), JackNullOption, &status);
    if (client == nullptr) {
        std::cerr << "Could not open JACK client" << '\n';
        // if (status & JackFailure)
        //     std::cerr << "JackFailure: Overall operation failed" << '\n';
        return 1;
    }

    if (status & JackNameNotUnique) {
        std::cout << "Name was taken: assigned " << jack_get_client_name(client) << "instead" << '\n';
    }
    if (status & JackServerStarted) {
        std::cout << "Connected to JACK" << '\n';
    }

    synth.setSamplesPerBlock(jack_get_buffer_size(client));
    synth.setSampleRate(jack_get_sample_rate(client));

    jack_set_sample_rate_callback(client, sampleRateChanged, &synth);
    jack_set_buffer_size_callback(client, sampleBlockChanged, &synth);
    jack_set_process_callback(client, process, &synth);

    midiInputPort = jack_port_register(client, "input", JACK_DEFAULT_MIDI_TYPE, JackPortIsInput, 0);
    if (midiInputPort == nullptr) {
        std::cerr << "Could not open MIDI input port" << '\n';
        return 1;
    }

    outputPort1 = jack_port_register(client, "output_1", JACK_DEFAULT_AUDIO_TYPE, JackPortIsOutput, 0);
    outputPort2 = jack_port_register(client, "output_2", JACK_DEFAULT_AUDIO_TYPE, JackPortIsOutput, 0);
    if (outputPort1 == nullptr || outputPort2 == nullptr) {
        std::cerr << "Could not open output ports" << '\n';
        return 1;
    }

    if (jack_activate(client) != 0) {
        std::cerr << "Could not activate client" << '\n';
        return 1;
    }

    if (jack_autoconnect) {
        auto systemPorts = jack_get_ports(client, nullptr, nullptr, JackPortIsPhysical | JackPortIsInput);
        if (systemPorts == nullptr) {
            std::cerr << "No physical output ports found" << '\n';
            return 1;
        }

        if (jack_connect(client, jack_port_name(outputPort1), systemPorts[0])) {
            std::cerr << "Cannot connect to physical output ports (0)" << '\n';
        }

        if (jack_connect(client, jack_port_name(outputPort2), systemPorts[1])) {
            std::cerr << "Cannot connect to physical output ports (1)" << '\n';
        }
        jack_free(systemPorts);
    }

    if (!filesToParse.empty() && filesToParse[0]) {
        loadInstrument(filesToParse[0]);
    }

    std::thread cli_thread(cliThreadProc);

    signal(SIGHUP, done);
    signal(SIGINT, done);
    signal(SIGTERM, done);
    signal(SIGQUIT, done);

    while (!shouldClose) {
        if (verboseState) {
            std::cout << "Active voices: " << synth.getNumActiveVoices() << '\n';
#ifndef NDEBUG
            std::cout << "Allocated buffers: " << synth.getAllocatedBuffers() << '\n';
            std::cout << "Total size: " << synth.getAllocatedBytes() << '\n';
#endif
        }
        std::this_thread::sleep_for(std::chrono::seconds(1));
    }

    std::cout << "Closing..." << '\n';
    jack_client_close(client);
    return 0;
}
