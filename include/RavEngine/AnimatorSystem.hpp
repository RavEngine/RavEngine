#pragma once
#include "System.hpp"
#include "AnimatorComponent.hpp"

namespace RavEngine{
class AnimatorSystem : public AutoCTTI{
public:
	void Tick(float fpsScale, Ref<AnimatorComponent> c) {
		c->Tick(fpsScale);
	}
};
}
