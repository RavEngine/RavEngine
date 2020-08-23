#include "PhysicsCollider.hpp"
#include "Entity.hpp"
#include "PhysicsBodyComponent.hpp"
#include <extensions/PxRigidActorExt.h>
#include "WeakRef.hpp"

using namespace physx;
using namespace RavEngine;


void BoxCollider::AddHook(const WeakRef<Entity>& e) {
    auto body = e.get()->Components().GetComponentOfSubclass<PhysicsBodyComponent>();
	//add the physics body to the Entity's physics actor
	collider = PxRigidActorExt::createExclusiveShape(*(body->rigidActor), PxBoxGeometry(extent.x, extent.y, extent.z), *material->getPhysXmat());
}


void RavEngine::PhysicsCollider::SetType(CollisionType type)
{
	switch (type) {
	case CollisionType::Collider:
		collider->setFlag(PxShapeFlag::eSIMULATION_SHAPE,true);
		collider->setFlag(PxShapeFlag::eTRIGGER_SHAPE,false);
		break;
	case CollisionType::Trigger:
		collider->setFlag(PxShapeFlag::eSIMULATION_SHAPE, false);
		collider->setFlag(PxShapeFlag::eTRIGGER_SHAPE, true);
		break;
	}
}

PhysicsCollider::CollisionType RavEngine::PhysicsCollider::GetType()
{
	return collider->getFlags() & PxShapeFlag::eTRIGGER_SHAPE ? CollisionType::Trigger : CollisionType::Collider;
}

void RavEngine::PhysicsCollider::SetQueryable(bool state)
{
	collider->setFlag(PxShapeFlag::eSCENE_QUERY_SHAPE,state);
}

bool RavEngine::PhysicsCollider::GetQueryable()
{
	return collider->getFlags() & PxShapeFlag::eSCENE_QUERY_SHAPE;
}

PhysicsCollider::~PhysicsCollider() {
	//collider->release();
}