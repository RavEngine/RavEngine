#pragma once
#include "System.hpp"
#include "CTTI.hpp"

namespace RavEngine{

class AudioRoomSyncSystem : public AutoCTTI {
public:
	void Tick(float fpsScale, Ref<Component> c, ctti_t id);
	
	const System::list_type& QueryTypes() const {
		return queries;
	}
	
protected:
	static const System::list_type queries;
	
};

}
