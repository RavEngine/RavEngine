#include "Transform.hpp"
#include "mathtypes.hpp"
#include <glm/gtc/type_ptr.hpp>
#include "Common3D.hpp"

using namespace std;
using namespace glm;
using namespace RavEngine;

void Transform::AddChild(const WeakRef<Transform>& child)
{
	childModifyLock.lock();
	Ref<Transform> ctrans(child);
	auto worldPos = ctrans->GetWorldPosition();
	auto worldRot = ctrans->GetWorldRotation();
	
	ctrans->parent = shared_from_this();
	children.insert(child);
	
	ctrans->SetWorldPosition(worldPos);
	ctrans->SetWorldRotation(worldRot);
	childModifyLock.unlock();
}

void Transform::RemoveChild(const WeakRef<Transform>& child)
{
	childModifyLock.lock();
	Ref<Transform> ctrans(child);
	auto worldPos = ctrans->GetWorldPosition();
	auto worldRot = ctrans->GetWorldRotation();
	ctrans->parent.reset();
	children.erase(child);
	ctrans->SetWorldPosition(worldPos);
	ctrans->SetWorldRotation(worldRot);
	childModifyLock.unlock();
}
