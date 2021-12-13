#include "AudioRoom.hpp"
#include "AudioRoomSyncSystem.hpp"
#include "mathtypes.hpp"
#include <common/room_properties.h>
#include <common/room_effects_utils.h>
#include "Entity.hpp"
#include "Transform.hpp"

using namespace RavEngine;
using namespace std;

void AudioRoomSyncSystem::operator()(float fpsScale, AudioRoom& room, Transform& tr) const{
    auto pos = tr.GetWorldPosition();
    auto rot = tr.GetWorldRotation();
    auto mtx = tr.CalculateWorldMatrix();
    
    quaternion r;
    vector3 t;
    vector3 scale;
    vector4 p;
    glm::decompose(mtx, scale, r, t, t, p);
    
    vraudio::RoomProperties data;
            
    data.position[0] = pos.x;
    data.position[1] = pos.y;
    data.position[2] = pos.z;
    
    data.rotation[0] = rot.x;
    data.rotation[1] = rot.y;
    data.rotation[2] = rot.z;
    data.rotation[3] = rot.w;
    
    //attempt to scale (does not factor in shear)
    auto dim = room.data->roomDimensions * scale;
    
    data.dimensions[0] = dim.x;
    data.dimensions[1] = dim.y;
    data.dimensions[2] = dim.z;
    
    data.reflection_scalar = room.data->reflection_scalar;
    data.reverb_gain = room.data->reverb_gain;
    data.reverb_time = room.data->reverb_time;
    data.reverb_brightness = room.data->reverb_brightness;
    
    //add material data
    std::memcpy(data.material_names, room.data->wallMaterials.data(), sizeof(data.material_names));
    
    //compute the reflection data
    auto ref_data = vraudio::ComputeReflectionProperties(data);
    auto rev_data = vraudio::ComputeReverbProperties(data);
            
    //write changes
    room.data->audioEngine->SetReflectionProperties(ref_data);
    room.data->audioEngine->SetReverbProperties(rev_data);
}
