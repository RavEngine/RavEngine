#pragma once
#include "DataStructures.hpp"
#include "AudioSource.hpp"
#include "AudioRoom.hpp"

namespace RavEngine{
struct AudioSnapshot{
    struct PointSource{
        Ref<AudioPlayerData::Player> data;
        vector3 worldpos;
        quaternion worldrot;
        PointSource(const decltype(data)& data, const decltype(worldpos)& wp, const decltype(worldrot)& wr): data(data), worldpos(wp), worldrot(wr){}
    };
    Vector<PointSource> sources;
    Vector<Ref<AudioPlayerData::Player>> ambientSources;
    
    struct Room{
        Ref<AudioRoom::RoomData> room;
        vector3 worldpos;
        quaternion worldrot;
        Room(const decltype(room)& room,const decltype(worldpos)& wp, const decltype(worldrot)& wr): room(room), worldpos(wp), worldrot(wr){}
    };
    
    Vector<Room> rooms;
    vector3 listenerPos;
    quaternion listenerRot;
    
    void Clear(){
        sources.clear();
        ambientSources.clear();
        rooms.clear();
    }
};
}

