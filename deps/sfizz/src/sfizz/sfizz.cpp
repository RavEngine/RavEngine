// SPDX-License-Identifier: BSD-2-Clause

// This code is part of the sfizz library and is licensed under a BSD 2-clause
// license. You should have receive a LICENSE.md file along with the code.
// If not, contact the sfizz maintainers at https://github.com/sfztools/sfizz

#include "Synth.h"
#include "Messaging.h"
#include "sfizz.hpp"
#include "sfizz_private.hpp"
#include "absl/memory/memory.h"

sfz::Sfizz::Sfizz()
    : synth(new sfizz_synth_t)
{
}

sfz::Sfizz::~Sfizz()
{
    if (synth)
        synth->forget();
}

sfz::Sfizz::Sfizz(sfizz_synth_t* synth)
    : synth(synth)
{
    if (synth)
        synth->remember();
}

sfz::Sfizz::Sfizz(Sfizz&& other) noexcept
    : synth(other.synth)
{
    other.synth = nullptr;
}

sfz::Sfizz& sfz::Sfizz::operator=(Sfizz&& other) noexcept
{
    if (this != &other) {
        if (synth)
            synth->forget();
        synth = other.synth;
        other.synth = nullptr;
    }
    return *this;
}

bool sfz::Sfizz::loadSfzFile(const std::string& path)
{
    return synth->synth.loadSfzFile(path);
}

bool sfz::Sfizz::loadSfzString(const std::string& path, const std::string& text)
{
    return synth->synth.loadSfzString(path, text);
}

bool sfz::Sfizz::loadScalaFile(const std::string& path)
{
    return synth->synth.loadScalaFile(path);
}

bool sfz::Sfizz::loadScalaString(const std::string& text)
{
    return synth->synth.loadScalaString(text);
}

void sfz::Sfizz::setScalaRootKey(int rootKey)
{
    return synth->synth.setScalaRootKey(rootKey);
}

int sfz::Sfizz::getScalaRootKey() const
{
    return synth->synth.getScalaRootKey();
}

void sfz::Sfizz::setTuningFrequency(float frequency)
{
    return synth->synth.setTuningFrequency(frequency);
}

float sfz::Sfizz::getTuningFrequency() const
{
    return synth->synth.getTuningFrequency();
}

void sfz::Sfizz::loadStretchTuningByRatio(float ratio)
{
    return synth->synth.loadStretchTuningByRatio(ratio);
}

int sfz::Sfizz::getNumRegions() const noexcept
{
    return synth->synth.getNumRegions();
}

int sfz::Sfizz::getNumGroups() const noexcept
{
    return synth->synth.getNumGroups();
}

int sfz::Sfizz::getNumMasters() const noexcept
{
    return synth->synth.getNumMasters();
}

int sfz::Sfizz::getNumCurves() const noexcept
{
    return synth->synth.getNumCurves();
}

const std::vector<std::string>& sfz::Sfizz::getUnknownOpcodes() const noexcept
{
    return synth->synth.getUnknownOpcodes();
}

size_t sfz::Sfizz::getNumPreloadedSamples() const noexcept
{
    return synth->synth.getNumPreloadedSamples();
}

void sfz::Sfizz::setSamplesPerBlock(int samplesPerBlock) noexcept
{
    synth->synth.setSamplesPerBlock(samplesPerBlock);
}

void sfz::Sfizz::setSampleRate(float sampleRate) noexcept
{
    synth->synth.setSampleRate(sampleRate);
}

int sfz::Sfizz::getSampleQuality(ProcessMode mode)
{
    return synth->synth.getSampleQuality(static_cast<sfz::Synth::ProcessMode>(mode));
}

void sfz::Sfizz::setSampleQuality(ProcessMode mode, int quality)
{
    synth->synth.setSampleQuality(static_cast<sfz::Synth::ProcessMode>(mode), quality);
}

int sfz::Sfizz::getOscillatorQuality(ProcessMode mode)
{
    return synth->synth.getOscillatorQuality(static_cast<sfz::Synth::ProcessMode>(mode));
}

