#pragma once

/**
 Workaround for deadlock on metal. Manually creates the metal layer.
 @param wnd the Cocoa Window pointer
 @return pointer to the created metal layer.
 */
void *cbSetupMetalLayer(void *wnd);

/**
 SDL opts-out of inertial scrolling on macOS. This function re-enables it.
 */
void enableSmoothScrolling();
