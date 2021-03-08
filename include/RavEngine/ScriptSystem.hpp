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

		void Tick(float fpsScale, Ref<Entity> e) {
			auto scripts = e->GetAllComponentsOfTypeFastPath<ScriptComponent>();
			for (auto script : scripts) {
				std::static_pointer_cast<ScriptComponent>(script)->Tick(fpsScale);
			}
		}
		
	protected:
		static const System::list_type queries;
	};
}
