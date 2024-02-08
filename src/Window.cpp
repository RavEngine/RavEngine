#if !RVE_SERVER
#include "Window.hpp"
#include <SDL.h>
#include "Debug.hpp"
#include <RGL/RGL.hpp>
#include <RGL/Surface.hpp>
#include <RGL/Device.hpp>
#include <RGL/Swapchain.hpp>
#include <RGL/CommandQueue.hpp>


#if _WIN32 && !_UWP
#include <shtypes.h>
#include <ShellScalingApi.h>
#elif _UWP
#include <winrt/Windows.Graphics.Display.h>
using namespace winrt;
#elif __APPLE__
#include "AppleUtilities.h"
#endif

namespace RavEngine {
	Window::Window(int width, int height, const std::string_view title, RGLDevicePtr device, RGLCommandQueuePtr mainCommandQueue)
	{
		window = SDL_CreateWindow(title.data(), width, height, SDL_WINDOW_RESIZABLE | SDL_WINDOW_HIGH_PIXEL_DENSITY);

		if (window == NULL) {
			Debug::Fatal("Unable to create window: {}", SDL_GetError());
		}


        RGL::CreateSurfaceConfig surfaceConfig{nullptr, 0};
        
#if _UWP
        surfaceConfig.pointer = SDL_GetProperty(SDL_GetWindowProperties(window), SDL_PROP_WINDOW_WINRT_WINDOW_POINTER, NULL);
#elif _WIN32
        HWND hwnd = (HWND)SDL_GetProperty(SDL_GetWindowProperties(window), SDL_PROP_WINDOW_WIN32_HWND_POINTER, NULL);
        surfaceConfig.pointer = &hwnd;
#elif TARGET_OS_IPHONE
        surfaceConfig.pointer = SDL_GetProperty(SDL_GetWindowProperties(window), SDL_PROP_WINDOW_UIKIT_WINDOW_POINTER, NULL);
#elif __APPLE__
        surfaceConfig.pointer = SDL_GetProperty(SDL_GetWindowProperties(window), SDL_PROP_WINDOW_COCOA_WINDOW_POINTER, NULL);
#elif __linux__ && !__ANDROID__
        if (SDL_strcmp(SDL_GetCurrentVideoDriver(), "x11") == 0) {
            Display *xdisplay = (Display *)SDL_GetProperty(SDL_GetWindowProperties(window), SDL_PROP_WINDOW_X11_DISPLAY_POINTER, NULL);
            Window xwindow = (Window)SDL_GetNumberProperty(SDL_GetWindowProperties(window), SDL_PROP_WINDOW_X11_WINDOW_NUMBER, 0);
            surfaceConfig.pointer = xdisplay;
            surfaceConfig.pointer2 = uintptr_t(xwindow);
        } else if (SDL_strcmp(SDL_GetCurrentVideoDriver(), "wayland") == 0) {
            struct wl_display *display = (struct wl_display *)SDL_GetProperty(SDL_GetWindowProperties(window), SDL_PROP_WINDOW_WAYLAND_DISPLAY_POINTER, NULL);
            struct wl_surface *surface = (struct wl_surface *)SDL_GetProperty(SDL_GetWindowProperties(window), SDL_PROP_WINDOW_WAYLAND_SURFACE_POINTER, NULL);
            surfaceConfig.pointer = display;
            surfaceConfig.pointer2 = reinterpret_cast<uintptr_t>(surface);
        }
#elif __ANDROID__
        surfaceConfig.pointer = SDL_GetProperty(SDL_GetWindowProperties(window), SDL_PROP_WINDOW_ANDROID_WINDOW_POINTER, NULL);
#elif __EMSCRIPTEN__
			// noop
#else
#error Unknown platform
#endif
        surface = RGL::CreateSurfaceFromPlatformHandle(surfaceConfig, true);
        
        // re-query the window size because some platforms (like iOS) ignore the passed window size
        SDL_GetWindowSize(window, &width, &height);
        
        windowdims = {width, height};
        currentScaleFactor = QueryScaleFactor();
        auto size = GetSizeInPixels();
		swapchain = device->CreateSwapchain(surface, mainCommandQueue, size.width, size.height);
        NotifySizeChanged(width, height);
		swapchainFence = device->CreateFence(true);
	}
	void Window::NotifySizeChanged(int width, int height)
{
        currentScaleFactor = QueryScaleFactor();
		windowdims = { width, height };
#if TARGET_OS_IPHONE
        //view must be manually sized on iOS
        //also this API takes screen points not pixels
        resizeMetalLayer(metalLayer, windowdims.width, windowdims.height);
#endif
        auto pixelSize = GetSizeInPixels();
        swapchain->Resize(pixelSize.width, pixelSize.height);
    }

	void Window::SetSize(int width, int height)
	{
		SDL_SetWindowSize(window, width, height);
		NotifySizeChanged(width, height);
	}

    dim_t<int> Window::GetSizeInPixels() const{
#if __APPLE__
        float scale = GetDPIScale();
#else
        float scale = win_scalefactor;
#endif
        return {static_cast<int>(windowdims.width * scale), static_cast<int>(windowdims.height * scale)};
    }

    float Window::QueryScaleFactor() const{
# if _WIN32 && !_UWP

        SDL_SysWMinfo wmi;
        SDL_VERSION(&wmi.version);
        if (!SDL_GetWindowWMInfo(window, &wmi)) {
            Debug::Fatal("Cannot get native window information");
        }
        auto monitor = MonitorFromWindow(wmi.info.win.window, MONITOR_DEFAULTTONEAREST);
        DEVICE_SCALE_FACTOR fac;
        if (GetScaleFactorForMonitor(monitor, &fac) == S_OK) {
            return (static_cast<int>(fac) / 100.0);
        }
        else {
            Debug::Fatal("GetScaleFactorForMonitor failed");
        }
#elif _UWP
        auto dinf = winrt::Windows::Graphics::Display::DisplayInformation::GetForCurrentView();
        return static_cast<int32_t>(dinf.ResolutionScale()) / 100.0;
#elif __APPLE__
        // since iOS and macOS do not use OpenGL we cannot use the GL call here
        // instead we derive it by querying display data
        return GetWindowScaleFactor(window);
#endif
    }

	Window::SwapchainResult Window::GetNextSwapchainImage()
	{
		// queue up the next swapchain image as soon as possible, 
		// it will become avaiable in the background
		RGL::SwapchainPresentConfig presentConfig{
		};
		swapchain->GetNextImage(&presentConfig.imageIndex);

		// execute when render fence says its ok
		// did we get the swapchain image yet? if not, block until we do

		swapchainFence->Wait();
		swapchainFence->Reset();

		auto nextimg = swapchain->ImageAtIndex(presentConfig.imageIndex);
		return { nextimg, presentConfig };
	}
	void Window::SetWindowMode(WindowMode mode)
	{
		int flag;
		switch (mode) {
		case WindowMode::Windowed:
			flag = 0;
			break;
		case WindowMode::BorderlessFullscreen:
            // SDL_WINDOW_FULLSCREEN_DESKTOP has been removed
			break;
		case WindowMode::Fullscreen:
			flag = SDL_WINDOW_FULLSCREEN;
			break;
		}
		SDL_SetWindowFullscreen(window, flag);
		
	}
}
#endif
