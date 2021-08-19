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
	auto owner = GetOwner().lock();
	return owner  && owner->IsInWorld();
}

Ref<Transform> RavEngine::ScriptComponent::GetTransform()
{
	auto owner = GetOwner().lock();
	if (!owner) {
		throw std::runtime_error("Cannot get transform from Script with no attached Entity");
	}
	return owner->GetTransform();
}

Ref<RavEngine::World> RavEngine::ScriptComponent::GetWorld()
{
	auto owner = Ref<Entity>(GetOwner());
	if (!owner) {
		return nullptr;
	}
	return owner->GetWorld().lock();
}
