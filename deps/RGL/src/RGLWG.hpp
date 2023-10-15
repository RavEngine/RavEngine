#pragma once
#include "RGLCommon.hpp"
#include <RGL/Types.hpp>
#include <emscripten/html5_webgpu.h>

namespace RGL{
    extern WGPUInstance instance;

    void InitWebGPU(const RGL::InitOptions&);
	void DeinitWebGPU();
    RGLRenderPassPtr CreateRenderPassWG(const RenderPassConfig&);
}