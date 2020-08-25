#include "SDLSurface.hpp"
#include <SDL_syswm.h>
#include <SDL.h>

using namespace RavEngine;

SDLSurface::SDLSurface(const LLGL::Extent2D& size, const std::string& title) : size(size)
{
	window = createWindow();
}

SDLSurface::~SDLSurface()
{
	SDL_DestroyWindow(window);
}

bool SDLSurface::GetNativeHandle(void* nativeHandle, std::size_t nativeHandleSize) const
{
	if (nativeHandleSize == sizeof(LLGL::NativeHandle)) {
		auto handle = reinterpret_cast<LLGL::NativeHandle*>(nativeHandle);
#ifdef _WIN32
		handle->window = (HWND)getNativeWindow(window);
#endif
		return true;
	}
	return false;
}

LLGL::Extent2D SDLSurface::GetContentSize() const
{
	return size;
}

bool SDLSurface::AdaptForVideoMode(LLGL::VideoModeDescriptor& videoModeDesc)
{
	size = videoModeDesc.resolution;
	SDL_SetWindowSize(window, size.width, size.height);
	return true;
}

std::unique_ptr<LLGL::Display> RavEngine::SDLSurface::FindResidentDisplay() const
{
	return std::unique_ptr<LLGL::Display>();
}

void SDLSurface::ResetPixelFormat()
{
	//on windows the only way to reset the internal pixel format of a window is to destroy and re-create
	SDL_DestroyWindow(window);
	window = createWindow();
}

SDL_Window* SDLSurface::createWindow()
{
	size_t sdlflags = SDL_INIT_EVENTS;
	if (!SDL_WasInit(sdlflags)) {
		SDL_Init(sdlflags);
	}

	uint32_t windowFlags = SDL_WINDOW_SHOWN | SDL_WINDOW_ALLOW_HIGHDPI | SDL_WINDOW_RESIZABLE;

	return SDL_CreateWindow("RavEngine", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 800, 480, windowFlags);
}
