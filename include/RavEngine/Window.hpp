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
		Window(int width, int height, const std::string_view title);

		void NotifySizeChanged(int width, int height);

		void SetSize(int width, int height);
        
		struct SwapchainResult {
			RGL::ITexture* texture;
			RGL::SwapchainPresentConfig presentConfig;
		};
        void QueueGetNextSwapchainImage(RGL::SwapchainPresentConfig&);
		SwapchainResult BlockGetNextSwapchainImage(const RGL::SwapchainPresentConfig& );

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

		void InitSwapchain(RGLDevicePtr device, RGLCommandQueuePtr mainCommandQueue);


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
        
        /**
         * Set the state of relative mouse mode. If true, the mouse will send events even if outside the application window. If false, the mouse will only send events if inside the application window.
         * @param mode the new state
         */
        void SetRelativeMouseMode(bool mode);
        
        /**
         * @returns the current relative mouse mode
         */
        bool GetRelativeMouseMode();
    private:
        float QueryScaleFactor() const;
	};


}
#endif
