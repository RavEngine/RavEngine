#pragma once
#include "DataStructures.hpp"
#include "AudioSource.hpp"
#include "AudioRoom.hpp"

namespace RavEngine{
struct AudioGraphAsset;

struct AudioSnapshot{
    struct PointSourceBase{
        vector3 worldpos;
        quaternion worldrot;
    };
    
    struct PointSource : public PointSourceBase{
        Ref<AudioDataProvider> data;
        PointSource(const decltype(data)& data, const decltype(worldpos)& wp, const decltype(worldrot)& wr): data(data), PointSourceBase{wp, wr} {}
        bool operator==(const PointSource& other) const{
            return data == other.data && worldpos == other.worldpos && worldrot == other.worldrot;
        }
    };
    
    struct Room{
        Ref<SimpleAudioSpace::RoomData> room;
        vector3 worldpos;
        quaternion worldrot;
        Room(const decltype(room)& room,const decltype(worldpos)& wp, const decltype(worldrot)& wr): room(room), worldpos(wp), worldrot(wr){}
    };
    
    UnorderedSet<PointSource> sources;
    UnorderedSet<Ref<AudioDataProvider>> ambientSources;
    
    Vector<Room> rooms;
    vector3 listenerPos;
    quaternion listenerRot;
    Ref<AudioGraphAsset> listenerGraph;
    
    void Clear(){
        sources.clear();
        ambientSources.clear();
        rooms.clear();
    }
};
}

namespace std{
    template<>
    struct hash<RavEngine::AudioSnapshot::PointSource>{
        inline size_t operator()(const RavEngine::AudioSnapshot::PointSource& obj){
            return reinterpret_cast<size_t>(obj.data.get());
        }
    };
}
