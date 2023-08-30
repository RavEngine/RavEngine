#if !RVE_SERVER
#include "Window.hpp"
#include <SDL.h>
#include "Debug.hpp"
#include <SDL_syswm.h>
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
		window = SDL_CreateWindow(title.data(), SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, width, height, SDL_WINDOW_RESIZABLE | SDL_WINDOW_ALLOW_HIGHDPI);

		if (window == NULL) {
			Debug::Fatal("Unable to create window: {}", SDL_GetError());
		}

		SDL_SysWMinfo wmi;
		SDL_VERSION(&wmi.version);
		if (!SDL_GetWindowWMInfo(window, &wmi)) {
			throw std::runtime_error("Cannot get native window information");
		}
		surface = RGL::CreateSurfaceFromPlatformHandle(
#if _UWP
			{ wmi.info.winrt.window },
#elif _WIN32
			{ &wmi.info.win.window },
#elif TARGET_OS_IPHONE
			{ wmi.info.uikit.window },
#elif __APPLE__
			{ wmi.info.cocoa.window },
#elif __linux__
			{ wmi.info.x11.display, wmi.info.x11.window },
#elif __EMSCRIPTEN__
			{ nullptr, nullptr },
#else
#error Unknown platform
#endif
			true
		);
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
			flag = SDL_WINDOW_FULLSCREEN_DESKTOP;
			break;
		case WindowMode::Fullscreen:
			flag = SDL_WINDOW_FULLSCREEN;
			break;
		}
		SDL_SetWindowFullscreen(window, flag);
		
	}
}
#endif
