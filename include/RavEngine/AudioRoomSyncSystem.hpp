#pragma once
#include "CTTI.hpp"
#include "AudioRoom.hpp"
#include "ComponentHandle.hpp"

namespace RavEngine{
struct Transform;
class AudioRoomSyncSystem : public AutoCTTI {
public:
	void operator()(float fpsScale, AudioRoom& c, Transform& tr) const;
};

}
