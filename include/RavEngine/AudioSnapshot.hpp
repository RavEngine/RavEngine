#pragma once
#include "DataStructures.hpp"
#include "AudioSource.hpp"
#include "AudioRoom.hpp"

namespace RavEngine{
struct AudioSnapshot{
    struct PointSource{
        AudioPlayerData::Player data;
        vector3 worldpos;
        quaternion worldrot;
        PointSource(const decltype(data)& data, const decltype(worldpos)& wp, const decltype(worldrot)& wr): data(data), worldpos(wp), worldrot(wr){}
    };
    Vector<PointSource> sources;
    Vector<AudioPlayerData::Player> ambientSources;
    
    void Clear(){
        sources.clear();
        ambientSources.clear();
    }
};
}

