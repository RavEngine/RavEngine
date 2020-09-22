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

static const Vertex vertices[] =
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

static const uint16_t indices[] = {
	2, 1, 0, // 0
	2, 3, 1,
	5, 6, 4, // 2
	7, 6, 5,
	4, 2, 0, // 4
	6, 2, 4,
	3, 5, 1, // 6
	3, 7, 5,
	1, 4, 0, // 8
	1, 5, 4,
	6, 3, 2, // 10
	7, 3, 6,
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
	
	if(! bgfx::isValid(vertexBuffer) || !bgfx::isValid(indexBuffer)){
		throw runtime_error("Buffers could not be created.");
	}
   
    RegisterAllAlternateTypes();
}

RavEngine::StaticMesh::~StaticMesh()
{
//	bgfx::destroy(vertexBuffer);
//	bgfx::destroy(indexBuffer);
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
