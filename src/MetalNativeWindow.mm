//
//  MetalNativeWindow.m
//  RavEngine
//

#include "RenderEngine.hpp"
#include <SDL_syswm.h>
#import <AppKit/NSView.h>
#import <AppKit/NSWindow.h>
#import <QuartzCore/CAMetalLayer.h>

// On Mac, this is the implementation of getNativeWindow
void* RavEngine::RenderEngine::getNativeWindow(SDL_Window* sdlWindow) {
    SDL_SysWMinfo wmi;
    SDL_VERSION(&wmi.version);
    assert(SDL_GetWindowWMInfo(sdlWindow, &wmi));
    NSWindow* win = wmi.info.cocoa.window;
    NSView* view = [win contentView];
    return view;
}

void* RavEngine::RenderEngine::setUpMetalLayer(void* nativeView) {
    NSView* view = (NSView*) nativeView;
    [view setWantsLayer:YES];
    CAMetalLayer* metalLayer = [CAMetalLayer layer];
    metalLayer.bounds = view.bounds;

    // It's important to set the drawableSize to the actual backing pixels. When rendering
    // full-screen, we can skip the macOS compositor if the size matches the display size.
    metalLayer.drawableSize = [view convertSizeToBacking:view.bounds.size];

    // In its implementation of vkGetPhysicalDeviceSurfaceCapabilitiesKHR, MoltenVK takes into
    // consideration both the size (in points) of the bounds, and the contentsScale of the
    // CAMetalLayer from which the Vulkan surface was created.
    // See also https://github.com/KhronosGroup/MoltenVK/issues/428
    metalLayer.contentsScale = view.window.backingScaleFactor;

    // This is set to NO by default, but is also important to ensure we can bypass the compositor
    // in full-screen mode
    // See "Direct to Display" http://metalkit.org/2017/06/30/introducing-metal-2.html.
    metalLayer.opaque = YES;

    [view setLayer:metalLayer];

    return metalLayer;
}

void* RavEngine::RenderEngine::resizeMetalLayer(void* nativeView) {
    NSView* view = (NSView*) nativeView;
    CAMetalLayer* metalLayer = (CAMetalLayer*) view.layer;
    CGSize viewSize = view.bounds.size;
    NSSize newDrawableSize = [view convertSizeToBacking:view.bounds.size];
    metalLayer.drawableSize = newDrawableSize;
    metalLayer.contentsScale = view.window.backingScaleFactor;
    return metalLayer;
}
