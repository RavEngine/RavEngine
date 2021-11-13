#include "Transform.hpp"
#include "mathtypes.hpp"
#include <glm/gtc/type_ptr.hpp>
#include "Common3D.hpp"

using namespace std;
using namespace glm;
using namespace RavEngine;

void Transform::AddChild(ComponentHandle<Transform> child)
{
	childModifyLock.lock();
    auto cptr = child.Get();
	auto worldPos = cptr->GetWorldPosition();
	auto worldRot = cptr->GetWorldRotation();
	
	cptr->parent = ComponentHandle<Transform>(GetOwner());
	children.insert(child);
	
    cptr->SetWorldPosition(worldPos);
    cptr->SetWorldRotation(worldRot);
	childModifyLock.unlock();
}

void Transform::RemoveChild(ComponentHandle<Transform> child)
{
	childModifyLock.lock();
    auto cptr = child.Get();
	auto worldPos = cptr->GetWorldPosition();
	auto worldRot = cptr->GetWorldRotation();
	cptr->parent.reset();
	children.erase(child);
	cptr->SetWorldPosition(worldPos);
	cptr->SetWorldRotation(worldRot);
	childModifyLock.unlock();
}
