#pragma once
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
        
        void RefreshBufferDims(int width, int height);

		struct SwapchainResult {
			RGL::ITexture* texture;
			RGL::SwapchainPresentConfig presentConfig;
		};
		SwapchainResult GetNextSwapchainImage();

		dim_t<int> windowdims;
        
        dim_t<int> GetSizeInPixels() const;

#ifdef _WIN32
		constexpr static float win_scalefactor = 1;
#endif

		/**
 @return the High DPI scale factor. Only applicable on macOS.
 */
        float GetDPIScale() const;


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
	};


}
