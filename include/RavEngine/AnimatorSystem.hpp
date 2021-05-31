#pragma once
#include "System.hpp"
#include "AnimatorComponent.hpp"
#include "QueryIterator.hpp"

namespace RavEngine{
class AnimatorSystem : public AutoCTTI{
public:
	constexpr inline QueryIteratorAND<AnimatorComponent> QueryTypes() const {
		return QueryIteratorAND<AnimatorComponent>();
	}
	
	void Tick(float fpsScale, Ref<Component> c, ctti_t id) {
		std::static_pointer_cast<AnimatorComponent>(c)->Tick(fpsScale);
	}
};
}
