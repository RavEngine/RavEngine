#pragma once
#include <RGL/Types.hpp>
#include <RGL/Texture.hpp>
#include <RGL/Span.hpp>
#include <vulkan/vulkan.h>
#include <vk_mem_alloc.h>
#include <memory>
#include <vector>
#include <string>

namespace RGL {
	struct DeviceVk;
	struct TextureVk : public ITexture, public std::enable_shared_from_this<TextureVk> {
		VkImageView vkImageView = VK_NULL_HANDLE;
		VkImage vkImage = VK_NULL_HANDLE;
		struct SwapchainVK* owningSwapchain = nullptr;	// will remain null if the texture is not created by a swapchain
		const std::shared_ptr<DeviceVk> owningDevice;
		bool owning = false;
		TextureVk(decltype(owningDevice), decltype(vkImageView) imageView, decltype(vkImage) image, const Dimension& size);
		TextureVk(decltype(owningDevice), const TextureConfig&, untyped_span bytes);
		TextureVk(decltype(owningDevice), const TextureConfig&);
		Dimension GetSize() const final;
		virtual ~TextureVk();

		const TextureConfig createdConfig;
		VkImageAspectFlags createdAspectVk = VK_IMAGE_ASPECT_COLOR_BIT;
		VkFormat format;

		VkImageLayout nativeFormat = VK_IMAGE_LAYOUT_UNDEFINED;

		VmaAllocation alloc = VK_NULL_HANDLE;

		TextureView GetDefaultView() const final;
		TextureView GetViewForMip(uint32_t mip) const final;

		RGLCustomTextureViewPtr MakeCustomTextureView(const CustomTextureViewConfig& config) const;

		std::vector<TextureView> mipViews;

		std::string debugName;
	};

	struct CustomTextureViewVk : public ICustomTextureView {
		const std::shared_ptr<const TextureVk> owningTexture = nullptr;
		const CustomTextureViewConfig config;
		CustomTextureViewVk(decltype(owningTexture) owning, const CustomTextureViewConfig& config);

		VkImageView imageView = VK_NULL_HANDLE;

		~CustomTextureViewVk();

		TextureView GetView() const;
	};
}