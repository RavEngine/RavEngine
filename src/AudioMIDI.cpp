#include "AudioMIDI.hpp"
#include <sfizz.hpp>
#include "Common3D.hpp"
#include <iostream>

using namespace RavEngine;

void AudioMIDIPlayer::EnqueueEvent(const MidiEvent &evt, uint16_t track){
    instrumentTrackMap.at(track).events.push(evt);
}

void AudioMIDIPlayer::Render(buffer_t out_buffer){
    // pop events off and give to the proper instrument
}

void AudioMIDIPlayer::SetInstrumentForTrack(uint16_t channel, std::unique_ptr<InstrumentSynth>& instrument){
    if (instrumentTrackMap.size() <= channel){
        instrumentTrackMap.resize(closest_multiple_of(channel+1,2));
    }
    instrumentTrackMap[channel].instrument = std::move(instrument);
}

Ref<AudioAsset> AudioMIDIRenderer::Render(const MidiFile& file, AudioMIDIPlayer& player){
        
    // load up all the events
    for (int i = 0; i < file.getNumTracks(); i++){
        const auto& track = file[i];
        for(int t = 0; t < track.getEventCount(); t++){
            auto& event = track.getEvent(t);
            player.EnqueueEvent(event, i);
        }
    }
    
    size_t buffsize = 100;
    
    // create the AudioAsset
    float* buf = new float[buffsize]{0};   // this will be freed by the AudioAsset
    auto asset = RavEngine::New<AudioAsset>(buf, buffsize, 1);
    
    // render into it
    player.Render(AudioMIDIPlayer::buffer_t(buf,buffsize));
    
    return asset;
}

InstrumentSynth::InstrumentSynth(const Filesystem::Path& pathOnDisk) {
    synthesizer.loadSfzFile(pathOnDisk);
}
