#include "ScriptComponent.hpp"

using namespace RavEngine;

Transform& RavEngine::ScriptComponent::GetTransform()
{
	return GetOwner().GetTransform();
}
