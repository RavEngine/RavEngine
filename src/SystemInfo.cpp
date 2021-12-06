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

using namespace RavEngine;
using namespace std;

uint16_t RavEngine::SystemInfo::NumLogicalProcessors()
{
	return thread::hardware_concurrency();
}

std::string SystemInfo::CPUBrandString(){
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
    return 0;
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
