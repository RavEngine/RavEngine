//
//  VulkanNativeWindow.cpp
//  RavEngine
//
//

#include "RenderEngine.hpp"
#include <SDL_syswm.h>

#ifndef __APPLE__
void* RenderEngine::getNativeWindow(SDL_Window* sdlWindow) {
#endif
    
#ifdef _WIN32
    //Windows implementation
    SDL_SysWMinfo wmi;
    SDL_VERSION(&wmi.version);
    assert(SDL_GetWindowWMInfo(window, &wmi));
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

