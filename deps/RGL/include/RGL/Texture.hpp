#pragma once
#include "Types.hpp"
#include "Span.hpp"
#include <cstdint>
#include <limits>
#include <string_view>
#include <array>
#include "TextureFormat.hpp"
#include "SubresourceRange.hpp"

#if RGL_VK_AVAILABLE
#include <volk.h>
#endif

#if RGL_WEBGPU_AVAILABLE
#include <emscripten/html5_webgpu.h>
#endif

namespace RGL {

#if RGL_MTL_AVAILABLE
    struct TextureMTL;
#endif

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
            struct mtl_t {
                const TextureMTL* texture = nullptr;
                uint32_t mip = 0;
                constexpr static decltype(mip) ALL_MIPS = std::numeric_limits<decltype(mip)>::max();
                bool representsBindless = false;
            } mtl;
#endif
#if RGL_WEBGPU_AVAILABLE
			WGPUTextureView wg;
			NativeHandles(decltype(wg) wg) : wg(wg) {}
#endif
#if RGL_DX12_AVAILABLE
			struct dx {
				constexpr static uint32_t unallocated = std::numeric_limits<uint32_t>::max();
				uint32_t
					dsvIDX = unallocated,
					rtvIDX = unallocated,
					srvIDX = unallocated,
					uavIDX = unallocated;
				
				bool representsBindless = false;

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
				covered_mips_t coveredMips = 0;
				covered_layers_t coveredLayers = 0;
			} dx;
			NativeHandles(const decltype(dx)& dx) : dx(dx) {}
#endif
#if RGL_VK_AVAILABLE
			struct vk {
				VkImageView view = VK_NULL_HANDLE;
				covered_mips_t coveredMips = 0;
				covered_layers_t coveredLayers = 0;
				struct bindlessInfo {
					VkDescriptorSet bindlessSet = NULL;

				} bindlessInfo;
			}
			vk;
			NativeHandles(decltype(vk.view) view, decltype(vk.coveredMips) mips, decltype(vk.coveredLayers) layers) : vk{ view, mips, layers } {}
			NativeHandles(decltype(vk.bindlessInfo) set) : vk{ .bindlessInfo = set } {}
#endif

			NativeHandles() {}

		} texture;

#if RGL_VK_AVAILABLE
		const RGL::ITexture* parent = nullptr;
		TextureView(decltype(parent) parent, VkImageView in_img, covered_mips_t mips, covered_layers_t layers, Dimension dim) : parent(parent), viewSize(dim), texture(in_img, mips, layers) {}
		TextureView(decltype(texture.vk.bindlessInfo) bindless) : texture(bindless) {}
#endif

#if RGL_DX12_AVAILABLE
		TextureView(decltype(texture.dx) dx) : texture(dx) {}
#endif

#if RGL_WEBGPU_AVAILABLE
		TextureView(decltype(texture.wg) wg) : texture(wg) {}
#endif

#if RGL_MTL_AVAILABLE
		TextureView(const decltype(texture.mtl)& inTexture) {
			texture.mtl = inTexture;
		}
#endif
		TextureView() {}

		uint32_t GetReadonlyBindlessTextureHandle();
	};


	struct TextureConfig {
		TextureUsage usage;
		TextureAspect aspect;
		uint32_t width = 0, height = 0, depth = 1, mipLevels = 1, arrayLayers = 1;
		std::array<float, 4> optimizedClearValue{ 0,0,0,0 };
		TextureType imageType = decltype(imageType)::T2D;
		TextureFormat format;
		TilingMode mode = decltype(mode)::Optimal;
		ResourceLayout initialLayout = ResourceLayout::Undefined;
		bool isCubemap = false;
		bool readbackEnabled = false;
        std::string_view debugName;
	};

	class ICustomTextureView {

		virtual TextureView GetView() const = 0;
	};

	struct CustomTextureViewConfig {
		uint32_t mip = 0;
		uint32_t layer = 0;
	};

	struct TextureUploadData {
		untyped_span data{nullptr, 0};
		TextureUploadData(const decltype(data)& data) : data(data) {}
		TextureUploadData() {};
	};

	class ITexture {
	protected:
		Dimension size;
	public:
		ITexture(decltype(size) size) : size(size) {}
		virtual Dimension GetSize() const = 0;
		virtual TextureView GetDefaultView() const = 0;
		virtual TextureView GetViewForMip(uint32_t mip) const = 0;
		virtual uint8_t GetNumMips() const = 0;
		virtual RGLCustomTextureViewPtr MakeCustomTextureView(const CustomTextureViewConfig& config) const = 0;
	};

}
