#pragma once
#include <sfizz.hpp>
#include <MidiFile.h>
#include "AudioSource.hpp"
#include "Manager.hpp"
#include "Filesystem.hpp"
#include <fmidi/fmidi.h>
#include "AudioGraphAsset.hpp"
#include "SpinLock.hpp"

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
class AudioMIDIPlayer : public AudioGraphComposed, public AudioDataProvider{
    // data structure for events
//    struct MIDIComparator{
//        constexpr bool operator() (const MidiEvent& a, const MidiEvent& b){
//            return b.tick < a.tick;
//        };
//    };
    SpinLock mtx;
    friend class AudioPlayer;
    struct InstrumentChannelPair{
        std::shared_ptr<InstrumentSynth> instrument;
        //std::priority_queue<MidiEvent, std::vector<MidiEvent>, MIDIComparator> events;
    };
    std::vector<InstrumentChannelPair> instrumentTrackMap;
    
    
    fmidi_player_u midiPlayer;
    Ref<fmidi_smf_t> midiSMF;
    
    uint32_t delay = 0;  // tells the synth in the callback function when to start playing the sound
        
public:
    AudioMIDIPlayer();

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
    void RenderMonoBuffer1024OrLess(PlanarSampleBufferInlineView& out_buffer, PlanarSampleBufferInlineView& effectScratchBuffer);
    
    /**
     Render the state of the player to the provided buffer
     @param out_buffer the buffer to write to
     */
    void ProvideBufferData(PlanarSampleBufferInlineView& out_buffer, PlanarSampleBufferInlineView& effectScratchBuffer) final;
    
    auto GetVolume() const{
        return volume;
    }
    
    void Restart() final;
    
    int ticksPerQuarterNote = 0;
    float beatsPerMinute = 60;

};

/**
 For rendering a MIDI song to an AudioAsset
 */
struct AudioMIDIRenderer{
    Ref<AudioAsset> Render(const Ref<fmidi_smf_t>& file, AudioMIDIPlayer& player);
};

}
