#pragma once
#include "System.hpp"

namespace RavEngine{

class AudioRoomSyncSystem : public System{
	void Tick(float fpsScale, Ref<Entity> e) override;
	
	const list_type& QueryTypes() const override {
		return queries;
	}
	
	ctti_t ID() const override{
		return CTTI<AudioRoomSyncSystem>;
	}
	
protected:
	static const list_type queries;
	
};

}
