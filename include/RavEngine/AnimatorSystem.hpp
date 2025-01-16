#pragma once
#include "AnimatorComponent.hpp"
#include "GetApp.hpp"

namespace RavEngine{
/**
* Updates all AnimatorComponents
*/
class AnimatorSystem : public AutoCTTI{
public:
    inline void operator()(AnimatorComponent& c, const Transform& t) const {
		c.Tick(t);
	}
};
}
