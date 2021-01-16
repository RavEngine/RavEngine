#pragma once

#include "System.hpp"
#include "ScriptComponent.hpp"
#include "CTTI.hpp"

namespace RavEngine {
	class ScriptSystem : public System {
	public:
		ScriptSystem() : System() {}

		const list_type& QueryTypes() const override {
			return queries;
		}

		void Tick(float fpsScale, Ref<Entity> e) override{
			auto scripts = e->GetAllComponentsOfTypeSubclassFastPath<ScriptComponent>();
			for (auto script : scripts) {
				std::static_pointer_cast<ScriptComponent>(script)->Tick(fpsScale);
			}
		}
		
		ctti_t ID() const override{
			return CTTI<ScriptSystem>;
		}
		
	protected:
		static const list_type queries;
	};
}
