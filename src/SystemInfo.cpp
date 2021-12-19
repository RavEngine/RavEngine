#include "SystemInfo.hpp"
#include "App.hpp"
#include <thread>
#include "Debug.hpp"
#include <bgfx/bgfx.h>
#include <bx/bx.h>

#if defined(WINAPI_FAMILY) && (WINAPI_FAMILY == WINAPI_FAMILY_APP)
    #define _UWP 1   
#else
    #define _UWP 0
#endif

#ifdef _WIN32
    #include <PortableDeviceApi.h>
    #include <psapi.h>
    #include <tchar.h>
    #include <sysinfoapi.h>
#elif defined __APPLE__
    #include "AppleUtilities.h"
#elif defined __linux__
    #include <sys/utsname.h>
    #include <sys/sysinfo.h>
    #include <fstream>
#endif

#if _UWP
#include <winrt/Windows.System.Diagnostics.h>
using namespace winrt;
#endif

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
#elif _WIN32
    int CPUInfo[4] = { -1 };
    unsigned   nExIds, i = 0;
    char CPUBrandString[0x40]{0};
    // Get the information associated with each extended ID.
    __cpuid(CPUInfo, 0x80000000);
    nExIds = CPUInfo[0];
    for (i = 0x80000000; i <= nExIds; ++i)
    {
        __cpuid(CPUInfo, i);
        // Interpret CPU brand string
        switch (i) {
        case 0x80000002:
            memcpy(CPUBrandString, CPUInfo, sizeof(CPUInfo));
            break;
        case 0x80000003:
            memcpy(CPUBrandString + 16, CPUInfo, sizeof(CPUInfo));
            break;
        case 0x80000004:
            memcpy(CPUBrandString + 32, CPUInfo, sizeof(CPUInfo));
            break;
        }
    }
    return string(CPUBrandString,64);
#elif __linux__
	ifstream in("/proc/cpuinfo");
	string line;
	while (in >> line && line != "model");
	in >> line >> line >> line >> line >> line;
	getline(in,line);
	return line;
#endif
    return "Unknown CPU";
}

std::string SystemInfo::OperatingSystemNameString(){
#ifdef __APPLE__
    char buf[8]{0};
    AppleOSName(buf, sizeof(buf));
    return buf;
#elif _UWP
    return "Windows-UWP";
#elif _WIN32
    return "Windows";
#elif __linux__
    ifstream in("/etc/os-release");
    if (!in.is_open()){
    	return "Linux-Unknown";
    }
    string str;
    while(getline(in,str,'=') && str != "PRETTY_NAME"){
    	getline(in,str);
    }
    char c;
    in >> c;
    getline(in,str,'"');
    return str;
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
#elif _WIN32
    // microsoft doesn't seem to have a working API for this
    // so everything is "10"
    vers.major = 10;
#elif __linux__
    utsname s;
    uname(&s);
    sscanf(s.release, "%u.%u.%u-%u",&vers.major,&vers.minor,&vers.patch,&vers.extra);

#endif
    return vers;
}

uint32_t SystemInfo::SystemRAM(){
#ifdef __APPLE__
    return GetAppleSystemRAM();
#elif _UWP
    auto pk = Windows::System::Diagnostics::SystemDiagnosticInfo::GetForCurrentSystem();
    auto bytes = pk.MemoryUsage().GetReport().TotalPhysicalSizeInBytes();
    return bytes / 1024 / 1024;
#elif _WIN32
    ULONGLONG memKB;
    GetPhysicallyInstalledSystemMemory(&memKB);
    return memKB / 1024;
#elif __linux__
	struct sysinfo info;
	sysinfo(&info);
	return info.totalram / 1024 / 1024;
#endif
    return 0;
}

uint32_t SystemInfo::GPUVRAM(){
    return GetApp()->GetRenderEngine().GetTotalVRAM();
}

uint32_t SystemInfo::GPUVRAMinUse(){
    return GetApp()->GetRenderEngine().GetCurrentVRAMUse();
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
    return features;
}

SystemInfo::PCIDevice RavEngine::SystemInfo::GPUPCIData()
{
    SystemInfo::PCIDevice dev{ bgfx::getCaps()->vendorId,bgfx::getCaps()->deviceId };
    return dev;
}
