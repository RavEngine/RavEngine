#pragma once
#if !RVE_SERVER
#include <RGL/Types.hpp>
#include <RGL/Swapchain.hpp>
#include <string_view>
#include "mathtypes.hpp"

struct SDL_Window;

namespace RavEngine {
	struct Window {
		SDL_Window* window;
		RGLSwapchainPtr swapchain;
		RGLSurfacePtr surface;
		RGLFencePtr swapchainFence;
		void* metalLayer;
		Window(int width, int height, const std::string_view title, RGLDevicePtr device, RGLCommandQueuePtr mainCommandQueue);

		void NotifySizeChanged(int width, int height);

		void SetSize(int width, int height);
        
		struct SwapchainResult {
			RGL::ITexture* texture;
			RGL::SwapchainPresentConfig presentConfig;
		};
		SwapchainResult GetNextSwapchainImage();

		dim_t<int> windowdims;
        
        dim_t<int> GetSizeInPixels() const;

		constexpr static float win_scalefactor = 1;
        float currentScaleFactor = win_scalefactor;

		/**
 @return the High DPI scale factor. Only applicable on macOS.
 */
        float GetDPIScale() const{
            return currentScaleFactor;
        }


		enum class WindowMode {
			Windowed,
			BorderlessFullscreen,
			Fullscreen
		};
		/**
		 Set the video mode
		 @param mode the new mode to use
		 */
		void SetWindowMode(WindowMode mode);
    private:
        float QueryScaleFactor() const;
	};


}
#endif
