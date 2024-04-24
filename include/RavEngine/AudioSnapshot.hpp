#pragma once
#include "DataStructures.hpp"
#include "AudioSource.hpp"
#include "AudioSpace.hpp"

namespace RavEngine{
struct AudioGraphAsset;
struct AudioMeshAsset;

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
    
    template<typename room_t>
    struct TAudioSpace{
        Ref<room_t> room;
        vector3 worldpos;
        TAudioSpace(const decltype(room)& room,const decltype(worldpos)& wp): room(room), worldpos(wp){}

        bool IsInsideSourceArea(const vector3& pos) const{
            return glm::distance2(pos, worldpos) < (room->sourceRadius * room->sourceRadius);
        }
    };

    using SimpleAudioSpaceData = TAudioSpace<SimpleAudioSpace::RoomData>;

    struct GeometryAudioSpaceData : public TAudioSpace<GeometryAudioSpace::RoomData> {

        GeometryAudioSpaceData(const decltype(room)& room, const decltype(worldpos)& wp) : TAudioSpace(room, wp) {}

        bool IsInsideMeshArea(const vector3& pos) const{
            return glm::distance2(pos, worldpos) < (room->meshRadius * room->meshRadius);
        }
    };

    struct AudioMeshData {
        matrix4 worldTransform;
        Ref<AudioMeshAsset> asset;
        AudioMeshData(const decltype(worldTransform)& wt, const decltype(asset)& a) : worldTransform(wt), asset(a) {}
    };
    
    UnorderedVector<PointSource> sources;
    UnorderedSet<Ref<AudioDataProvider>> dataProviders;
    UnorderedVector<Ref<AudioDataProvider>> ambientSources;
    
    Vector<SimpleAudioSpaceData> simpleAudioSpaces;
    Vector<GeometryAudioSpaceData> geometryAudioSpaces;
    Vector<AudioMeshData> audioMeshes;

    vector3 listenerPos;
    quaternion listenerRot;
    Ref<AudioGraphAsset> listenerGraph;
    WeakRef<World> sourceWorld;
    
    void Clear(){
        sources.clear();
        ambientSources.clear();
        simpleAudioSpaces.clear();
        geometryAudioSpaces.clear();
        sourceWorld.reset();
        audioMeshes.clear();
        dataProviders.clear();
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
