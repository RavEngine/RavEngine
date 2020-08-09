#include "PhysicsCollider.hpp"
#include "Entity.hpp"
#include "PhysicsBodyComponent.hpp"
#include <extensions/PxRigidActorExt.h>
#include "WeakRef.hpp"

using namespace physx;

void BoxCollider::AddHook(WeakRef<Entity> e) {
    auto body = e.get()->Components().GetComponentOfSubclass<PhysicsBodyComponent>();
	//add the physics body to the Entity's physics actor
	PxRigidActorExt::createExclusiveShape(*(body->rigidActor), PxBoxGeometry(extent.x, extent.y, extent.z), *material);
}
