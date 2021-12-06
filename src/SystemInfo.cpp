#include "SystemInfo.hpp"
#include <thread>
#include "Debug.hpp"
#include <bgfx/bgfx.h>
#ifdef _WIN32
    #include <PortableDeviceApi.h>
    #include <psapi.h>
    #include <tchar.h>
#elif defined __APPLE__
    #include "AppleUtilities.h"
#endif
#include "App.hpp"

using namespace RavEngine;
using namespace std;
using namespace RavEngine::SystemInfo;

uint16_t RavEngine::SystemInfo::NumLogicalProcessors()
{
	return thread::hardware_concurrency();
}

std::string SystemInfo::CPUBrandString(){
#ifdef __APPLE__
    char buf[100]{0};
    AppleCPUName(buf, sizeof(buf));
    return buf;
#endif
    return "Generic CPU Device";
}

std::string SystemInfo::OperatingSystemNameString(){
#ifdef __APPLE__
    char buf[8]{0};
    AppleOSName(buf, sizeof(buf));
    return buf;
#endif
    return "Unknown OS";
}

SystemInfo::OSVersion SystemInfo::OperatingSystemVersion(){
    OSVersion vers;
#ifdef __APPLE__
    auto v = GetAppleOSVersion();
    vers.major = v.major;
    vers.minor = v.minor;
    vers.patch = v.patch;
#endif
    return vers;
}

uint32_t SystemInfo::SystemRAM(){
#ifdef __APPLE__
    return GetAppleSystemRAM();
#endif
    return 0;
}

uint32_t SystemInfo::GPUVRAM(){
    return App::GetRenderEngine().GetTotalVRAM();
}

uint32_t SystemInfo::GPUVRAMinUse(){
    return App::GetRenderEngine().GetCurrentVRAMUse();
}

SystemInfo::GPUFeatures SystemInfo::GetSupportedGPUFeatures(){
    SystemInfo::GPUFeatures features;
    auto caps = bgfx::getCaps()->supported;
    features.features[SystemInfo::GPUFeatures::Features::iDrawIndirect] = caps & BGFX_CAPS_DRAW_INDIRECT;
    features.features[SystemInfo::GPUFeatures::Features::iHDR10] = caps & BGFX_CAPS_HDR10;
    features.features[GPUFeatures::Features::iOcclusionQuery] = caps & BGFX_CAPS_OCCLUSION_QUERY;
    features.features[GPUFeatures::Features::iDMA] = caps & BGFX_CAPS_TEXTURE_DIRECT_ACCESS;
    features.features[GPUFeatures::Features::iReadback] = caps & BGFX_CAPS_TEXTURE_READ_BACK;
    features.features[GPUFeatures::Features::iHIDPI] = caps & BGFX_CAPS_HIDPI;
    features.features[GPUFeatures::Features::iUint10Attribute] = caps & BGFX_CAPS_VERTEX_ATTRIB_UINT10;
    features.features[GPUFeatures::Features::iHalfAttribute] = caps & BGFX_CAPS_VERTEX_ATTRIB_HALF;
}

std::string RavEngine::SystemInfo::GPUBrandString()
{
    auto device_id = bgfx::getCaps()->deviceId;
#ifdef _WIN32
    return "Generic GPU Device - Windows";
#elif defined __APPLE__
	return "Generic GPU Device - Apple";
#elif defined __linux__
	return "Generic GPU Device - Linux";
#else
	return "Generic GPU Device";
#endif
}
