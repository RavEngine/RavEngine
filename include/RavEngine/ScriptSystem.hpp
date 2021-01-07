#pragma once

#include "System.hpp"
#include "ScriptComponent.hpp"
#include "CTTI.hpp"

namespace RavEngine {
	class ScriptSystem : public System {
	public:
		ScriptSystem() : System() {}

		plf::list<ctti_t> QueryTypes() const override {
			return {CTTI<ScriptComponent>};
		}

		void Tick(float fpsScale, Ref<Entity> e) override{
			auto scripts = e->GetAllComponentsOfTypeSubclassFastPath<ScriptComponent>();
			for (const Ref<ScriptComponent>& script : scripts) {
				script->Tick(fpsScale);
			}
		}
	};
}
