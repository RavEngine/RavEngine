#pragma once
#include "DataStructures.hpp"
#include "AudioSource.hpp"
#include "AudioRoom.hpp"
#include "AudioMIDI.hpp"

namespace RavEngine{
struct AudioSnapshot{
    struct PointSourceBase{
        vector3 worldpos;
        quaternion worldrot;
    };
    
    struct PointSource : public PointSourceBase{
        Ref<AudioPlayerData::Player> data;
        PointSource(const decltype(data)& data, const decltype(worldpos)& wp, const decltype(worldrot)& wr): data(data), PointSourceBase{wp, wr} {}
    };
    
    struct PointMIDISource : PointSourceBase{
        AudioMIDISourceComponent source;    // contains only a
        static_assert(sizeof(AudioMIDISourceComponent) == sizeof(Ref<AudioMIDIPlayer>), "MIDISource is larger than a smart pointer, consider refactoring");
        PointMIDISource(const decltype(source)& source, const decltype(worldpos)& wp, const decltype(worldrot)& wr): source(source), PointSourceBase{wp, wr} {}
        
        auto hashcode() const{
            return std::hash<decltype(source.midiPlayer)>()(source.midiPlayer);
        }
    };
    
    struct Room{
        Ref<AudioRoom::RoomData> room;
        vector3 worldpos;
        quaternion worldrot;
        Room(const decltype(room)& room,const decltype(worldpos)& wp, const decltype(worldrot)& wr): room(room), worldpos(wp), worldrot(wr){}
    };
    
    Vector<PointSource> sources;
    Vector<Ref<AudioPlayerData::Player>> ambientSources;
    Vector<PointMIDISource> midiPointSources;
    Vector<AudioMIDIAmbientSourceComponent> ambientMIDIsources;
    
    static_assert(sizeof(AudioMIDIAmbientSourceComponent) == sizeof(Ref<AudioMIDIPlayer>), "AudioMIDIAmbientSourceComponent is larger than a smart pointer, consider refactoring");

    
    Vector<Room> rooms;
    vector3 listenerPos;
    quaternion listenerRot;
    
    void Clear(){
        sources.clear();
        ambientSources.clear();
        rooms.clear();
        midiPointSources.clear();
        ambientMIDIsources.clear();
    }
};
}

