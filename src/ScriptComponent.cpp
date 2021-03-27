#include "ScriptComponent.hpp"
#include "Ref.hpp"
#include "Entity.hpp"
#include "World.hpp"

using namespace RavEngine;

bool RavEngine::ScriptComponent::Destroy()
{
	auto owner = getOwner().lock();
	if (owner) {
		owner->Destroy();
		return true;
	}
	return false;
}

bool RavEngine::ScriptComponent::IsInWorld()
{
	auto owner = getOwner().lock();
	return owner  && owner->IsInWorld();
}

Ref<Transform> RavEngine::ScriptComponent::transform()
{
	auto owner = getOwner().lock();
	if (!owner) {
		throw std::runtime_error("Cannot get transform from Script with no attached Entity");
	}
	return owner->transform();
}

Ref<RavEngine::World> RavEngine::ScriptComponent::GetWorld()
{
	auto owner = Ref<Entity>(getOwner());
	if (!owner) {
		return nullptr;
	}
	return owner->GetWorld().lock();
}
