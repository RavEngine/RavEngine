#pragma once
#include <sfizz.hpp>
#include <MidiFile.h>
#include "AudioSource.hpp"
#include "Manager.hpp"
#include <span>
#include "Filesystem.hpp"

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
    void Callback(int delay, const char* path, const char* sig, const sfizz_arg_t* args);
public:
    InstrumentSynth(const Filesystem::Path& path);
    static void CallbackStatic(void* data, int delay, const char* path, const char* sig, const sfizz_arg_t* args){
        static_cast<InstrumentSynth*>(data)->Callback(delay,path,sig,args);
    }
};

class AudioMIDIPlayer{
    // data structure for events
    struct MIDIComparator{
        constexpr bool operator() (const MidiEvent& a, const MidiEvent& b){
            return a.tick < b.tick;
        };
    };
    
    struct InstrumentChannelPair{
        std::unique_ptr<InstrumentSynth> instrument;
        std::priority_queue<MidiEvent, std::vector<MidiEvent>, MIDIComparator> events;
    };
    std::vector<InstrumentChannelPair> instrumentTrackMap;
    
    double currentTime;
    
public:
    void EnqueueEvent(const MidiEvent& evt, uint16_t track);
    void SetInstrumentForTrack(uint16_t channel, std::unique_ptr<InstrumentSynth>& instrument);
    
    using buffer_t = std::span<float,std::dynamic_extent>;
    void Render(buffer_t out_buffer);
};
    
struct AudioMIDIRenderer{
    Ref<AudioAsset> Render(MidiFile& file, AudioMIDIPlayer& player);
};

}
