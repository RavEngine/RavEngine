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
	
	void Tick(float fpsScale, AccessReadWrite<AnimatorComponent> c) {
		c.get()->Tick(fpsScale);
	}
};
}