void sfz::Sfizz::setOscillatorQuality(ProcessMode mode, int quality)
{
    synth->synth.setOscillatorQuality(static_cast<sfz::Synth::ProcessMode>(mode), quality);
}

void sfz::Sfizz::setSustainCancelsRelease(bool value)
{
    synth->synth.setSustainCancelsRelease(value);
}

float sfz::Sfizz::getVolume() const noexcept
{
    return synth->synth.getVolume();
}

void sfz::Sfizz::setVolume(float volume) noexcept
{
    synth->synth.setVolume(volume);
}

void sfz::Sfizz::noteOn(int delay, int noteNumber, int velocity) noexcept
{
    synth->synth.noteOn(delay, noteNumber, velocity);
}

void sfz::Sfizz::hdNoteOn(int delay, int noteNumber, float velocity) noexcept
{
    synth->synth.hdNoteOn(delay, noteNumber, velocity);
}

void sfz::Sfizz::noteOff(int delay, int noteNumber, int velocity) noexcept
{
    synth->synth.noteOff(delay, noteNumber, velocity);
}

void sfz::Sfizz::hdNoteOff(int delay, int noteNumber, float velocity) noexcept
{
    synth->synth.hdNoteOff(delay, noteNumber, velocity);
}

void sfz::Sfizz::cc(int delay, int ccNumber, int ccValue) noexcept
{
    synth->synth.cc(delay, ccNumber, ccValue);
}

void sfz::Sfizz::hdcc(int delay, int ccNumber, float normValue) noexcept
{
    synth->synth.hdcc(delay, ccNumber, normValue);
}

void sfz::Sfizz::automateHdcc(int delay, int ccNumber, float normValue) noexcept
{
    synth->synth.automateHdcc(delay, ccNumber, normValue);
}

void sfz::Sfizz::programChange(int delay, int program) noexcept
{
    synth->synth.programChange(delay, program);
}

void sfz::Sfizz::pitchWheel(int delay, int pitch) noexcept
{
    synth->synth.pitchWheel(delay, pitch);
}

void sfz::Sfizz::hdPitchWheel(int delay, float pitch) noexcept
{
    synth->synth.hdPitchWheel(delay, pitch);
}

void sfz::Sfizz::aftertouch(int delay, int aftertouch) noexcept
{
    synth->synth.channelAftertouch(delay, aftertouch);
}

void sfz::Sfizz::channelAftertouch(int delay, int aftertouch) noexcept
{
    synth->synth.channelAftertouch(delay, aftertouch);
}

void sfz::Sfizz::hdChannelAftertouch(int delay, float aftertouch) noexcept
{
    synth->synth.hdChannelAftertouch(delay, aftertouch);
}

void sfz::Sfizz::polyAftertouch(int delay, int noteNumber, int aftertouch) noexcept
{
    synth->synth.polyAftertouch(delay, noteNumber, aftertouch);
}

void sfz::Sfizz::hdPolyAftertouch(int delay, int noteNumber, float aftertouch) noexcept
{
    synth->synth.hdPolyAftertouch(delay, noteNumber, aftertouch);
}

void sfz::Sfizz::tempo(int delay, float secondsPerBeat) noexcept
{
    synth->synth.tempo(delay, secondsPerBeat);
}

void sfz::Sfizz::bpmTempo(int delay, float beatsPerMinute) noexcept
{
    synth->synth.bpmTempo(delay, beatsPerMinute);
}

void sfz::Sfizz::timeSignature(int delay, int beatsPerBar, int beatUnit)
{
    synth->synth.timeSignature(delay, beatsPerBar, beatUnit);
}

void sfz::Sfizz::timePosition(int delay, int bar, double barBeat)
{
    synth->synth.timePosition(delay, bar, barBeat);
}

void sfz::Sfizz::playbackState(int delay, int playbackState)
{
    synth->synth.playbackState(delay, playbackState);
}

void sfz::Sfizz::renderBlock(float** buffers, size_t numSamples, int numOutputs) noexcept
{
    sfz::AudioSpan<float> bufferSpan { buffers, static_cast<size_t>(numOutputs * 2), 0, numSamples };
    synth->synth.renderBlock(bufferSpan);
}

