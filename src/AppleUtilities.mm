#include "AppleUtilities.h"
#import <QuartzCore/CAMetalLayer.h>
#import <Metal/Metal.h>
#import <MetalKit/MetalKit.h>
#include <bx/bx.h>
#include <SDL_syswm.h>
#include "Debug.hpp"
#include "SystemInfo.hpp"
#include <sys/utsname.h>
#include <bgfx/platform.h>

#if BX_PLATFORM_OSX
	#import <Cocoa/Cocoa.h>
#elif BX_PLATFORM_IOS
	#import <UIKit/UIKit.h>
#endif
#include <sys/types.h>
#include <sys/sysctl.h>
#include <mach/machine.h>

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

float GetWindowScaleFactor(void* window){
	SDL_SysWMinfo wmi;
	SDL_VERSION(&wmi.version);
	if (!SDL_GetWindowWMInfo((SDL_Window*)window, &wmi)) {
		RavEngine::Debug::Fatal("Cannot get native window information");
	}
#if BX_PLATFORM_OSX
	NSWindow *nswin = (NSWindow*)wmi.info.cocoa.window;
	return [nswin backingScaleFactor];
#elif BX_PLATFORM_IOS
	return wmi.info.uikit.window.screen.scale;
#endif
}

void enableSmoothScrolling(){
    [[NSUserDefaults standardUserDefaults] setBool: YES forKey: @"AppleMomentumScrollSupported"];
}

void AppleGPUName(char* buffer, size_t size){
	auto internalData = bgfx::getInternalData();
	auto device = (id<MTLDevice>)internalData->context;
	auto name = [device name];
	
	std::memcpy(buffer, [name UTF8String], std::min(size,[name length]));
}

uint32_t AppleVRAMUsed(){
    auto internalData = bgfx::getInternalData();
    auto device = (id<MTLDevice>)internalData->context;
    return static_cast<uint32_t>([device currentAllocatedSize] / 1024 / 1024);
}

uint32_t AppleVRAMTotal(){
#if BX_PLATFORM_IOS
    return GetAppleSystemRAM();
#else
    auto internalData = bgfx::getInternalData();
    auto device = (id<MTLDevice>)internalData->context;
    return [device recommendedMaxWorkingSetSize] / 1024 / 1024;
#endif
}

AppleOSVersion GetAppleOSVersion(){
    auto p = [[NSProcessInfo processInfo] operatingSystemVersion];

    AppleOSVersion vers;
    vers.major = p.majorVersion;
    vers.minor = p.minorVersion;
    vers.patch = p.patchVersion;
    return vers;
}

uint32_t GetAppleSystemRAM(){
    auto v = [[NSProcessInfo processInfo] physicalMemory];
    return static_cast<uint32_t>(v/1024/1024);
}

void AppleOSName(char* buffer, uint16_t size){
    auto name =
#if TARGET_OS_IOS
    "iOS";
#elif TARGET_OS_TV
    "tvOS";
#elif TARGET_OS_OSX
    "macOS";
#else
    #error This Apple platform is not supported
#endif
    memcpy(buffer, name, strnlen(name, size));
}

void AppleCPUName(char* buffer, size_t size){
	if constexpr (RavEngine::SystemInfo::IsMobile()){
		utsname sysinfo;
		uname(&sysinfo);
		//NSString* devName = [[UIDevice currentDevice] modelName];
		//const char* ptr = [devName UTF8String];
		memcpy(buffer, sysinfo.machine, strnlen(sysinfo.machine, size));
	}
	else{
		sysctlbyname("machdep.cpu.brand_string", buffer, &size, NULL, 0);
	}
}
