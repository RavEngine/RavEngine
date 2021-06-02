#pragma once

#include "System.hpp"
#include "ScriptComponent.hpp"
#include "CTTI.hpp"
#include "QueryIterator.hpp"

namespace RavEngine {
	class ScriptSystem : public AutoCTTI {
	public:
		constexpr QueryIteratorAND<ScriptComponent> QueryTypes() const {
			return QueryIteratorAND<ScriptComponent>();
		}

		void Tick(float fpsScale, AccessReadWrite<ScriptComponent> c) {
			c.get()->Tick(fpsScale);
		}
	};
}
