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

		void Tick(float fpsScale, Ref<Component> c) {
            //don't need to look at id here, it's always CTTI<ScriptComponent>()
            if (!c->getOwner().expired()){
                std::static_pointer_cast<ScriptComponent>(c)->Tick(fpsScale);
            }
		}
	};
}
