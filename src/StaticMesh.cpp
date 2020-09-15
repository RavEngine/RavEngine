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

const Vertex vertices[] =
{
	{-1.0f,  1.0f,  1.0f, 0xff000000 },
	{ 1.0f,  1.0f,  1.0f, 0xff0000ff },
	{-1.0f, -1.0f,  1.0f, 0xff00ff00 },
	{ 1.0f, -1.0f,  1.0f, 0xff00ffff },
	{-1.0f,  1.0f, -1.0f, 0xffff0000 },
	{ 1.0f,  1.0f, -1.0f, 0xffff00ff },
	{-1.0f, -1.0f, -1.0f, 0xffffff00 },
	{ 1.0f, -1.0f, -1.0f, 0xffffffff },
};

const uint32_t indices[] = {
	0, 1, 2,
	1, 3, 2,
	4, 6, 5,
	5, 6, 7,
	0, 2, 4,
	4, 2, 6,
	1, 5, 3,
	5, 7, 3,
	0, 4, 1,
	4, 5, 1,
	2, 3, 6,
	6, 3, 7,
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