int sfz::Sfizz::getNumActiveVoices() const noexcept
{
    return synth->synth.getNumActiveVoices();
}

int sfz::Sfizz::getNumVoices() const noexcept
{
    return synth->synth.getNumVoices();
}

void sfz::Sfizz::setNumVoices(int numVoices) noexcept
{
    synth->synth.setNumVoices(numVoices);
}

bool sfz::Sfizz::setOversamplingFactor(int) noexcept
{
    return true;
}


int sfz::Sfizz::getOversamplingFactor() const noexcept
{
    return 1;
}

void sfz::Sfizz::setPreloadSize(uint32_t preloadSize) noexcept
{
    synth->synth.setPreloadSize(preloadSize);
}

uint32_t sfz::Sfizz::getPreloadSize() const noexcept
{
    return synth->synth.getPreloadSize();
}

int sfz::Sfizz::getAllocatedBuffers() const noexcept
{
    return synth->synth.getAllocatedBuffers();
}

int sfz::Sfizz::getAllocatedBytes() const noexcept
{
    return synth->synth.getAllocatedBytes();
}

void sfz::Sfizz::enableFreeWheeling() noexcept
{
    synth->synth.enableFreeWheeling();
}

void sfz::Sfizz::disableFreeWheeling() noexcept
{
    synth->synth.disableFreeWheeling();
}

bool sfz::Sfizz::shouldReloadFile()
{
    return synth->synth.shouldReloadFile();
}

bool sfz::Sfizz::shouldReloadScala()
{
    return synth->synth.shouldReloadScala();
}

void sfz::Sfizz::enableLogging() noexcept
{
}

void sfz::Sfizz::enableLogging(const std::string& prefix) noexcept
{
    (void)prefix;
}

void sfz::Sfizz::setLoggingPrefix(const std::string& prefix) noexcept
{
    (void)prefix;
}

void sfz::Sfizz::disableLogging() noexcept
{
}

sfz::Sfizz::CallbackBreakdown sfz::Sfizz::getCallbackBreakdown() noexcept
{
    CallbackBreakdown breakdown;
    const auto& bd = synth->synth.getCallbackBreakdown();
    breakdown.amplitude = bd.amplitude;
    breakdown.panning = bd.panning;
    breakdown.renderMethod = bd.renderMethod;
    breakdown.data = bd.data;
    breakdown.dispatch = bd.dispatch;
    breakdown.filters = bd.filters;
    breakdown.effects = bd.effects;
    return breakdown;
}

void sfz::Sfizz::allSoundOff() noexcept
{
    synth->synth.allSoundOff();
}

void sfz::Sfizz::addExternalDefinition(const std::string& id, const std::string& value)
{
    synth->synth.addExternalDefinition(id, value);
}

void sfz::Sfizz::clearExternalDefinitions()
{
    synth->synth.clearExternalDefinitions();
}

std::string sfz::Sfizz::exportMidnam(const std::string& model) const
{
    return synth->synth.exportMidnam(model);
}

const std::vector<std::pair<uint8_t, std::string>>& sfz::Sfizz::getKeyLabels() const noexcept
{
    return synth->synth.getKeyLabels();
}

const std::vector<std::pair<uint16_t, std::string>>& sfz::Sfizz::getCCLabels() const noexcept
{
    return synth->synth.getCCLabels();
}

void sfz::Sfizz::ClientDeleter::operator()(Client *client) const noexcept
{
    delete client;
}

auto sfz::Sfizz::createClient(void* data) -> ClientPtr
{
    return ClientPtr(new Client(data));
}

void* sfz::Sfizz::getClientData(Client& client)
{
    return client.getClientData();
}

void sfz::Sfizz::setReceiveCallback(Client& client, sfizz_receive_t* receive)
{
    client.setReceiveCallback(receive);
}

void sfz::Sfizz::sendMessage(Client& client, int delay, const char* path, const char* sig, const sfizz_arg_t* args)
{
    synth->synth.dispatchMessage(client, delay, path, sig, args);
}

void sfz::Sfizz::setBroadcastCallback(sfizz_receive_t* broadcast, void* data)
{
    synth->synth.setBroadcastCallback(broadcast, data);
}
