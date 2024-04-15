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
        entity_t ownerID = INVALID_ENTITY;
        PointSource(const decltype(data)& data, const decltype(worldpos)& wp, const decltype(worldrot)& wr, decltype(ownerID) ownerID): data(data), ownerID(ownerID), PointSourceBase{wp, wr} {}
        bool operator==(const PointSource& other) const{
            return data == other.data && worldpos == other.worldpos && worldrot == other.worldrot;
        }
    };
    
    struct Room{
        Ref<SimpleAudioSpace::RoomData> room;
        vector3 worldpos;
        quaternion worldrot;
        Room(const decltype(room)& room,const decltype(worldpos)& wp, const decltype(worldrot)& wr): room(room), worldpos(wp), worldrot(wr){}

        bool IsInsideRoom(const vector3& pos) const{
            return glm::distance2(pos, worldpos) < (room->radius * room->radius);
        }
    };
    
    UnorderedSet<PointSource> sources;
    UnorderedSet<Ref<AudioDataProvider>> ambientSources;
    
    Vector<Room> rooms;
    vector3 listenerPos;
    quaternion listenerRot;
    Ref<AudioGraphAsset> listenerGraph;
    WeakRef<World> sourceWorld;
    
    void Clear(){
        sources.clear();
        ambientSources.clear();
        rooms.clear();
        sourceWorld.reset();
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
