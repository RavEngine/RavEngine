#pragma once
#include "System.hpp"

namespace RavEngine{

class AudioRoomSyncSystem{
public:
	void Tick(float fpsScale, Ref<Entity> e);
	
	const System::list_type& QueryTypes() const {
		return queries;
	}
	
protected:
	static const System::list_type queries;
	
};

}
