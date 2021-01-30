#include "AudioRoom.hpp"
#include "AudioRoomSyncSystem.hpp"

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
		
		vraudio::ReflectionProperties data;
		
		data.room_position[0] = pos.x;
		data.room_position[1] = pos.y;
		data.room_position[2] = pos.z;
		
		data.room_rotation[0] = rot.x;
		data.room_rotation[1] = rot.y;
		data.room_rotation[2] = rot.z;
		data.room_rotation[3] = rot.w;
		
		auto dim = room->roomDimensions * scale;
		
		data.room_dimensions[0] = dim.x;
		data.room_dimensions[1] = dim.y;
		data.room_dimensions[2] = dim.z;
				
		//write changes
		room->audioEngine->SetReflectionProperties(data);
	}
}
