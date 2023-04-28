#if RGL_MTL_AVAILABLE
#include <QuartzCore/CAMetalLayer.h>
#include "MTLSurface.hpp"
#import <Metal/Metal.h>

#include <TargetConditionals.h>
#if TARGET_OS_OSX
#include <Cocoa/Cocoa.h>
#elif TARGET_OS_IPHONE
#include <UIKit/UIKit.h>
#else
#endif


RGLSurfacePtr CreateMTLSurfaceFromLayer(CAMetalLayer* layer){
	return std::make_shared<RGL::SurfaceMTL>(layer);
}

RGLSurfacePtr RGL::CreateMTLSurfaceFromPlatformHandle(void* pointer, bool createSurfaceObject){
	if (createSurfaceObject){
#if TARGET_OS_OSX
        NSWindow* window = (__bridge NSWindow*)(pointer);
		NSView *contentView = [window contentView];
		[contentView setWantsLayer:YES];
		CAMetalLayer *res = [CAMetalLayer layer];
		[contentView setLayer:res];
		[res setAutoresizingMask:NSViewWidthSizable | NSViewHeightSizable];
        return CreateMTLSurfaceFromLayer(res);
#elif TARGET_OS_IPHONE
        UIWindow* window = (__bridge UIWindow*)pointer;
		UIView* contentView = [[window subviews] lastObject];
		
		CAMetalLayer *res = [CAMetalLayer layer];
		res.frame = window.bounds;
		[contentView.layer addSublayer:res];
		res.needsDisplayOnBoundsChange = true;
        return CreateMTLSurfaceFromLayer(res);
#endif
	}
	
}
#endif
