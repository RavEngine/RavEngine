#pragma once

#include "System.hpp"
#include "ScriptComponent.hpp"
#include "CTTI.hpp"

namespace RavEngine {
	class ScriptSystem : public AutoCTTI {
	public:
		inline void operator()(float fpsScale, ScriptComponent& c) const{
			c.Tick(fpsScale);
		}
	};
}
