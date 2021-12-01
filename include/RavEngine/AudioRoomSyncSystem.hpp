#pragma once
#include "System.hpp"
#include "CTTI.hpp"
#include "AudioRoom.hpp"
#include "ComponentHandle.hpp"

namespace RavEngine{
struct Transform;
class AudioRoomSyncSystem : public AutoCTTI {
public:
	void operator()(float fpsScale, ComponentHandle<AudioRoom> c, ComponentHandle<Transform> tr) const;
};

}
