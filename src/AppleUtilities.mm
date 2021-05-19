#include "AppleUtilities.h"
#import <QuartzCore/CAMetalLayer.h>
#import <Metal/Metal.h>
#import <MetalKit/MetalKit.h>
#include <bx/bx.h>
#include <SDL_syswm.h>

#if BX_PLATFORM_OSX
	#import <Cocoa/Cocoa.h>
#elif BX_PLATFORM_IOS
	#import <UIKit/UIKit.h>
#endif

/**
 Workaround for deadlock on metal
 */
void *cbSetupMetalLayer(void *wnd) {
#if BX_PLATFORM_OSX
	NSWindow *window = (NSWindow*)wnd;
	NSView *contentView = [window contentView];
	[contentView setWantsLayer:YES];
	CAMetalLayer *res = [CAMetalLayer layer];
	[contentView setLayer:res];
	return res;
#elif BX_PLATFORM_IOS
	UIWindow* window = (UIWindow*)wnd;
	UIView* contentView = [[window subviews] lastObject];
	
	CAMetalLayer *res = [CAMetalLayer layer];
	res.frame = window.bounds;
	[contentView.layer addSublayer:res];
	return res;
#endif
}

void resizeMetalLayer(void* ptr, int width, int height){
	CAMetalLayer* layer = (CAMetalLayer*)ptr;
	layer.frame = CGRectMake(0, 0, width, height);
}

float GetWindowScaleFactor(void* wmi_vp){
	SDL_SysWMinfo* wmi = (SDL_SysWMinfo*)wmi_vp;
#if BX_PLATFORM_OSX
	NSWindow *nswin = (NSWindow*)wmi->info.cocoa.window;
	return [nswin backingScaleFactor];
#elif BX_PLATFORM_IOS
	return wmi->info.uikit.window.screen.scale;
#endif
}

void enableSmoothScrolling(){
    [[NSUserDefaults standardUserDefaults] setBool: YES forKey: @"AppleMomentumScrollSupported"];
}
