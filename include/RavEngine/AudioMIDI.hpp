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
public:
    InstrumentSynth(const Filesystem::Path& path);
};

class AudioMIDIPlayer{
    // data structure for events
    struct MIDIComparator{
        constexpr bool operator() (const MidiEvent& a, const MidiEvent& b){
            return b.tick < a.tick;
        };
    };
    
    struct InstrumentChannelPair{
        std::unique_ptr<InstrumentSynth> instrument;
        std::priority_queue<MidiEvent, std::vector<MidiEvent>, MIDIComparator> events;
    };
    std::vector<InstrumentChannelPair> instrumentTrackMap;
    
    uint64_t playhead = 0;  // position in total samples emitted
    
public:
    void EnqueueEvent(const MidiEvent& evt, uint16_t track);
    void SetInstrumentForTrack(uint16_t channel, std::unique_ptr<InstrumentSynth>& instrument);
    
    using buffer_t = std::span<float,std::dynamic_extent>;
    void Render(buffer_t out_buffer);
    
    void ResetPlayhead(){
        playhead = 0;
    }
    
    int ticksPerQuarterNote = 0;
    float beatsPerMinute = 60;

};
    
struct AudioMIDIRenderer{
    Ref<AudioAsset> Render(MidiFile& file, AudioMIDIPlayer& player);
    Ref<AudioAsset> Render(const Filesystem::Path& path, AudioMIDIPlayer& player);
};

}
