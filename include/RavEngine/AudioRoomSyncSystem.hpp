#pragma once
#include "System.hpp"
#include "CTTI.hpp"
#include "QueryIterator.hpp"

namespace RavEngine{

class AudioRoom;

class AudioRoomSyncSystem : public AutoCTTI {
public:
	void Tick(float fpsScale, Ref<Component> c, ctti_t id);
	
	constexpr QueryIteratorAND<AudioRoom> QueryTypes() const {
		return QueryIteratorAND<AudioRoom>();
	}
};

}
