#include "AppleUtilities.h"
#import <QuartzCore/CAMetalLayer.h>
#import <Metal/Metal.h>
#import <MetalKit/MetalKit.h>
#include <bx/bx.h>

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
	[contentView.layer setBackgroundColor:UIColor.yellowColor.CGColor];
	
	CAMetalLayer *res = [CAMetalLayer layer];
	res.frame = window.bounds;
	[contentView.layer addSublayer:res];
	return res;
#endif
}

void enableSmoothScrolling(){
    [[NSUserDefaults standardUserDefaults] setBool: YES forKey: @"AppleMomentumScrollSupported"];
}
