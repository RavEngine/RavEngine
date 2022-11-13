#include "AudioMIDI.hpp"
#include <sfizz.hpp>
#include "Common3D.hpp"
#include "AudioPlayer.hpp"
#include "AudioRoom.hpp"
#include <sfizz/MathHelpers.h>
#include <sfizz/SfzHelpers.h>
#include <sfizz/SIMDHelpers.h>
#include <sfizz/../../external/st_audiofile/thirdparty/dr_libs/dr_wav.h>

#include <cstdint>

namespace midi {
    constexpr uint8_t statusMask { 0b11110000 };
    constexpr uint8_t channelMask { 0b00001111 };
    constexpr uint8_t noteOff { 0x80 };
    constexpr uint8_t noteOn { 0x90 };
    constexpr uint8_t polyphonicPressure { 0xA0 };
    constexpr uint8_t controlChange { 0xB0 };
    constexpr uint8_t programChange { 0xC0 };
    constexpr uint8_t channelPressure { 0xD0 };
    constexpr uint8_t pitchBend { 0xE0 };
    constexpr uint8_t systemMessage { 0xF0 };

    constexpr uint8_t status(uint8_t midiStatusByte)
    {
        return midiStatusByte & statusMask;
    }
    constexpr uint8_t channel(uint8_t midiStatusByte)
    {
        return midiStatusByte & channelMask;
    }

    constexpr int buildAndCenterPitch(uint8_t firstByte, uint8_t secondByte)
    {
        return (int)(((unsigned int)secondByte << 7) + (unsigned int)firstByte) - 8192;
    }
}


using namespace RavEngine;

void AudioMIDIPlayer::EnqueueEvent(const MidiEvent &evt, uint16_t track){
    instrumentTrackMap.at(track).events.push(evt);
}

void AudioMIDIPlayer::Render(buffer_t out_buffer){
    // pop events off and give to the proper instrument
    const auto samplesPerSec = AudioPlayer::GetSamplesPerSec();
    
    float* scratchbuffer[]{new float[out_buffer.size()], new float[out_buffer.size()]};

    for (auto& instrument : instrumentTrackMap){
        // set the instrument's callback function
        instrument.instrument->synthesizer.setSampleRate(samplesPerSec);
        instrument.instrument->synthesizer.enableFreeWheeling();    //TODO: restore old state after processing is finished
        
        while (!instrument.events.empty()){
            // get the next event (without popping it)
            auto& nextEvent = instrument.events.top();
            
            // convert the start tick time into a buffer index
            // I had help with these equations
            auto seconds = (nextEvent.tick) * (1.0/ticksPerQuarterNote) * (1.0/beatsPerMinute) * 60;
            auto bufferidx = size_t(seconds * AudioPlayer::GetSamplesPerSec()) - playhead;
            
            // is this event's start point within the buffer?
            if (bufferidx < out_buffer.size()){
                // if so, pop it and provide it to the Instrument
                if (nextEvent.isNoteOn()){
                    instrument.instrument->synthesizer.noteOn(bufferidx, nextEvent.getKeyNumber(), nextEvent.getVelocity());
                }
                else if (nextEvent.isNoteOff()){
                    instrument.instrument->synthesizer.noteOff(bufferidx, nextEvent.getKeyNumber(), nextEvent.getVelocity());
                }
                
                // consume the event
                instrument.events.pop();
            }
            else{
                // otherwise, stop the loop
                break;
            }
        }
         instrument.instrument->synthesizer.renderBlock(scratchbuffer, out_buffer.size(),1);
    }
    // advance playhead, now that this buffer processing has completed
    playhead += out_buffer.size();
    delete[] scratchbuffer[0];
    delete[] scratchbuffer[1];
}

void AudioMIDIPlayer::SetInstrumentForTrack(uint16_t channel, std::unique_ptr<InstrumentSynth>& instrument){
    if (instrumentTrackMap.size() <= channel){
        instrumentTrackMap.resize(closest_multiple_of(channel+1,2));
    }
    instrumentTrackMap[channel].instrument = std::move(instrument);
}

Ref<AudioAsset> AudioMIDIRenderer::Render(MidiFile& file, AudioMIDIPlayer& player){
        
    // load up all the events
    for (int i = 0; i < file.getNumTracks(); i++){
        const auto& track = file[i];
        for(int t = 0; t < track.getEventCount(); t++){
            auto& event = track.getEvent(t);
            player.EnqueueEvent(event, i);
        }
    }
    player.ticksPerQuarterNote = file.getTicksPerQuarterNote();
    
    auto totalSecs = file.getFileDurationInSeconds();
    
    const size_t buffsize = AudioPlayer::GetSamplesPerSec() * totalSecs;
    
    // create the AudioAsset
    float* buf = new float[buffsize]{0};   // this will be freed by the AudioAsset
    auto asset = RavEngine::New<AudioAsset>(buf, buffsize, 1);
    
    // render into it
    player.Render(AudioMIDIPlayer::buffer_t(buf,1024));
    
    return asset;
}

