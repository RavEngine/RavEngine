#if RGL_DX12_AVAILABLE
#include "D3D12Surface.hpp"

using namespace RGL;

RGLSurfacePtr RGL::CreateD3D12SurfaceFromPlatformData(const void* HWNDptr)
{
	return std::make_shared<SurfaceD3D12>(HWNDptr);
}

RGL::SurfaceD3D12::~SurfaceD3D12()
{
	// nothing to do here
}

#endif
