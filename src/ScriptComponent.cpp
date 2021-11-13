#include "ScriptComponent.hpp"
#include "Ref.hpp"
#include "Entity.hpp"
#include "World.hpp"

using namespace RavEngine;

Transform& RavEngine::ScriptComponent::GetTransform()
{
	return GetOwner().GetTransform();
}
