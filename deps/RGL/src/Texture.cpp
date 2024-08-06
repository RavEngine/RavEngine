#include "Texture.hpp"
#include "RGL.hpp"
#if RGL_VK_AVAILABLE
#include "VkTexture.hpp"
#endif
#if RGL_MTL_AVAILABLE
#include "MTLTexture.hpp"
#endif
#include <stdexcept>

namespace RGL {
	uint32_t TextureView::GetReadonlyBindlessTextureHandle() {
		auto backend = RGL::CurrentAPI();
		switch (backend) {
#if RGL_DX12_AVAILABLE
		case API::Direct3D12:
			return texture.dx.srvIDX;
#endif
#if RGL_VK_AVAILABLE
		case API::Vulkan:
			return static_cast<const TextureVk*>(parent)->globalDescriptorIndex;
#endif
#if RGL_MTL_AVAILABLE
        case API::Metal:
            return static_cast<const TextureMTL*>(texture.mtl.texture)->globalIndex;
#endif
		default:
			throw std::runtime_error("Current backend does not support bindless texturing.");
		}
		return 0;
	}
}
