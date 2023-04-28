#pragma once
#include <RGL/Texture.hpp>
#include <RGL/Synchronization.hpp>
#include <span>
#include <memory>

namespace RGL{
	struct SwapchainPresentConfig {
		uint32_t imageIndex = 0;
	};
	struct ISwapchain{
		virtual ~ISwapchain() {}
		virtual void Resize(uint32_t width, uint32_t height) = 0;
		virtual void GetNextImage(uint32_t* index) = 0;
		virtual ITexture* ImageAtIndex(uint32_t index) = 0;
		virtual void Present(const SwapchainPresentConfig&) = 0;
	};
}
