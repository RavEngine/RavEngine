#pragma once
#include <cstdint>
#include "TextureFormat.hpp"

#if RGL_VK_AVAILABLE
#include <vulkan/vulkan.h>
#endif
#if RGL_DX12_AVAILABLE
#include <limits>
#endif

#if RGL_WEBGPU_AVAILABLE
#include <emscripten/html5_webgpu.h>
#endif

#if __APPLE__
#include <objc/objc.h>

#ifdef __OBJC__
#define OBJC_ID(a) id< a >
#define APPLE_API_PTR(a) a *
#define APPLE_API_TYPE(a) a
#else
#define OBJC_ID(a) id
#define APPLE_API_PTR(a) void *
#define APPLE_API_TYPE(a) int
#endif
#endif


namespace RGL {

	struct Dimension {
		uint32_t width = 0, height = 0;
	};

	enum class TextureType : uint8_t {
		T1D,
		T2D,
		T3D,
	};

	enum class TilingMode : uint8_t {
		Optimal,
		Linear
	};
#if RGL_DX12_AVAILABLE
	struct TextureD3D12;
#endif

	struct TextureView {
#if RGL_VK_AVAILABLE || RGL_DX12_AVAILABLE
		Dimension viewSize{ 0,0 };
#endif
#if RGL_MTL_AVAILABLE
		struct
#else
		union
#endif
			NativeHandles {
#if RGL_MTL_AVAILABLE
			OBJC_ID(MTLTexture) mtl;
#endif
#if RGL_WEBGPU_AVAILABLE
			WGPUTextureView wg;
			NativeHandles(decltype(wg) wg) : wg(wg) {}
#endif
#if RGL_DX12_AVAILABLE
			struct {
				constexpr static uint32_t unallocated = std::numeric_limits<uint32_t>::max();
				uint32_t
					dsvIDX = unallocated,
					rtvIDX = unallocated,
					srvIDX = unallocated,
					uavIDX = unallocated;

				bool dsvAllocated() const {
					return dsvIDX != unallocated;
				}
				bool rtvAllocated() const {
					return rtvIDX != unallocated;
				}
				bool srvAllocated() const {
					return srvIDX != unallocated;
				}
				bool uavAllocated() const {
					return uavIDX != unallocated;
				}
				const TextureD3D12* parentResource;
			} dx;
			NativeHandles(const decltype(dx)& dx) : dx(dx) {}
#endif
#if RGL_VK_AVAILABLE
			struct vk {
				VkImageView view;
				uint32_t mip = 0;
				constexpr static decltype(mip) ALL_MIPS = std::numeric_limits<decltype(mip)>::max();
			}
			vk;
			NativeHandles(decltype(vk.view) view, decltype(vk.mip) mip) : vk{ view, mip } {}
#endif
			NativeHandles() {}

		} texture;

#if RGL_VK_AVAILABLE
		const RGL::ITexture* parent = nullptr;
		TextureView(decltype(parent) parent, VkImageView in_img, uint32_t mip, Dimension dim) : parent(parent), viewSize(dim), texture(in_img, mip) {}
#endif

#if RGL_DX12_AVAILABLE
		TextureView(decltype(texture.dx) dx) : texture(dx) {}
#endif

#if RGL_WEBGPU_AVAILABLE
		TextureView(decltype(texture.wg) wg) : texture(wg) {}
#endif

#if RGL_MTL_AVAILABLE
		TextureView(id tx) {
			texture.mtl = tx;
		}
#endif
		TextureView() {}
	};

	struct TextureConfig {
		TextureUsage usage;
		TextureAspect aspect;
		uint32_t width = 0, height = 0, depth = 1, mipLevels = 1, arrayLayers = 1;
		TextureType imageType = decltype(imageType)::T2D;
		TextureFormat format;
		TilingMode mode = decltype(mode)::Optimal;
		ResourceLayout initialLayout = ResourceLayout::Undefined;
		const char* debugName = nullptr;
		bool readbackEnabled = false;
	};

	class ITexture {
	protected:
		Dimension size;
	public:
		ITexture(decltype(size) size) : size(size) {}
		virtual Dimension GetSize() const = 0;
		virtual TextureView GetDefaultView() const = 0;
		virtual TextureView GetViewForMip(uint32_t mip) const = 0;
	};

}