#pragma once
#include <sfizz/Synth.h>
#include <MidiFile.h>
#include "AudioSource.hpp"
#include "Manager.hpp"


namespace RavEngine{
class AudioAsset;

using MidiFile = smf::MidiFile;

struct InstrumentSynth{
    struct Manager : public GenericWeakReadThroughCache<std::string,InstrumentSynth>{};
};
    
struct AudioMIDIRenderer{
    
    Ref<AudioAsset> Render(const MidiFile& file);
};

}
