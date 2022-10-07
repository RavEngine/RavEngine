#pragma once
#include <sfizz/Synth.h>
#include "AudioSource.hpp"
#include "Manager.hpp"


namespace RavEngine{
class AudioAsset;

struct InstrumentSynth{
    struct Manager : public GenericWeakReadThroughCache<std::string,InstrumentSynth>{};
};
    
struct AudioMIDIRenderer{
    
    Ref<AudioAsset> Render();
};

}
