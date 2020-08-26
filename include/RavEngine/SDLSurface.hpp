#pragma once

#include <LLGL/LLGL.h>
#include <LLGL/Platform/NativeHandle.h>

struct SDL_Window;

namespace RavEngine {
	class SDLSurface : public LLGL::Surface {
	public:
		/**
		Create an SDLSurface given a size and a window title
		*/
		SDLSurface(const LLGL::Extent2D& size, const std::string& title);
		~SDLSurface();

		/**
		Get a pointer to the native window
		@param nativeHandle the pointer will be set to this
		@param nativeHandleSize used for validation
		@return true if native handle retrieved
		*/
		bool GetNativeHandle(void* nativeHandle, std::size_t nativeHandleSize) const override;

		/**
		@return the current size
		*/
		LLGL::Extent2D GetContentSize() const override;

		/**
		Adapt for a new video mode. 
		@param videoModeDesc the new video mode
		@return true if successfully update
		@note This will destroy and recreate the window
		*/
		bool AdaptForVideoMode(LLGL::VideoModeDescriptor& videoModeDesc) override;

		/**
		Find the current display for this surface. The window must be 50% or nore on this display.
		*/
		std::unique_ptr<LLGL::Display> FindResidentDisplay() const override;

		/**
		Clear the pixel format
		*/
		void ResetPixelFormat() override;

		/**
		Currently unused.
		*/
		bool ProcessEvents() override { return true; }

		/**
		@return pointer to the SDL window. This is different from the Native Handle.
		*/
		SDL_Window* const getWindowPtr() const{
			return window;
		}

		struct WindowSize {
			unsigned int width = 0, height = 0;
		};

		/**
		@return the drawable size of the window. This takes High DPI into account.
		*/
		WindowSize GetDrawableArea();

	private:
		SDL_Window* createWindow();

		static void* getNativeWindow(SDL_Window*);
#ifdef __APPLE__
		static void* setUpMetalLayer(void*);
		static void* resizeMetalLayer(void* nativeView);
#endif

		SDL_Window* window = nullptr;

		LLGL::Extent2D size;

	};
}