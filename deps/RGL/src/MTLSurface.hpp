#pragma once
#if RGL_MTL_AVAILABLE
#include "MTLObjCCompatLayer.hpp"
#include <RGL/Types.hpp>
#include <RGL/Surface.hpp>

namespace RGL {
	struct SurfaceMTL : public ISurface{
        APPLE_API_PTR(CAMetalLayer) layer;
		SurfaceMTL(decltype(layer) layer ): layer(layer){}
	};

	RGLSurfacePtr CreateMTLSurfaceFromPlatformHandle(void* pointer, bool createSurfaceObject);
}

#endif

