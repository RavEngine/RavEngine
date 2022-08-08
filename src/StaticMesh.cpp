#include "StaticMesh.hpp"
#include "World.hpp"

void RavEngine::StaticMesh::updateMaterialInWorldRenderData(Ref<PBRMaterialInstance> to)
{
	auto prev = GetMaterial();
	auto owner = GetOwner();
	auto world = owner.GetWorld();
	auto localID = owner.GetIdInWorld();
	world->updateStaticMeshMaterial(localID, prev, to,getMesh());
}
