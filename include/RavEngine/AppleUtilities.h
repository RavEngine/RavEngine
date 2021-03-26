#pragma once

/**
 Workaround for deadlock on metal. Manually creates the metal layer.
 @param wnd the Cocoa Window pointer
 @return pointer to the created metal layer.
 */
void *cbSetupMetalLayer(void *wnd);

/**
 Resize a metal layer manually. Required on iOS
 @param ptr the pointer to the CAMetalLayer
 @param width the new width, in pixels
 @param height the new height, in pixels
 */
void resizeMetalLayer(void* ptr, int width, int height);

/**
 SDL opts-out of inertial scrolling on macOS. This function re-enables it.
 */
void enableSmoothScrolling();
