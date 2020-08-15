#include "Component.hpp"
#include "RenderEngine.hpp"
#include <filament/TransformManager.h>
#include <filament/Engine.h>

using namespace RavEngine;
using namespace utils;

void RavEngine::Component::filamentParentToEntity(const WeakRef<RavEngine::Entity>& e, const utils::Entity& myFilamentEntity)
{
	auto& tm = RenderEngine::getEngine()->getTransformManager();
	auto newParent = e.get()->transform()->getEntity();
	tm.create(newParent);	//important! must create the entity in the transform manager to make it valid
	tm.setParent(tm.getInstance(myFilamentEntity), tm.getInstance(newParent));
}

