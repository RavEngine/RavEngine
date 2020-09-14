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

// Vertex data structure

Vertex vertices[] =
{
    {{ -1, -1, -1 }, {1,0,0}}, {{ -1,  1, -1 },{0,1,0}}, {{  1,  1, -1 },{0,0,1}}, {{  1, -1, -1 },{1,0,0}},
    {{ -1, -1,  1 },{0,1,0}}, {{ -1,  1,  1 },{0,0,1}}, {{  1,  1,  1 },{1,0,0}}, {{  1, -1,  1 },{0,1,0}},
};

uint32_t indices[] = {
    0, 1, 2, 0, 2, 3, // front
    3, 2, 6, 3, 6, 7, // right
    4, 5, 1, 4, 1, 0, // left
    1, 5, 6, 1, 6, 2, // top
    4, 0, 3, 4, 3, 7, // bottom
    7, 6, 5, 7, 5, 4, // back
};


StaticMesh::StaticMesh() : RenderableComponent() {
	bgfx::VertexLayout pcvDecl;
	
	//vertex format
	pcvDecl.begin()
	.add(bgfx::Attrib::Position, 3, bgfx::AttribType::Float)
	.add(bgfx::Attrib::Color0, 4, bgfx::AttribType::Uint8, true)
	.end();
	
	//create buffers
	vertexBuffer = bgfx::createVertexBuffer(bgfx::makeRef(vertices, sizeof(vertices)), pcvDecl);
	indexBuffer = bgfx::createIndexBuffer(bgfx::makeRef(indices, sizeof(indices)));
   
    RegisterAllAlternateTypes();
}

RavEngine::StaticMesh::~StaticMesh()
{
    //delete indexBuffer; indexBuffer = nullptr;
    //delete vertexBuffer; vertexBuffer = nullptr;
}

void RavEngine::StaticMesh::SetMaterial(Ref<Material> mat)
{
	material = mat;
}

void RavEngine::StaticMesh::Draw()
{
    //skip draw if no material assigned
    if (material.isNull()) {
        return;
    }
    //apply transform and set it for the material
    auto owner = Ref<Entity>(getOwner());
    owner->transform()->Apply();
    material->SetTransformMatrix(owner->transform()->GetCurrentWorldMatrix());
    material->Draw(vertexBuffer, indexBuffer);
}

void RavEngine::StaticMesh::AddHook(const WeakRef<RavEngine::Entity>& e)
{
}
