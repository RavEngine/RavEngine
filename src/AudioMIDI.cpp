#include "AudioMIDI.hpp"
#include <sfizz.hpp>
#include "Common3D.hpp"
#include "AudioPlayer.hpp"
#include "AudioRoom.hpp"

using namespace RavEngine;

void AudioMIDIPlayer::EnqueueEvent(const MidiEvent &evt, uint16_t track){
    instrumentTrackMap.at(track).events.push(evt);
}

void AudioMIDIPlayer::Render(buffer_t out_buffer){
    // pop events off and give to the proper instrument
    const auto samplesPerSec = AudioPlayer::GetSamplesPerSec();

    for (auto& instrument : instrumentTrackMap){
        // set the instrument's callback function
        instrument.instrument->synthesizer.setBroadcastCallback(InstrumentSynth::CallbackStatic,instrument.instrument.get());
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
        //TODO: use the scratch buffer instead
        float* beginbuf = out_buffer.data();
        instrument.instrument->synthesizer.renderBlock(&beginbuf, out_buffer.size(),1);
    }
    // advance playhead, now that this buffer processing has completed
    playhead += out_buffer.size();
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
    player.Render(AudioMIDIPlayer::buffer_t(buf,buffsize));
    
    return asset;
}

InstrumentSynth::InstrumentSynth(const Filesystem::Path& pathOnDisk) {
    synthesizer.loadSfzFile(pathOnDisk.string());
    synthesizer.setSampleRate(AudioPlayer::GetSamplesPerSec());
}

void InstrumentSynth::Callback(int delay, const char *path, const char *sig, const sfizz_arg_t *args){
    // do we even need this?
}
