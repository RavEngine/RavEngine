#pragma once
#include "CTTI.hpp"

namespace RavEngine{
struct Transform;
class AudioRoom;
class AudioRoomSyncSystem : public AutoCTTI {
public:
	void operator()(AudioRoom& c, Transform& tr) const;
};

}
