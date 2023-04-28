#include <RGL/Surface.hpp>
#include <RGL/Core.hpp>
#include "RGLCommon.hpp"

#if RGL_VK_AVAILABLE
#include "VkSurface.hpp"
#endif

#if RGL_DX12_AVAILABLE
#include "D3D12Surface.hpp"
#endif

#if RGL_MTL_AVAILABLE
#include "MTLSurface.hpp"
#endif

using namespace RGL;

RGLSurfacePtr RGL::CreateSurfaceFromPlatformHandle(const CreateSurfaceConfig& pointer, bool createSurfaceObject)
{
    switch (CurrentAPI()) {
#if RGL_MTL_AVAILABLE
    case API::Metal:
        return CreateMTLSurfaceFromPlatformHandle(pointer.pointer, createSurfaceObject);
#endif
#if RGL_VK_AVAILABLE
    case API::Vulkan:
        return CreateVKSurfaceFromPlatformData(pointer);
#endif
#if RGL_DX12_AVAILABLE
    case API::Direct3D12:
        return CreateD3D12SurfaceFromPlatformData(pointer.pointer);
#endif
    default:
        FatalError("Invalid API");
    }

    return RGLSurfacePtr{};
}
