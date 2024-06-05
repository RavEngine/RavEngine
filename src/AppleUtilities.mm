#include "AppleUtilities.h"
#if !RVE_SERVER
    #import <QuartzCore/CAMetalLayer.h>
    #import <Metal/Metal.h>
    #include <SDL3/SDL.h>
    #include <RGL/RGL.hpp>
    #include <RGL/Device.hpp>
#else
    #include <Foundation/NSProcessInfo.h>
#endif
#include "Debug.hpp"
#include <sys/utsname.h>


#if TARGET_OS_IOS || TARGET_OS_TV || TARGET_OS_VISION
#define TARGET_OS_NONOSX 1
#else
#define TARGET_OS_NONOSX 0
#endif

#if TARGET_OS_OSX
	#import <Cocoa/Cocoa.h>
#elif TARGET_OS_NONOSX
	#import <UIKit/UIKit.h>
#endif
#include <sys/types.h>
#include <sys/sysctl.h>
#include <mach/machine.h>
#import <Foundation/NSAutoreleasePool.h>

//thread_local NSAutoreleasePool *pool;

void AppleAutoreleasePoolInit(){
    //pool = [[NSAutoreleasePool alloc] init];
}

void AppleAutoreleasePoolDrain(){
    //[pool drain];
}

#if !RVE_SERVER
void resizeMetalLayer(void* ptr, int width, int height){
	// on mac, auto resizing mask takes care of this
#if TARGET_OS_NONOSX
    CAMetalLayer* layer = (__bridge CAMetalLayer*)ptr;
	layer.frame = CGRectMake(0, 0, width, height);
#endif
}
float GetWindowScaleFactor(void* window){
#if TARGET_OS_OSX
    NSWindow *nswin = (__bridge NSWindow *)SDL_GetProperty(SDL_GetWindowProperties((SDL_Window*)window), SDL_PROP_WINDOW_COCOA_WINDOW_POINTER, NULL);
	return [nswin backingScaleFactor];
#elif TARGET_OS_NONOSX
#if TARGET_OS_VISION
    return 1;   // visionOS does not have DPI scale
#else
    UIWindow* uiwin = (__bridge UIWindow*)SDL_GetProperty(SDL_GetWindowProperties((SDL_Window*)window), SDL_PROP_WINDOW_UIKIT_WINDOW_POINTER, NULL);
	return uiwin.screen.scale;
#endif
#endif
}

void enableSmoothScrolling(){
    [[NSUserDefaults standardUserDefaults] setBool: YES forKey: @"AppleMomentumScrollSupported"];
}
#endif
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
#elif TARGET_OS_VISION
    "visionOS";
#else
    #error This Apple platform is not supported
#endif
    memcpy(buffer, name, strnlen(name, size));
}

void AppleCPUName(char* buffer, size_t size){
	if constexpr (TARGET_OS_IOS){
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

#if !RVE_SERVER
bool AppleGPUMeetsMinSpec(RGLDevicePtr deviceWrapper){
    auto internalData = deviceWrapper->GetDeviceData();
    
    id<MTLDevice> device = (__bridge id<MTLDevice>)internalData.mtlData.device;
#if TARGET_OS_SIMULATOR
    return true;        // simulator does not properly report features, so assume it will work
#elif TARGET_OS_IPHONE
    if (@available(iOS 13.0, *)) {
        return [device supportsFamily:MTLGPUFamilyApple2];
    } else {
        return false;   // too old!
    };
#elif TARGET_OS_MAC
    if (@available(macOS 10.15, *)) {
        return [device supportsFamily:MTLGPUFamilyMac2];
    } else {
        return false;   // too old
    };
#endif
}
#endif
