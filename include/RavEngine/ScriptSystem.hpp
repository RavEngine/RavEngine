#pragma once

#include "System.hpp"
#include "ScriptComponent.hpp"

namespace RavEngine {
	class ScriptSystem : public System {
	public:
		ScriptSystem() : System() {}

		std::list<std::type_index> QueryTypes() const override {
			return {typeid(ScriptComponent)};
		}

		void Tick(float fpsScale, Ref<Entity> e) const override{
			auto& scripts = e->Components().GetAllComponentsOfSubclass<ScriptComponent>();
			for (auto& script : scripts) {
				script->Tick(fpsScale);
			}
		}
	};
}
