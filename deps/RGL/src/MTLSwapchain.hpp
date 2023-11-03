#pragma once
#include <RGL/Types.hpp>
#include <RGL/Swapchain.hpp>
#include "MTLSurface.hpp"
#include "MTLTexture.hpp"
#include <array>

namespace RGL{
	struct SwapchainMTL : public ISwapchain{
		std::shared_ptr<SurfaceMTL> surface;
		SwapchainMTL(decltype(surface) surface, uint32_t width, uint32_t height) : surface(surface){
            Resize(width,height);
        }
		virtual ~SwapchainMTL() {}
		void Resize(uint32_t width, uint32_t height) final;
        void GetNextImage(uint32_t* index) final;
        ITexture* ImageAtIndex(uint32_t index) final;
        void Present(const SwapchainPresentConfig&) final;
        
        std::array<TextureMTL,3> activeTextures;
        uint32_t idx = 0;
        
        void SetVsyncMode(bool mode) final;
	};
}
