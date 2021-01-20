#include "PhysicsCollider.hpp"
#include "Entity.hpp"
#include "PhysicsBodyComponent.hpp"
#include <extensions/PxRigidActorExt.h>
#include "WeakRef.hpp"
#include "DebugDraw.hpp"

using namespace physx;
using namespace RavEngine;

void BoxCollider::AddHook(const WeakRef<Entity>& e) {
    auto body = e.lock()->GetComponentOfSubclass<PhysicsBodyComponent>();
	//add the physics body to the Entity's physics actor
	collider = PxRigidActorExt::createExclusiveShape(*(body->rigidActor), PxBoxGeometry(extent.x, extent.y, extent.z), *material->getPhysXmat());

	//set relative transformation
	SetRelativeTransform(position, rotation);
}

void SphereCollider::AddHook(const WeakRef<RavEngine::Entity> &e){
	auto body = e.lock()->GetComponentOfSubclass<PhysicsBodyComponent>();
	
	collider = PxRigidActorExt::createExclusiveShape(*(body->rigidActor), PxSphereGeometry(radius), *material->getPhysXmat());
	
	SetRelativeTransform(position, rotation);
}

void CapsuleCollider::AddHook(const WeakRef<RavEngine::Entity> &e){
	auto body = e.lock()->GetComponentOfSubclass<PhysicsBodyComponent>();

	collider = PxRigidActorExt::createExclusiveShape(*(body->rigidActor), PxCapsuleGeometry(radius,halfHeight), *material->getPhysXmat());

	SetRelativeTransform(position, rotation);
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

PhysicsCollider::CollisionType RavEngine::PhysicsCollider::GetType() const
{
	return collider->getFlags() & PxShapeFlag::eTRIGGER_SHAPE ? CollisionType::Trigger : CollisionType::Collider;
}

void RavEngine::PhysicsCollider::SetQueryable(bool state)
{
	collider->setFlag(PxShapeFlag::eSCENE_QUERY_SHAPE,state);
}

bool RavEngine::PhysicsCollider::GetQueryable() const
{
	return collider->getFlags() & PxShapeFlag::eSCENE_QUERY_SHAPE;
}

PhysicsCollider::~PhysicsCollider() {
	//collider->release();
}

void PhysicsCollider::SetRelativeTransform(const vector3 &position, const quaternion &rotation){
	collider->setLocalPose(PxTransform(PxVec3(position.x,position.y,position.z),PxQuat(rotation.x,rotation.y,rotation.z,rotation.w)));
}

matrix4 PhysicsCollider::CalculateWorldMatrix() const{
	return Ref<Entity>(getOwner())->transform()->CalculateWorldMatrix() * (matrix4)Transformation{position,rotation};;
}

void BoxCollider::DebugDraw(RavEngine::DebugDraw& dbg, const color_t color) const{
	dbg.DrawRectangularPrism(CalculateWorldMatrix(), color, vector3(extent.x * 2, extent.y * 2, extent.z*2));
}

void SphereCollider::DebugDraw(RavEngine::DebugDraw& dbg, const color_t color) const{
	dbg.DrawSphere(CalculateWorldMatrix(), color, radius);
}

void CapsuleCollider::DebugDraw(RavEngine::DebugDraw& dbg, const color_t color) const{
	dbg.DrawCapsule(glm::translate(glm::rotate(CalculateWorldMatrix(), glm::radians(90.0), vector3(0,0,1)), vector3(0,-halfHeight,0)) , color, radius, halfHeight * 2);
}
