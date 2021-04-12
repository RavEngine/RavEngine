#pragma once
#include "System.hpp"
#include "AnimatorComponent.hpp"

namespace RavEngine{
class AnimatorSystem : public AutoCTTI{
public:
	const System::list_type& QueryTypes() const {
		return queries;
	}
	
	void Tick(float fpsScale, Ref<Component> c, ctti_t id) {
		std::static_pointer_cast<AnimatorComponent>(c)->Tick(fpsScale);
	}
	
protected:
	static const System::list_type queries;
};
}
