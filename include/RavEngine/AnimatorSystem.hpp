#pragma once
#include "System.hpp"
#include "AnimatorComponent.hpp"

namespace RavEngine{
class AnimatorSystem : public AutoCTTI{
public:
	inline void Tick(float fpsScale, Ref<AnimatorComponent> c) const {
		c->Tick(fpsScale);
	}
};
}
