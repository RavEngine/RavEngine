#include "AudioRoom.hpp"
#include "AudioRoomSyncSystem.hpp"
#include "mathtypes.hpp"
#include <common/room_properties.h>
#include <common/room_effects_utils.h>

using namespace RavEngine;
using namespace std;

const System::list_type AudioRoomSyncSystem::queries{CTTI<AudioRoom>};

void AudioRoomSyncSystem::Tick(float fpsScale, Ref<Entity> e){
	auto rooms = e->GetAllComponentsOfTypeFastPath<AudioRoom>();
	
	auto pos = e->transform()->GetWorldPosition();
	auto rot = e->transform()->GetWorldRotation();
	auto mtx = e->transform()->CalculateWorldMatrix();
	
	quaternion r;
	vector3 t;
	vector3 scale;
	vector4 p;
	glm::decompose(mtx, scale, r, t, t, p);
	
	for(const auto& r : rooms){
		auto room = static_pointer_cast<AudioRoom>(r);
		
		vraudio::RoomProperties data;
				
		data.position[0] = pos.x;
		data.position[1] = pos.y;
		data.position[2] = pos.z;
		
		data.rotation[0] = rot.x;
		data.rotation[1] = rot.y;
		data.rotation[2] = rot.z;
		data.rotation[3] = rot.w;
		
		//attempt to scale (does not factor in shear)
		auto dim = room->roomDimensions * scale;
		
		data.dimensions[0] = dim.x;
		data.dimensions[1] = dim.y;
		data.dimensions[2] = dim.z;
		
		data.reflection_scalar = room->reflection_scalar;
		data.reverb_gain = room->reverb_gain;
		data.reverb_time = room->reverb_time;
		data.reverb_brightness = room->reverb_brightness;
		
		//add material data
		std::memcpy(data.material_names, room->wallMaterials.data(), sizeof(data.material_names));
		
		//compute the reflection data
		auto ref_data = vraudio::ComputeReflectionProperties(data);
		auto rev_data = vraudio::ComputeReverbProperties(data);
				
		//write changes
		room->audioEngine->SetReflectionProperties(ref_data);
		room->audioEngine->SetReverbProperties(rev_data);
	}
}
