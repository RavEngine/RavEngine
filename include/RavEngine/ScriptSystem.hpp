#pragma once

#include "System.hpp"
#include "ScriptComponent.hpp"
#include "CTTI.hpp"

namespace RavEngine {
	class ScriptSystem {
	public:
		const System::list_type& QueryTypes() const {
			return queries;
		}

		void Tick(float fpsScale, Ref<Component> c, ctti_t id) {
            //don't need to look at id here, it's always CTTI<ScriptComponent>()
            if (!c->getOwner().expired()){
                std::static_pointer_cast<ScriptComponent>(c)->Tick(fpsScale);
            }
		}
		
	protected:
		static const System::list_type queries;
	};
}
