//
//  VulkanNativeWindow.cpp
//  RavEngine
//
//

#include "SDLSurface.hpp"
#include <cassert>
#include <SDL_syswm.h>

using namespace RavEngine;

#ifndef __APPLE__
void* SDLSurface::getNativeWindow(SDL_Window* sdlWindow) {
#endif
    
#ifdef _WIN32
    //Windows implementation
    SDL_SysWMinfo wmi;
    SDL_VERSION(&wmi.version);
    assert(SDL_GetWindowWMInfo(sdlWindow, &wmi));
    //HDC nativeWindow = (HDC)wmi.info.win.hdc;
    HWND nativeWindow = wmi.info.win.window;
    return (void*)nativeWindow;
}
#endif

#ifdef __linux__
    SDL_SysWMinfo wmi;
    SDL_VERSION(&wmi.version);
    assert(SDL_GetWindowWMInfo(sdlWindow, &wmi));
    Window win = (Window) wmi.info.x11.window;
    return (void*) win;
}
#endif

