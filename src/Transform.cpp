#include "Transform.hpp"
#include "mathtypes.hpp"
#include <glm/gtc/type_ptr.hpp>
#include "Common3D.hpp"

using namespace std;
using namespace glm;
using namespace RavEngine;

void Transform::AddChild(const WeakRef<Transform>& child)
{
	Ref<Transform> ctrans(child);
	auto worldPos = ctrans->GetWorldPosition();
	auto worldRot = ctrans->GetWorldRotation();
	
	ctrans->parent = WeakRef<Transform>(this);
	children.insert(child);
	
	ctrans->SetWorldPosition(worldPos);
	ctrans->SetWorldRotation(worldRot);
}

void Transform::RemoveChild(const WeakRef<Transform>& child)
{
	Ref<Transform> ctrans(child);
	auto worldPos = ctrans->GetWorldPosition();
	auto worldRot = ctrans->GetWorldRotation();
	ctrans->parent = nullptr;
	children.erase(child);
	ctrans->SetWorldPosition(worldPos);
	ctrans->SetWorldRotation(worldRot);
}
