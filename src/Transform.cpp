#include "Transform.hpp"
#include "mathtypes.hpp"
#include <glm/gtc/type_ptr.hpp>
#include "Common3D.hpp"

using namespace std;
using namespace glm;
using namespace RavEngine;

Transform& Transform::AddChild(ComponentHandle<Transform> child)
{
    auto cptr = child.get();
	auto worldPos = cptr->GetWorldPosition();
	auto worldRot = cptr->GetWorldRotation();
	
	cptr->parent = ComponentHandle<Transform>(GetOwner());
	children.insert(child);
	
    cptr->SetWorldPosition(worldPos);
    cptr->SetWorldRotation(worldRot);
    return *this;
}

Transform& Transform::RemoveChild(ComponentHandle<Transform> child)
{
    auto cptr = child.get();
	auto worldPos = cptr->GetWorldPosition();
	auto worldRot = cptr->GetWorldRotation();
	cptr->parent.reset();
	children.erase(child);
	cptr->SetWorldPosition(worldPos);
	cptr->SetWorldRotation(worldRot);
    return *this;
}
