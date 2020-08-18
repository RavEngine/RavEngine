#include "StaticMesh.hpp"
#include "Entity.hpp"
#include "mathtypes.hpp"
#include <SDL_stdinc.h>
#include <fstream>
#include <sstream>
#include <filament/Material.h>
#include <filament/VertexBuffer.h>
#include <filament/IndexBuffer.h>
#include <filament/RenderableManager.h>
#include "RenderEngine.hpp"
#include <utils/EntityManager.h>
#include "filament/Engine.h"

using namespace RavEngine;
using namespace std;
using namespace filament;

//default triangle
static const filament::math::float3 vertices[] = {
		{ -1, -1,  1},  // 0. left bottom far
		{  1, -1,  1},  // 1. right bottom far
		{ -1,  1,  1},  // 2. left top far
		{  1,  1,  1},  // 3. right top far
		{ -1, -1, -1},  // 4. left bottom near
		{  1, -1, -1},  // 5. right bottom near
		{ -1,  1, -1},  // 6. left top near
		{  1,  1, -1} }; // 7. right top near

static constexpr uint32_t indices[] = {
	// solid
	2,0,1, 2,1,3,  // far
	6,4,5, 6,5,7,  // near
	2,0,4, 2,4,6,  // left
	3,1,5, 3,5,7,  // right
	0,4,5, 0,5,1,  // bottom
	2,6,7, 2,7,3,  // top

	// wire-frame
	0,1, 1,3, 3,2, 2,0,     // far
	4,5, 5,7, 7,6, 6,4,     // near
	0,4, 1,5, 3,7, 2,6,
};

filament::Material* material = nullptr;
MaterialInstance* materialInstance = nullptr;
void initMat() {
	string mat;
	{
		auto path = "../deps/filament/filament/generated/material/defaultMaterial.filamat";
#ifdef _WIN32
		path += 3;
#endif
		ifstream fin(path, ios::binary);
		assert(fin.good());	//ensure file exists
		ostringstream buffer;
		buffer << fin.rdbuf();
		mat = buffer.str();
	}

	material = filament::Material::Builder()
		.package((void*)mat.c_str(), mat.size())
		.build(*RenderEngine::getEngine());
	materialInstance = material->createInstance();
}


StaticMesh::StaticMesh() : Component(), material(new Material()) {
	/*vb = {
		{{1, 0}, 0xffff0000u},
		{{cos(M_PI * 2 / 3), sin(M_PI * 2 / 3)}, 0xff00ff00u},
		{{cos(M_PI * 4 / 3), sin(M_PI * 4 / 3)}, 0xff0000ffu},
	};

	ib = { 0, 1, 2 };*/

	//material
	if (materialInstance == nullptr) {
		initMat();
	}


	auto filamentEngine = RenderEngine::getEngine();
	renderable = utils::EntityManager::get().create();

	auto vertexBuffer = VertexBuffer::Builder()
		.vertexCount(8)
		.bufferCount(1)
		.attribute(VertexAttribute::POSITION, 0, VertexBuffer::AttributeType::FLOAT3)
		.build(*filamentEngine);

	auto indexBuffer = IndexBuffer::Builder()
		.indexCount(12 * 2 + 3 * 2 * 6)
		.build(*filamentEngine);

	vertexBuffer->setBufferAt(*filamentEngine, 0,
		VertexBuffer::BufferDescriptor(
			vertices, vertexBuffer->getVertexCount() * sizeof(vertices[0])));

	indexBuffer->setBuffer(*filamentEngine,
		IndexBuffer::BufferDescriptor(
			indices, indexBuffer->getIndexCount() * sizeof(uint32_t)));

	RenderableManager::Builder(1)
		.boundingBox({ { 0, 0, 0 },
					  { 1, 1, 1 } })
		.material(0, materialInstance)
		.geometry(0, RenderableManager::PrimitiveType::LINES, vertexBuffer, indexBuffer, 0, 3 * 2 * 6)
		.priority(7)
		.culling(true)
		.build(*filamentEngine, renderable);
}

RavEngine::StaticMesh::~StaticMesh()
{
	auto engine = RenderEngine::getEngine();
	engine->destroy(fvb);
	engine->destroy(fib);
	engine->destroy(renderable);
}

void RavEngine::StaticMesh::AddHook(const WeakRef<RavEngine::Entity>& e)
{
	filamentParentToEntity(e, renderable);
}
