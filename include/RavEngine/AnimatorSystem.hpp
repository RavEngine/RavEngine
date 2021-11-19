#pragma once
#include "System.hpp"
#include "AnimatorComponent.hpp"

namespace RavEngine{
class AnimatorSystem : public AutoCTTI{
public:
    inline void operator()(float fpsScale, AnimatorComponent& c) const {
		c.Tick(fpsScale);
	}
};
}
