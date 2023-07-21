#pragma once
#include "RGLCommon.hpp"
#include "TextureFormat.hpp"
#include "MTLObjCCompatLayer.hpp"
#include <RGL/Types.hpp>
#include <RGL/Common.hpp>

#define MTL_CHECK(a) {NSError* err = nullptr; a; if(err != nullptr){ NSLog(@"%@",err); assert(false);}}


namespace RGL {
    struct RenderPassConfig;

	void InitMTL(const RGL::InitOptions&);
	void DeinitMTL();

    APPLE_API_TYPE(MTLPixelFormat) rgl2mtlformat(TextureFormat format);
    APPLE_API_TYPE(MTLTextureUsage) rgl2mtlTextureUsage(TextureUsage usage);
    
    RGLRenderPassPtr CreateRenderPassMTL(const RenderPassConfig&);

    APPLE_API_TYPE(MTLCompareFunction) rgl2mtlcomparefunction(RGL::DepthCompareFunction fn);
}
