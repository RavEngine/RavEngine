#pragma once
#include "AnimatorComponent.hpp"
#include "GetApp.hpp"

namespace RavEngine{
class AnimatorSystem : public AutoCTTI{
public:
    inline void operator()(AnimatorComponent& c) const {
		c.Tick();
	}
};
}
