#pragma once
#include <RGL/Types.hpp>
#include <RGL/Swapchain.hpp>
#include "WGSurface.hpp"
#include "WGTexture.hpp"
#include <array>

namespace RGL{
    struct DeviceWG;
	struct SwapchainWG : public ISwapchain{
        WGPUSwapChain swapchain;
		std::shared_ptr<SurfaceWG> surface;
        const std::shared_ptr<DeviceWG> owningDevice;
		SwapchainWG(decltype(surface) surface, uint32_t width, uint32_t height, const std::shared_ptr<DeviceWG> owningDevice);
		virtual ~SwapchainWG();
		void Resize(uint32_t width, uint32_t height) final;
        void GetNextImage(uint32_t* index) final;
        ITexture* ImageAtIndex(uint32_t index) final;
        void Present(const SwapchainPresentConfig&) final;

        Dimension currentSize;
        
        std::array<TextureWG,3> activeTextures;
        uint32_t idx = 0;

        void SetVsyncMode(bool mode) final;
    private:
        WGPUSwapChain makeSwapchain(uint32_t width, uint32_t height);
        bool vsync = true;
	};
}
