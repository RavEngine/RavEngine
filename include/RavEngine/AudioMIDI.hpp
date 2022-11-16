#pragma once
#include <sfizz.hpp>
#include <MidiFile.h>
#include "AudioSource.hpp"
#include "Manager.hpp"
#include <span>
#include "Filesystem.hpp"
#include <fmidi/fmidi.h>

namespace RavEngine{
class AudioAsset;

using MidiFile = smf::MidiFile;
using MidiEvent = smf::MidiEvent;

struct SoundFont{
    struct Manager : public GenericWeakReadThroughCache<std::string,SoundFont>{};
};

class InstrumentSynth{
    sfz::Sfizz synthesizer;
    friend class AudioMIDIPlayer;
    bool freeWheel = false;
public:
    InstrumentSynth(const Filesystem::Path& path);
    void setNumVoices(uint32_t poly){
        synthesizer.setNumVoices(poly);
    }
    void setVolume(int32_t vol){
        synthesizer.setVolume(vol);
    }
    
    void enableFreewheeling(){
        synthesizer.enableFreeWheeling();
        freeWheel = true;
    }
    void disableFreewheeling(){
        synthesizer.disableFreeWheeling();
        freeWheel = false;
    }
    void isFreewheeling(){
        return freeWheel;
    }
    void setSamplesPerBlock(uint32_t samples){
        synthesizer.setSamplesPerBlock(samples);
    }
};

/**
 Must be allocated to a stable location in memory
 */
class AudioMIDIPlayer{
    // data structure for events
//    struct MIDIComparator{
//        constexpr bool operator() (const MidiEvent& a, const MidiEvent& b){
//            return b.tick < a.tick;
//        };
//    };
    
    struct InstrumentChannelPair{
        std::shared_ptr<InstrumentSynth> instrument;
        //std::priority_queue<MidiEvent, std::vector<MidiEvent>, MIDIComparator> events;
    };
    std::vector<InstrumentChannelPair> instrumentTrackMap;
    
    fmidi_player_u midiPlayer;
    
    //uint64_t playhead = 0;  // position in total samples emitted
    uint32_t delay = 0;  // tells the synth in the callback function when to start playing the sound
public:
    // internal use only
    bool finishedCurrent = true;

    //void EnqueueEvent(const MidiEvent& evt, uint16_t track);
    void SetInstrumentForTrack(uint16_t track, std::shared_ptr<InstrumentSynth>& instrument);
    
    void SetMidi(const fmidi_smf_u&);
    
    // internal use only
    void processEvent(const fmidi_event_t * event, fmidi_seq_event_t* fulldata);
    
    using buffer_t = std::span<float,std::dynamic_extent>;
    void Render(buffer_t out_buffer);
    
//    void ResetPlayhead(){
//        playhead = 0;
//    }
    
    int ticksPerQuarterNote = 0;
    float beatsPerMinute = 60;

};
    
struct AudioMIDIRenderer{
    Ref<AudioAsset> Render(const fmidi_smf_u& file, AudioMIDIPlayer& player);
    Ref<AudioAsset> Render(const Filesystem::Path& path, AudioMIDIPlayer& player);
};

}
