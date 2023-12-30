#if !RVE_SERVER
#include "RenderEngine.hpp"
#include "Utilities.hpp"
#include "Debug.hpp"
#include <RGL/CommandBuffer.hpp>
#include <RGL/Device.hpp>

using namespace RavEngine;
using namespace std;
STATIC(RenderEngine::navMeshPolygon);

void RenderEngine::depthMask(bool state){
    navDebugDepthEnabled = state;
}

void RenderEngine::texture(bool state){
    //TODO: load a checkerboard texture, and bind / unbind it to the shader based on state
}

/**
 Size here is not the number of primitives, it's the pixel size of the primitive, for line and point primitives
 */
void RenderEngine::begin(duDebugDrawPrimitives prim, float size){
    navMeshPolygon.clear();

    //TODO: support navDebugDepthEnabled
    switch(prim){
        case duDebugDrawPrimitives::DU_DRAW_TRIS:
            mainCommandBuffer->BindRenderPipeline(recastTrianglePipeline);
            break;
        case duDebugDrawPrimitives::DU_DRAW_LINES:
            mainCommandBuffer->BindRenderPipeline(recastLinePipeline);
            break;
        case duDebugDrawPrimitives::DU_DRAW_POINTS:
            mainCommandBuffer->BindRenderPipeline(recastPointPipeline);
            break;
        case duDebugDrawPrimitives::DU_DRAW_QUADS:
            Debug::Fatal("Quad rendering mode is not supported");
            break;
    }
}

void RenderEngine::vertex(const float *pos, unsigned int color){
    float uv[2] = {0,0};
    vertex(pos, color,uv);
}

void RenderEngine::vertex(const float* pos, unsigned int color, const float* uv){
    VertexColorUV vert;
    vert.position[0] = pos[0];
    vert.position[1] = pos[1];
    vert.position[2] = pos[2];
    vert.uv[0] = uv[0];
    vert.uv[1] = uv[1];
    vert.color = color;
    navMeshPolygon.push_back(vert);
}

void RenderEngine::vertex(const float x, const float y, const float z, unsigned int color){
    float pos[3] = {x,y,z};
    float uv[2] = {0,0};
    vertex(pos,color,uv);
}

void RenderEngine::vertex(const float x, const float y, const float z, unsigned int color, const float u, const float v){
    float pos[3] = {x,y,z};
    float uv[2] = {u,v};
    vertex(pos,color,uv);
}

void RenderEngine::end(){
    // submit the primitive here
    if (navMeshPolygon.size() == 0){
        return;
    }
	auto size = navMeshPolygon.size() * sizeof(decltype(navMeshPolygon)::value_type);
	assert(size < std::numeric_limits<unsigned int>::max());

    auto vertBuffer = device->CreateBuffer({
        uint32_t(navMeshPolygon.size()),
        {.VertexBuffer = true},
        sizeof(decltype(navMeshPolygon)::value_type),
        RGL::BufferAccess::Private,
        });
    vertBuffer->SetBufferData({ navMeshPolygon.data(), navMeshPolygon.size() * sizeof(decltype(navMeshPolygon)::value_type)});

    mainCommandBuffer->SetVertexBuffer(vertBuffer);
    mainCommandBuffer->SetVertexBytes(currentNavState, 0);
    mainCommandBuffer->Draw(navMeshPolygon.size());

    // trash the buffer now that we're done with it
    gcBuffers.enqueue(vertBuffer);
}
#endif
