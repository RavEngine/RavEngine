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

StaticMesh::StaticMesh(Ref<MeshAsset> m) : RenderableComponent(), mesh(m) {
	
}


void RavEngine::StaticMesh::SetMaterial(Ref<MaterialInstanceBase> mat)
{
	material = mat;
}

void RavEngine::StaticMesh::Draw()
{
    //skip draw if no material or MeshAsset assigned
	if (material.isNull() || mesh.isNull()) {
        return;
    }
    //apply transform and set it for the material
    auto owner = Ref<Entity>(getOwner());
    owner->transform()->Apply();
    material->Draw(mesh->getVertexBuffer(), mesh->getIndexBuffer(), owner->transform()->GetCurrentWorldMatrix());
}
