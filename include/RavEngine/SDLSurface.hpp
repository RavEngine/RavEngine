#pragma once

#include <LLGL/LLGL.h>
#include <LLGL/Platform/NativeHandle.h>

struct SDL_Window;

namespace RavEngine {
	class SDLSurface : public LLGL::Surface {
	public:
		SDLSurface(const LLGL::Extent2D& size, const std::string& title);
		~SDLSurface();

		bool GetNativeHandle(void* nativeHandle, std::size_t nativeHandleSize) const override;
		LLGL::Extent2D GetContentSize() const override;

		bool AdaptForVideoMode(LLGL::VideoModeDescriptor& videoModeDesc) override;

		std::unique_ptr<LLGL::Display> FindResidentDisplay() const override;

		void ResetPixelFormat() override;

		bool ProcessEvents() override { return true; }

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