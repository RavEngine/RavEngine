#pragma once
#include <sfizz.hpp>
#include <MidiFile.h>
#include "AudioSource.hpp"
#include "Manager.hpp"
#include "Filesystem.hpp"
#include <fmidi/fmidi.h>
#include "AudioGraphAsset.hpp"

namespace RavEngine{

using MidiFile = smf::MidiFile;
using MidiEvent = smf::MidiEvent;

class InstrumentSynth : public AudioGraphComposed{
    sfz::Sfizz synthesizer;
    friend class AudioMIDIPlayer;
    bool freeWheel = false;
public:
    InstrumentSynth(const Filesystem::Path& path, bool notStreaming = false);
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
    auto isFreewheeling(){
        return freeWheel;
    }
    void setSamplesPerBlock(uint32_t samples){
        synthesizer.setSamplesPerBlock(samples);
    }
    
    void setSampleQuality(sfz::Sfizz::ProcessMode mode, int quality){
        synthesizer.setSampleQuality(mode, quality);
    }
    
    void Render(float** scratchBuffer, size_t size, PlanarSampleBufferInlineView output, uint8_t nchannels);
};

/**
 Must be allocated to a stable location in memory
 */
class AudioMIDIPlayer : public AudioGraphComposed{
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
    Ref<fmidi_smf_t> midiSMF;
    
    uint32_t delay = 0;  // tells the synth in the callback function when to start playing the sound
    
    float volume = 1;
    
    bool isPlaying = false;
public:
    // internal use only
    bool finishedCurrent = true;

    //void EnqueueEvent(const MidiEvent& evt, uint16_t track);
    void SetInstrumentForTrack(uint16_t track, std::shared_ptr<InstrumentSynth>& instrument);
    
    void SetMidi(const decltype(midiSMF)&);
    
    // internal use only
    void processEvent(const fmidi_event_t * event, fmidi_seq_event_t* fulldata);
    
    /**
     For internal use only. Use Render()
     */
    void RenderMonoBuffer1024OrLess(PlanarSampleBufferInlineView out_buffer);
    
    /**
     Render the state of the player to the provided buffer
     @param out_buffer the buffer to write to
     */
    void RenderMono(PlanarSampleBufferInlineView out_buffer);
    
    auto GetVolume() const{
        return volume;
    }
    
    void SetVolume(decltype(volume) inVol){
        volume = inVol;
    }
    
    void Play(){
        isPlaying = true;
    }
    void Pause(){
        isPlaying = false;
    }
    
    bool IsPlaying() const{
        return isPlaying;
    }
    
    void Reset();
    
    int ticksPerQuarterNote = 0;
    float beatsPerMinute = 60;

};

struct AudioMIDISourceBase : public AutoCTTI{
    Ref<AudioMIDIPlayer> midiPlayer;
};

/**
 For playing MIDI from a point in 3D space
 */
struct AudioMIDISourceComponent : public AudioMIDISourceBase, public Queryable<AudioMIDISourceComponent>{};

/**
 For playing MIDI from anywhere, intended for background audio
 */
struct AudioMIDIAmbientSourceComponent : public AudioMIDISourceBase, public Queryable<AudioMIDIAmbientSourceComponent>{};


/**
 For rendering a MIDI song to an AudioAsset
 */
struct AudioMIDIRenderer{
    Ref<AudioAsset> Render(const Ref<fmidi_smf_t>& file, AudioMIDIPlayer& player);
};

}
