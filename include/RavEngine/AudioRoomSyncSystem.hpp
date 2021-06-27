#pragma once
#include "System.hpp"
#include "CTTI.hpp"
#include "AudioRoom.hpp"

namespace RavEngine{

class AudioRoomSyncSystem : public AutoCTTI {
public:
	void Tick(float fpsScale, Ref<AudioRoom> c);
};

}
