#include "PhysicsCollider.hpp"
#include "Entity.hpp"
#include "PhysicsBodyComponent.hpp"
#include <extensions/PxRigidActorExt.h>
#include "WeakRef.hpp"
#include "DebugDraw.hpp"
#include "PhysicsSolver.hpp"

using namespace physx;
using namespace RavEngine;

void BoxCollider::AddHook(const WeakRef<Entity>& e) {
    auto body = e.lock()->GetComponent<PhysicsBodyComponent>().value();
	//add the physics body to the Entity's physics actor
	collider = PxRigidActorExt::createExclusiveShape(*(body->rigidActor), PxBoxGeometry(extent.x, extent.y, extent.z), *material->getPhysXmat());

	//set relative transformation
	SetRelativeTransform(position, rotation);
}

void SphereCollider::AddHook(const WeakRef<RavEngine::Entity> &e){
	auto body = e.lock()->GetComponent<PhysicsBodyComponent>().value();
	
	collider = PxRigidActorExt::createExclusiveShape(*(body->rigidActor), PxSphereGeometry(radius), *material->getPhysXmat());
	
	SetRelativeTransform(position, rotation);
}

void CapsuleCollider::AddHook(const WeakRef<RavEngine::Entity> &e){
	auto body = e.lock()->GetComponent<PhysicsBodyComponent>().value();

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
	if (collider != nullptr) {
		auto actor = collider->getActor();
		if (actor != nullptr) {
			actor->detachShape(*collider);
		}
		else {
			collider->release();
		}
	}
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

// mesh collider constructor
void MeshCollider::AddHook(const WeakRef<RavEngine::Entity> &e){
	auto& meshdata = meshAsset->GetSystemCopy();
	
	std::vector<PxVec3> vertices(meshdata.vertices.size());
	std::vector<PxU32> indices(meshdata.indices.size());
	// only want positional data here, UVs and other data are not relevant
	for(int i = 0; i < vertices.size(); i++){
		vertices[i] = PxVec3(meshdata.vertices[i].position[0],meshdata.vertices[i].position[1],meshdata.vertices[i].position[2]);
	}
	
	for(int i = 0; i < indices.size(); i++){
		indices[i] = meshdata.indices[i];
	}
	
	// cooking data info
	PxTriangleMeshDesc meshDesc;
	meshDesc.setToDefault();
	meshDesc.points.data = &vertices[0];
	meshDesc.points.stride = sizeof(vertices[0]);
	meshDesc.points.count = vertices.size();
	
	meshDesc.triangles.count = indices.size() / 3;
	meshDesc.triangles.stride = 3 * sizeof(indices[0]);
	meshDesc.triangles.data = &indices[0];
	
	// specify width
	if (sizeof(indices[0]) == sizeof(uint16_t)){
		meshDesc.flags = PxMeshFlag::e16_BIT_INDICES;
	}	//otherwise assume 32 bit
	
#ifdef _DEBUG
	//Debug::Assert(PhysicsSolver::cooking->validateTriangleMesh(meshDesc), "Triangle mesh validation failed");
#endif
	
	triMesh = PhysicsSolver::cooking->createTriangleMesh(meshDesc, PhysicsSolver::phys->getPhysicsInsertionCallback());
	
	auto body = e.lock()->GetComponent<PhysicsBodyComponent>().value();
	
	collider = PxRigidActorExt::createExclusiveShape(*(body->rigidActor), PxTriangleMeshGeometry(triMesh), *material->getPhysXmat());
}

MeshCollider::~MeshCollider(){
	triMesh->release();
}
