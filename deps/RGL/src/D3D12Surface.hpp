#pragma once
#define NOMINMAX
#include <RGL/Types.hpp>
#include <RGL/Surface.hpp>

#define WIN32_LEAN_AND_MEAN
#include <Windows.h>

namespace RGL {

	struct SurfaceD3D12 : public ISurface {
		const void* windowHandle;
		SurfaceD3D12(decltype(windowHandle) handle) : windowHandle(handle) {}
		~SurfaceD3D12();
	};

	RGLSurfacePtr CreateD3D12SurfaceFromPlatformData(const void*);
}