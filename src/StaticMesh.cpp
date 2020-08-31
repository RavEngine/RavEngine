#include "StaticMesh.hpp"
#include "Entity.hpp"
#include "mathtypes.hpp"
#include <SDL_stdinc.h>
#include <fstream>
#include <sstream>
#include "LLGL/LLGL.h"
#include "RenderEngine.hpp"

using namespace RavEngine;
using namespace std;

// Vertex data structure
struct Vertex
{
    float      position[3];
    LLGL::ColorRGBf   color;
};

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
    // Vertex format
    LLGL::VertexFormat vertexFormat;

    // Append 2D float vector for position attribute
    vertexFormat.AppendAttribute({ "position", LLGL::Format::RGB32Float });

    // Append 3D unsigned byte vector for color
    vertexFormat.AppendAttribute({ "color",    LLGL::Format::RGB32Float });

    // Update stride in case our vertex structure is not 4-byte aligned
    vertexFormat.SetStride(sizeof(Vertex));

    // Create vertex buffer
    LLGL::BufferDescriptor vertexBufferDesc;

    vertexBufferDesc.size = sizeof(vertices);                 // Size (in bytes) of the vertex buffer
    vertexBufferDesc.bindFlags = LLGL::BindFlags::VertexBuffer;    // Enables the buffer to be bound to a vertex buffer slot
    vertexBufferDesc.vertexAttribs = vertexFormat.attributes;          // Vertex format layout

    vertexBuffer = RenderEngine::GetRenderSystem()->CreateBuffer(vertexBufferDesc, vertices);

    LLGL::BufferDescriptor indexBufferDesc;
    indexBufferDesc.size = sizeof(indices);
    indexBufferDesc.bindFlags = LLGL::BindFlags::IndexBuffer;
    indexBufferDesc.format = LLGL::Format::R32UInt;
    indexBuffer = RenderEngine::GetRenderSystem()->CreateBuffer(indexBufferDesc, indices);

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

void RavEngine::StaticMesh::Draw(LLGL::CommandBuffer* commands)
{
    //skip draw if no material assigned
    if (material.isNull()) {
        return;
    }
    //apply transform and set it for the material
    auto owner = Ref<Entity>(getOwner());
    owner->transform()->Apply();
    material->SetTransformMatrix(owner->transform()->GetCurrentWorldMatrix());
    material->Draw(commands, vertexBuffer, indexBuffer);
}

void RavEngine::StaticMesh::AddHook(const WeakRef<RavEngine::Entity>& e)
{
}
