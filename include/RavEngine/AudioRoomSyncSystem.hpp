#pragma once
#include "CTTI.hpp"
#include "AudioRoom.hpp"
#include "ComponentHandle.hpp"

namespace RavEngine{
struct Transform;
class AudioRoomSyncSystem : public AutoCTTI {
public:
	void operator()(AudioRoom& c, Transform& tr) const;
};

}
