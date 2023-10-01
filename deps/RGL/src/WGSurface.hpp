#pragma once
#include <RGL/Types.hpp>
#include <RGL/Surface.hpp>
#include <emscripten/html5_webgpu.h>

namespace RGL{
    struct SurfaceWG : public ISurface{
        WGPUSurface surface = nullptr;
		SurfaceWG(const void* pointer);
        virtual ~SurfaceWG();
	};

	RGLSurfacePtr CreateWGSurfaceFromPlatformHandle(const void* pointer);
}