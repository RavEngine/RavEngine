#include "PhysicsCollider.hpp"
#include "Entity.hpp"
#include "PhysicsBodyComponent.hpp"
#include <extensions/PxRigidActorExt.h>
#include "WeakRef.hpp"
#include "DebugDrawer.hpp"
#include "PhysicsSolver.hpp"
#include "Transform.hpp"

using namespace physx;
using namespace RavEngine;

void PhysicsCollider::UpdateFilterData(PhysicsBodyComponent* owner){
    PxFilterData filterData;
    filterData.word0 = owner->filterGroup; // word0 = own ID
    filterData.word1 = owner->filterMask;
    collider->setSimulationFilterData(filterData);
}

BoxCollider::BoxCollider(PhysicsBodyComponent* owner, const vector3& ext, Ref<PhysicsMaterial> mat, const vector3& position, const quaternion& rotation) : extent(ext){
    material = mat;
    
    //add the physics body to the Entity's physics actor
    collider = PxRigidActorExt::createExclusiveShape(*owner->rigidActor, PxBoxGeometry(extent.x, extent.y, extent.z), *material->GetPhysXmat());

    //set relative transformation
    SetRelativeTransform(position, rotation);
    UpdateFilterData(owner);
}

SphereCollider::SphereCollider(PhysicsBodyComponent* owner, decimalType r, Ref<PhysicsMaterial> mat, const vector3& position, const quaternion& rotation){
    material = mat;
    collider = PxRigidActorExt::createExclusiveShape(*owner->rigidActor, PxSphereGeometry(r), *material->GetPhysXmat());

    SetRelativeTransform(position, rotation);
    UpdateFilterData(owner);
}
//
//void SphereCollider::SetRadius(decimalType newRadius){
//   
//    collider->getGeometry().sphere().radius = radius;
//}

decimalType SphereCollider::GetRadius() const{
    return collider->getGeometry().sphere().radius;
}


CapsuleCollider::CapsuleCollider(PhysicsBodyComponent* owner, decimalType r, decimalType hh, Ref<PhysicsMaterial> mat, const vector3& position, const quaternion& rotation) : radius(r), halfHeight(hh){
    material = mat;
    
    collider = PxRigidActorExt::createExclusiveShape(*owner->rigidActor, PxCapsuleGeometry(radius,halfHeight), *material->GetPhysXmat());

    SetRelativeTransform(position, rotation);
    UpdateFilterData(owner);
}

MeshCollider::MeshCollider(PhysicsBodyComponent* owner, Ref<MeshAsset> meshAsset, Ref<PhysicsMaterial> mat)
#ifndef NDEBUG
 : mesh(meshAsset)
#endif
{
    material = mat;
    auto& meshdata = meshAsset->GetSystemCopy();
    
    RavEngine::Vector<PxVec3> vertices(meshdata.vertices.size());
    RavEngine::Vector<PxU32> indices(meshdata.indices.size());
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
	assert(vertices.size() < std::numeric_limits<physx::PxU32>::max());
    meshDesc.points.count = static_cast<physx::PxU32>(vertices.size());
    
	assert(indices.size() / 3 < std::numeric_limits<physx::PxU32>::max());
    meshDesc.triangles.count = static_cast<physx::PxU32>(indices.size() / 3);
    meshDesc.triangles.stride = 3 * sizeof(indices[0]);
    meshDesc.triangles.data = &indices[0];
    
    // specify width
    if (sizeof(indices[0]) == sizeof(uint16_t)){
        meshDesc.flags = PxMeshFlag::e16_BIT_INDICES;
    }    //otherwise assume 32 bit
    
#ifndef NDEBUG
    //Debug::Assert(PhysicsSolver::cooking->validateTriangleMesh(meshDesc), "Triangle mesh validation failed");
#endif
    
    physx::PxTriangleMesh* triMesh = PhysicsSolver::cooking->createTriangleMesh(meshDesc, PhysicsSolver::phys->getPhysicsInsertionCallback());
    
    collider = PxRigidActorExt::createExclusiveShape(*owner->rigidActor, PxTriangleMeshGeometry(triMesh), *material->GetPhysXmat());
    triMesh->release();
    UpdateFilterData(owner);
}

