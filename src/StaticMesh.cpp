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

using namespace RavEngine;
using namespace std;
using namespace filament;

//default triangle
static const Vertex TRIANGLE_VERTICES[3] = {
	{{1, 0}, 0xffff0000u},
	{{cos(M_PI * 2 / 3), sin(M_PI * 2 / 3)}, 0xff00ff00u},
	{{cos(M_PI * 4 / 3), sin(M_PI * 4 / 3)}, 0xff0000ffu},
};

static constexpr uint16_t TRIANGLE_INDICES[3] = { 0, 1, 2 };


StaticMesh::StaticMesh() : Component(), material(new Material()) {
	/*vb = {
		{{1, 0}, 0xffff0000u},
		{{cos(M_PI * 2 / 3), sin(M_PI * 2 / 3)}, 0xff00ff00u},
		{{cos(M_PI * 4 / 3), sin(M_PI * 4 / 3)}, 0xff0000ffu},
	};

	ib = { 0, 1, 2 };*/

	//material
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

	auto filamentEngine = RenderEngine::getEngine();
	renderable = utils::EntityManager::get().create();

	filament::Material* material = filament::Material::Builder()
		.package((void*)mat.c_str(), mat.size())
		.build(*filamentEngine);
	MaterialInstance* materialInstance = material->createInstance();

	auto vertexBuffer = VertexBuffer::Builder()
		.vertexCount(3)
		.bufferCount(1)
		.attribute(VertexAttribute::POSITION, 0, VertexBuffer::AttributeType::FLOAT2, 0, 12)
		.attribute(VertexAttribute::COLOR, 0, VertexBuffer::AttributeType::UBYTE4, 8, 12)
		.normalized(VertexAttribute::COLOR)
		.build(*filamentEngine);
	vertexBuffer->setBufferAt(*filamentEngine, 0, VertexBuffer::BufferDescriptor(TRIANGLE_VERTICES, 36, nullptr));

	auto indexBuffer = IndexBuffer::Builder()
		.indexCount(3)
		.bufferType(IndexBuffer::IndexType::USHORT)
		.build(*filamentEngine);

	indexBuffer->setBuffer(*filamentEngine, IndexBuffer::BufferDescriptor(TRIANGLE_INDICES, 6, nullptr));

	RenderableManager::Builder(1)
		.boundingBox({ { -1, -1, -1 }, { 1, 1, 1 } })
		.material(0, material->getDefaultInstance())
		.geometry(0, RenderableManager::PrimitiveType::TRIANGLES, vertexBuffer, indexBuffer, 0, 3)
		.culling(false)
		.receiveShadows(false)
		.castShadows(false)
		.build(*filamentEngine, renderable);
}

void RavEngine::StaticMesh::AddHook(const WeakRef<RavEngine::Entity>& e)
{
	filamentParentToEntity(e, renderable);
}
