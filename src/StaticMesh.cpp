#include "StaticMesh.hpp"
#include "Entity.hpp"
#include "mathtypes.hpp"
#include <SDL_stdinc.h>
#include <fstream>
#include <sstream>
#include "RenderEngine.hpp"
#include "Common3D.hpp"


using namespace RavEngine;
using namespace std;

StaticMesh::StaticMesh(Ref<MeshAsset> m) : RenderableComponent(), mesh(m) {}


void RavEngine::StaticMesh::SetMaterial(Ref<PBRMaterialInstance> mat)
{
	material = mat;
}

void RavEngine::StaticMesh::Draw(int view)
{
    //skip draw if no material or MeshAsset assigned
    if (material.isNull() || mesh.isNull()) {
        return;
    }
    //apply transform and set it for the material, if no owner, do not attempt to draw
	auto owner = Ref<Entity>(getOwner());
    if (owner) {
        if (owner && owner->HasComponentOfType<Transform>()) {
            owner->transform()->Apply();
            material->Draw(mesh->getVertexBuffer(), mesh->getIndexBuffer(), owner->transform()->GetCurrentWorldMatrix(), view);
        }
    }
}
