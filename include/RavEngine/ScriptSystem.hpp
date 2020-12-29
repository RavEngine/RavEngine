#pragma once

#include "System.hpp"
#include "ScriptComponent.hpp"

namespace RavEngine {
	class ScriptSystem : public System {
	public:
		ScriptSystem() : System() {}

		plf::list<std::type_index> QueryTypes() const override {
			return {typeid(ScriptComponent)};
		}

		void Tick(float fpsScale, Ref<Entity> e) const override{
			auto scripts = e->GetAllComponentsOfTypeSubclassFastPath<ScriptComponent>();
			for (const Ref<ScriptComponent>& script : scripts) {
				script->Tick(fpsScale);
			}
		}
	};
}
