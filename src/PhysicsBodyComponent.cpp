
#include "PhysicsBodyComponent.hpp"
#include "PhysicsSolver.hpp"
#include <foundation/PxTransform.h>
#include <foundation/PxVec3.h>
#include <PxRigidBody.h>
#include "Transform.hpp"
#include "Entity.hpp"

using namespace physx;
using namespace RavEngine;

/// Dynamic Body ========================================

RigidBodyDynamicComponent::RigidBodyDynamicComponent() {
	rigidActor = PhysicsSolver::phys->createRigidDynamic(PxTransform(PxVec3(0, 0, 0)));	//will be set pre-tick to the entity's location
	RegisterAllAlternateTypes();
}

RavEngine::PhysicsBodyComponent::~PhysicsBodyComponent()
{
	//note: do not need to delete the rigid actor here. The PhysicsSolver will delete it
	if (rigidActor != nullptr) {
		rigidActor->release();
	}
}

vector3 PhysicsBodyComponent::getPos() {
	auto pos = rigidActor->getGlobalPose();
	return vector3(pos.p.x, pos.p.y, pos.p.z);
}

void PhysicsBodyComponent::setPos(const vector3& pos) {
	rigidActor->setGlobalPose(PxTransform(PxVec3(pos.x, pos.y, pos.z),rigidActor->getGlobalPose().q));
}

quaternion PhysicsBodyComponent::getRot() {
	auto rot = rigidActor->getGlobalPose();
	return quaternion(rot.q.w, rot.q.x,rot.q.y,rot.q.z);
}

void PhysicsBodyComponent::setRot(const quaternion& quat) {
	rigidActor->setGlobalPose(PxTransform(rigidActor->getGlobalPose().p,PxQuat(quat.x,quat.y,quat.z,quat.w)));
}

/**
Enable or disable gravity on this body
@param state the new state of gravity
*/
void RavEngine::PhysicsBodyComponent::SetGravityEnabled(bool state)
{
	rigidActor->setActorFlag(PxActorFlag::eDISABLE_GRAVITY,state);
}

bool RavEngine::PhysicsBodyComponent::GetGravityEnabled()
{
	return rigidActor->getActorFlags() & PxActorFlag::eDISABLE_GRAVITY;
}

/**
Sets whether or not onWake and onSleep events get called.
@param state new state for this property
*/
void RavEngine::PhysicsBodyComponent::SetSleepNotificationsEnabled(bool state)
{
	rigidActor->setActorFlag(PxActorFlag::eSEND_SLEEP_NOTIFIES, state);
}

bool RavEngine::PhysicsBodyComponent::GetSleepNotificationsEnabled()
{
	return rigidActor->getActorFlags() & PxActorFlag::eSEND_SLEEP_NOTIFIES;
}

/**
Enable or disable simulation for this body.
@param state the new simulation status.
@note If simulation is disbaled, all constraints are removed, all velocities and forces are cleared, and actor is set to sleep.
*/
void RavEngine::PhysicsBodyComponent::SetSimulationEnabled(bool state)
{
	rigidActor->setActorFlag(PxActorFlag::eDISABLE_SIMULATION,state);
}

bool RavEngine::PhysicsBodyComponent::GetSimulationEnabled()
{
	return rigidActor->getActorFlags() & PxActorFlag::eDISABLE_SIMULATION;
}


RigidBodyDynamicComponent::~RigidBodyDynamicComponent() {
	//note: do not need to delete the rigid actor here. The PhysicsSolver will delete it.
}

vector3 RavEngine::RigidBodyDynamicComponent::GetLinearVelocity()
{
	auto vel = static_cast<PxRigidBody*>(rigidActor)->getLinearVelocity();
	return vector3(vel.x,vel.y,vel.z);
}

vector3 RavEngine::RigidBodyDynamicComponent::GetAngularVelocity()
{
	auto vel = static_cast<PxRigidBody*>(rigidActor)->getAngularVelocity();
	return vector3(vel.x,vel.y,vel.z);
}

/**
Set the linear velocity of the physics body
@param newvel the new velocity for each axis
@param autowake whether to automatically wake this physics body to apply the change. If set to false, the body must be woken manually.
*/
void RavEngine::RigidBodyDynamicComponent::SetLinearVelocity(const vector3& newvel, bool autowake)
{
	static_cast<PxRigidBody*>(rigidActor)->setLinearVelocity(PxVec3(newvel.x, newvel.y, newvel.z),autowake);
}

/**
Set the angular velocity of the physics body
@param newvel the new velocity for each euler axis
@param autowake whether to automatically wake this physics body to apply the change. If set to false, the body must be woken manually.
*/
void RavEngine::RigidBodyDynamicComponent::SetAngularVelocity(const vector3& newvel, bool autowake)
{
	static_cast<PxRigidBody*>(rigidActor)->setAngularVelocity(PxVec3(newvel.x, newvel.y, newvel.z), autowake);
}

void RavEngine::RigidBodyDynamicComponent::Wake()
{
	static_cast<PxRigidDynamic*>(rigidActor)->wakeUp();
}

void RavEngine::RigidBodyDynamicComponent::Sleep()
{
	static_cast<PxRigidDynamic*>(rigidActor)->putToSleep();
}

bool RavEngine::RigidBodyDynamicComponent::IsSleeping()
{
	return static_cast<PxRigidDynamic*>(rigidActor)->isSleeping();
}

void PhysicsBodyComponent::OnColliderEnter(PhysicsBodyComponent* other)
{
	Ref<RavEngine::Entity>(owner)->OnColliderEnter(other);
}

void PhysicsBodyComponent::OnColliderPersist(PhysicsBodyComponent* other)
{
	Ref<RavEngine::Entity>(owner)->OnColliderPersist(other);
}

void PhysicsBodyComponent::OnColliderExit(PhysicsBodyComponent* other)
{
	Ref<RavEngine::Entity>(owner)->OnColliderExit(other);
}

/// Static Body ========================================
RigidBodyStaticComponent::RigidBodyStaticComponent() {
	rigidActor = PhysicsSolver::phys->createRigidStatic(PxTransform(PxVec3(0, 0, 0)));	//will be set pre-tick to the entity's location
	RegisterAllAlternateTypes();
}

RigidBodyStaticComponent::~RigidBodyStaticComponent() {
	//note: do not need to delete the rigid actor here. The PhysicsSolver will delete it
}