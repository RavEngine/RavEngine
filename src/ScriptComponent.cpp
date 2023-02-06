#include "ScriptComponent.hpp"
#include "ScriptSystem.hpp"
#include "App.hpp"

using namespace RavEngine;

Transform& RavEngine::ScriptComponent::GetTransform()
{
	return GetOwner().GetTransform();
}

void ScriptSystem::operator()(ScriptComponent& c) const{
    const auto fpsScale = GetApp()->GetCurrentFPSScale();
    c.Tick(fpsScale);
}
