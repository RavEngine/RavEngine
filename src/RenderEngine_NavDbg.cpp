#include "RenderEngine.hpp"

using namespace RavEngine;
using namespace std;

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
    currentPrimitive = prim;
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
  
    constexpr auto common = BGFX_STATE_WRITE_RGB | BGFX_STATE_WRITE_A | BGFX_STATE_DEPTH_TEST_LESS | BGFX_STATE_CULL_CW | BGFX_STATE_BLEND_FUNC(BGFX_STATE_BLEND_SRC_ALPHA, BGFX_STATE_BLEND_INV_DST_ALPHA) ;
    
    switch(currentPrimitive){
        case duDebugDrawPrimitives::DU_DRAW_TRIS:
            bgfx::setState(common | BGFX_STATE_PT_TRISTRIP | (navDebugDepthEnabled ? BGFX_STATE_WRITE_Z : BGFX_STATE_NONE));
            break;
        case duDebugDrawPrimitives::DU_DRAW_LINES:
            bgfx::setState(common | BGFX_STATE_PT_LINES | (navDebugDepthEnabled ? BGFX_STATE_WRITE_Z : BGFX_STATE_NONE));
            break;
        case duDebugDrawPrimitives::DU_DRAW_POINTS:
            bgfx::setState(common | BGFX_STATE_PT_POINTS | (navDebugDepthEnabled ? BGFX_STATE_WRITE_Z : BGFX_STATE_NONE));
            break;
        case duDebugDrawPrimitives::DU_DRAW_QUADS:
            Debug::Fatal("Quad rendering mode is not supported");
            break;
    }
    
    auto memory = bgfx::copy(navMeshPolygon.data(), navMeshPolygon.size() * sizeof(decltype(navMeshPolygon)::value_type));
    auto vb = bgfx::createVertexBuffer(memory, debugNavMeshLayout);
    
    bgfx::setVertexBuffer(0, vb);
    bgfx::submit(Views::FinalBlit, debugNavProgram);
    bgfx::destroy(vb);
    navMeshPolygon.clear();
}
