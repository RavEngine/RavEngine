#include "ScriptComponent.hpp"
#include "Ref.hpp"
#include "Entity.hpp"
#include "World.hpp"

using namespace RavEngine;

bool RavEngine::ScriptComponent::Destroy()
{
	auto owner = GetOwner().lock();
	if (owner) {
		owner->Destroy();
		return true;
	}
	return false;
}

bool RavEngine::ScriptComponent::IsInWorld()
{
    return true;
}

Transform& RavEngine::ScriptComponent::GetTransform()
{
	auto owner = GetOwner().lock();
	if (!owner) {
		throw std::runtime_error("Cannot get transform from Script with no attached Entity");
	}
	return owner->GetTransform();
}