struct CallbackData {
    sfz::Sfizz& synth;
    unsigned delay = 0;
    bool finished = false;
};

void midiTickCallback(const fmidi_event_t * event, void * cbdata, fmidi_seq_event_t* fulldata)
{
    auto data = reinterpret_cast<CallbackData*>(cbdata);

    if (event->type != fmidi_event_type::fmidi_event_message)
        return;

    switch (midi::status(event->data[0])) {
        case midi::noteOff:
            data->synth.noteOff(data->delay, event->data[1], event->data[2]);
            break;
        case midi::noteOn:
            if (event->data[2] == 0)
                data->synth.noteOff(data->delay, event->data[1], event->data[2]);
            else
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


Ref<AudioAsset> AudioMIDIRenderer::Render(const Filesystem::Path& path, AudioMIDIPlayer& player){
    unsigned blockSize { 1024 };
    unsigned sampleRate { AudioPlayer::GetSamplesPerSec() };
    int quality { 2 };
    int polyphony { 512 };
    
    fmidi_smf_u midiFile { fmidi_smf_file_read(path.string().c_str()) };
    
//    drwav outputFile;
//    drwav_data_format outputFormat {};
//    outputFormat.container = drwav_container_riff;
//    outputFormat.format = DR_WAVE_FORMAT_PCM;
//    outputFormat.channels = 2;
//    outputFormat.sampleRate = sampleRate;
//    outputFormat.bitsPerSample = 16;
//
//    Filesystem::Path outputPath{"/Users/Admin/output.wav"};
//
//#if !defined(_WIN32)
//    drwav_bool32 outputFileOk = drwav_init_file_write(&outputFile, outputPath.c_str(), &outputFormat, nullptr);
//#else
//    drwav_bool32 outputFileOk = drwav_init_file_write_w(&outputFile, outputPath.c_str(), &outputFormat, nullptr);
//#endif
    
    auto sampleRateDouble = static_cast<double>(sampleRate);
    const double increment { 1.0 / sampleRateDouble };
    uint64_t numFramesWritten { 0 };
    float* audioBuffer[]{new float[blockSize], new float[blockSize]};
//    float* interleavedBuffer = new float[blockSize * 2];
//    int16* interleavedPcm = new int16[blockSize * 2];
    
    sfz::Sfizz synth;
    synth.setSamplesPerBlock(blockSize);
    synth.setSampleRate(sampleRate);
    synth.setSampleQuality(sfz::Sfizz::ProcessMode::ProcessFreewheeling, quality);
    synth.setNumVoices(polyphony);
    synth.setVolume(5);
    synth.loadSfzFile("/Users/admin/Downloads/VSCO-2-CE-1.1.0/Harp.sfz");
    synth.enableFreeWheeling();
    
    fmidi_player_u midiPlayer { fmidi_player_new(midiFile.get()) };
    CallbackData callbackData { synth };
    fmidi_player_event_callback(midiPlayer.get(), &midiTickCallback, &callbackData);
    fmidi_player_finish_callback(midiPlayer.get(), &finishedCallback, &callbackData);
    
    const auto duration = fmidi_smf_compute_duration(midiFile.get());
    
    const size_t totalSamples = duration * AudioPlayer::GetSamplesPerSec();
    auto assetData = new float[totalSamples]{0};
    
    fmidi_player_start(midiPlayer.get());
    
    while (!callbackData.finished) {
        for (callbackData.delay = 0; callbackData.delay < blockSize && !callbackData.finished; callbackData.delay++)
            fmidi_player_tick(midiPlayer.get(), increment);
        synth.renderBlock(audioBuffer, blockSize);
        
        auto nBytesToWrite = std::min<size_t>(blockSize * sizeof(audioBuffer[0]), totalSamples - numFramesWritten);
        std::memcpy(assetData + numFramesWritten, audioBuffer[0],nBytesToWrite);
        
//        for(int i = 0; i < blockSize * 2; i+=2){
//            interleavedBuffer[i] = audioBuffer[0][i/2];
//            interleavedBuffer[i+1] = audioBuffer[1][i/2];
//        }
//        drwav_f32_to_s16(interleavedPcm, interleavedBuffer, 2 * blockSize);
//        drwav_write_pcm_frames(&outputFile, blockSize, interleavedPcm);
        numFramesWritten += nBytesToWrite / sizeof(audioBuffer[0]);//
    }
    
//    drwav_uninit(&outputFile);
    
    delete[] audioBuffer[0];
    delete[] audioBuffer[1];
    //delete[] interleavedBuffer;
    //delete[] interleavedPcm;
    
    auto asset = New<AudioAsset>(assetData,totalSamples,1);
    
    return asset;
}

InstrumentSynth::InstrumentSynth(const Filesystem::Path& pathOnDisk) {
    synthesizer.loadSfzFile(pathOnDisk.string());
    synthesizer.setSampleRate(AudioPlayer::GetSamplesPerSec());
}
