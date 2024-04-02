#if !RVE_SERVER

#pragma once
#include "CTTI.hpp"

namespace RavEngine{
struct Transform;
class SimpleAudioSpace;
class AudioRoomSyncSystem : public AutoCTTI {
public:
	void operator()(SimpleAudioSpace& c, Transform& tr) const;
};

}
#endif
