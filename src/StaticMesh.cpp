#if !RVE_SERVER

#include "StaticMesh.hpp"
#include "World.hpp"
#include "SkinnedMeshComponent.hpp"

void RavEngine::StaticMesh::updateMaterialInWorldRenderData(decltype(material) to)
{
	// if mesh is not enabled, it does not have render data in the world
	if (GetEnabled()){
		auto prev = GetMaterial();
		auto owner = GetOwner();
		auto world = owner.GetWorld();
		auto localID = owner.GetID();
		world->updateStaticMeshMaterial(localID, prev, to,GetMesh());
	}
	
}

void RavEngine::SkinnedMeshComponent::updateMaterialInWorldRenderData(decltype(mat) to)
{
	// if mesh is not enabled, it does not have render data in the world
	if (GetEnabled()){
		auto prev = GetMaterial();
		auto owner = GetOwner();
		auto world = owner.GetWorld();
		auto localID = owner.GetID();
		world->updateSkinnedMeshMaterial(localID, prev, to, GetMesh(), GetSkeleton());
	}
	
}
void RavEngine::StaticMesh::SetEnabled(bool in){
	Disableable::SetEnabled(in);
	// signal world
	auto owner = GetOwner();
	auto world = owner.GetWorld();
	world->StaticMeshChangedVisibility(this);
}

void RavEngine::SkinnedMeshComponent::SetEnabled(bool in){
	Disableable::SetEnabled(in);
	// signal world
	auto owner = GetOwner();
	auto world = owner.GetWorld();
	world->SkinnedMeshChangedVisibility(this);
}

#endif
