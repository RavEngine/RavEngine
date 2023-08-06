#pragma once
#include <RGL/Types.hpp>
#include <RGL/Swapchain.hpp>

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

		struct SwapchainResult {
			RGL::ITexture* texture;
			RGL::SwapchainPresentConfig presentConfig;
		};
		SwapchainResult GetNextSwapchainImage();

		struct dim {
			int width = 0, height = 0;
		} bufferdims, windowdims;

#ifdef _WIN32
		float win_scalefactor = 1;
#endif

		/**
 @return the High DPI scale factor. Only applicable on macOS.
 */
		constexpr float GetDPIScale() const {
#ifndef _WIN32
			return (float)bufferdims.width / windowdims.width;
#else
			return win_scalefactor;
#endif
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
	};


}