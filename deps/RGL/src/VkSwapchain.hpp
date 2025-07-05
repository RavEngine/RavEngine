#pragma once
#include <RGL/Types.hpp>
#include <RGL/Swapchain.hpp>
#include "VkDevice.hpp"
#include "VkSurface.hpp"
#include "VkTexture.hpp"
#include <vector>

namespace RGL {
	struct SwapchainVK : public ISwapchain {
		std::shared_ptr<RGL::DeviceVk> owningDevice;
		std::shared_ptr<SurfaceVk> owningSurface;
		VkSwapchainKHR swapChain = VK_NULL_HANDLE;
		VkSemaphore imageAvailableSemaphore = VK_NULL_HANDLE, renderCompleteSemaphore = VK_NULL_HANDLE;
		VkFence internalFence = VK_NULL_HANDLE;
		std::vector<VkImage> swapChainImages;
		std::vector<VkImageView> swapChainImageViews;
		std::vector<TextureVk> RGLTextureResources;
		VkExtent2D swapChainExtent;
		VkFormat swapChainImageFormat;

		virtual ~SwapchainVK();
		SwapchainVK(decltype(owningSurface), decltype(owningDevice), int width, int height);
		void Resize(uint32_t, uint32_t) final;
		void GetNextImage(uint32_t* index) final;
		ITexture* ImageAtIndex(uint32_t index) final {
			return &(RGLTextureResources[index]);
		}

		void Present(const SwapchainPresentConfig&) final;

		void SetVsyncMode(bool mode) final;

	private:
		bool vsync = true;
		void DestroySwapchainIfNeeded();
	};
}