ConvexMeshCollider::ConvexMeshCollider(PhysicsBodyComponent* owner, Ref<MeshAsset> meshAsset, Ref<PhysicsMaterial> mat) {
    material = mat;
    
    auto& meshdata = meshAsset->GetSystemCopy();
    
    // only want positional data here, UVs and other data are not relevant
    RavEngine::Vector<PxVec3> vertices(meshdata.vertices.size());
    for(int i = 0; i < vertices.size(); i++){
        vertices[i] = PxVec3(meshdata.vertices[i].position[0],meshdata.vertices[i].position[1],meshdata.vertices[i].position[2]);
    }
    
    PxBoundedData pointdata;
	assert(vertices.size() < std::numeric_limits<physx::PxU32>::max());
    pointdata.count = static_cast<physx::PxU32>(vertices.size());
    pointdata.stride = sizeof(PxVec3);
    pointdata.data = &vertices[0];
    
    PxConvexMeshDesc meshDesc;
    meshDesc.setToDefault();
    meshDesc.points.count = static_cast<physx::PxU32>(vertices.size());
    meshDesc.points = pointdata;
    meshDesc.flags = PxConvexFlag::eCOMPUTE_CONVEX;
    
    physx::PxConvexMesh* convMesh = PhysicsSolver::cooking->createConvexMesh(meshDesc, PhysicsSolver::phys->getPhysicsInsertionCallback());
    
    collider = PxRigidActorExt::createExclusiveShape(*owner->rigidActor, PxConvexMeshGeometry(convMesh), *material->GetPhysXmat());
    UpdateFilterData(owner);
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

void PhysicsCollider::SetRelativeTransform(const vector3 &position, const quaternion &rotation){
	collider->setLocalPose(PxTransform(PxVec3(position.x,position.y,position.z),PxQuat(rotation.x,rotation.y,rotation.z,rotation.w)));
}

Transformation PhysicsCollider::GetRelativeTransform() const{
	auto pose = collider->getLocalPose();
	return Transformation{vector3(pose.p.x,pose.p.y,pose.p.z),quaternion(pose.q.w, pose.q.x,pose.q.y,pose.q.z)};
}

matrix4 PhysicsCollider::CalculateWorldMatrix(const RavEngine::Transform& tr) const{
	return tr.CalculateWorldMatrix() * (matrix4)GetRelativeTransform();
}

void BoxCollider::DebugDraw(RavEngine::DebugDrawer& dbg,color_t debug_color, const RavEngine::Transform& tr) const{
    dbg.DrawRectangularPrism(CalculateWorldMatrix(tr), debug_color, vector3(extent.x * 2, extent.y * 2, extent.z*2));
}

void SphereCollider::DebugDraw(RavEngine::DebugDrawer& dbg,color_t debug_color, const RavEngine::Transform& tr) const{
    dbg.DrawSphere(CalculateWorldMatrix(tr), debug_color, GetRadius());
}

void CapsuleCollider::DebugDraw(RavEngine::DebugDrawer& dbg, color_t debug_color, const RavEngine::Transform& tr) const{
    dbg.DrawCapsule(glm::translate(glm::rotate(CalculateWorldMatrix(tr), deg_to_rad(90), vector3(0,0,1)), vector3(0,-halfHeight,0)) , debug_color, radius, halfHeight * 2);
}

void MeshCollider::DebugDraw(RavEngine::DebugDrawer &dbg, color_t color, const RavEngine::Transform & tr) const{
#ifndef NDEBUG
    dbg.DrawWireframeMesh(CalculateWorldMatrix(tr), mesh);
#endif
}
