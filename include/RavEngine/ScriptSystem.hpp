#pragma once

#include "System.hpp"
#include "ScriptComponent.hpp"
#include "CTTI.hpp"

namespace RavEngine {
	class ScriptSystem : public AutoCTTI {
	public:
		void Tick(float fpsScale, const Ref<ScriptComponent> c) {
			c->Tick(fpsScale);
		}
	};
}
