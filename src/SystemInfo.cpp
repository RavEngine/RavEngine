#include "SystemInfo.hpp"
#include "App.hpp"
#include <thread>
#include "Debug.hpp"
#include <cstring>
#include "Defines.hpp"
#if !RVE_SERVER

#include "RenderEngine.hpp"
#include <RGL/Device.hpp>
#endif

#ifdef _WIN32
    #include <PortableDeviceApi.h>
    #include <psapi.h>
    #include <tchar.h>
    #include <sysinfoapi.h>
    #include <d3d12.h>
    #include <dxgi1_4.h>
    #include <locale>
    #include <codecvt>
#elif defined __APPLE__
    #include "AppleUtilities.h"
#elif defined __linux__
    #include <sys/utsname.h>
    #include <sys/sysinfo.h>
    #include <fstream>
#endif

#if _UWP
#include <winrt/Windows.System.Diagnostics.h>
#include <winrt/Windows.System.Profile.h>
using namespace winrt;
#endif

using namespace RavEngine;
using namespace std;
using namespace RavEngine::SystemInfo;

static SystemInfo::Features SystemFeatures;

SystemInfo::Features::Features()
#if TARGET_OS_OSX
: HasTouchScreen(false)		// maybe one day there will be touch support on macOS, but currently there is not
#endif
{

}

static const SystemInfo::Features& getSupportedFeatures(){
	return SystemFeatures;
}

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
#ifndef _M_ARM64
    int CPUInfo[4] = { -1 };
    unsigned nExIds, i = 0;
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
#else
    return "Mystery WinARM64 CPU";
#endif
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
    auto vi = winrt::Windows::System::Profile::AnalyticsInfo::VersionInfo();
    return Format("UWP {}",to_string(vi.DeviceFamily()));
#elif _WIN32
    return "Windows WIN32";
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
#elif _UWP
    auto vi = winrt::Windows::System::Profile::AnalyticsInfo::VersionInfo();
    auto str = to_string(vi.DeviceFamilyVersion());
    auto asInt = std::stoull(str);
    // convert this int into the four-part version
    vers.major = (asInt & 0xFFFF000000000000) >> 48;
    vers.minor = (asInt & 0xFFFF00000000) >> 32;
    vers.patch = (asInt & 0xFFFF0000) >> 16;
    vers.extra = (asInt & 0xFFFF);
#elif _WIN32
    NTSTATUS(WINAPI * RtlGetVersion)(LPOSVERSIONINFOEXW);
    OSVERSIONINFOEXW osInfo;
    *(FARPROC*)&RtlGetVersion = GetProcAddress(GetModuleHandleA("ntdll"), "RtlGetVersion");
    if (NULL != RtlGetVersion)
    {
        osInfo.dwOSVersionInfoSize = sizeof(osInfo);
        RtlGetVersion(&osInfo);
        vers.major = osInfo.dwMajorVersion;
        vers.minor = osInfo.dwMinorVersion;
        vers.patch = osInfo.dwBuildNumber;
    }
#elif __linux__
    utsname s;
    uname(&s);
    int major = 0, minor = 0, patch = 0, extra = 0;
    sscanf(s.release, "%d.%d.%d-%d",&major,&minor,&patch,&extra);
	vers.major = major;
	vers.minor = minor;
	vers.patch = patch;
	vers.extra = extra;
#endif
    return vers;
}

uint32_t SystemInfo::SystemRAM(){
#ifdef __APPLE__
    return GetAppleSystemRAM();
#elif _UWP
    auto pk = winrt::Windows::System::Diagnostics::SystemDiagnosticInfo::GetForCurrentSystem();
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

#if !RVE_SERVER

std::string SystemInfo::GPUBrandString(){
    return GetApp()->GetDevice()->GetBrandString();
}

uint32_t SystemInfo::GPUVRAM(){
    return GetApp()->GetRenderEngine().GetTotalVRAM();
}

uint32_t SystemInfo::GPUVRAMinUse(){
    return GetApp()->GetRenderEngine().GetCurrentVRAMUse();
}
#endif

SystemInfo::GPUFeatures SystemInfo::GetSupportedGPUFeatures(){
    SystemInfo::GPUFeatures features;
#if 0
    auto caps = bgfx::getCaps()->supported;
    features.DrawIndirect = caps & BGFX_CAPS_DRAW_INDIRECT;
    features.HDR10 = caps & BGFX_CAPS_HDR10;
    features.OcclusionQuery = caps & BGFX_CAPS_OCCLUSION_QUERY;
    features.DMA = caps & BGFX_CAPS_TEXTURE_DIRECT_ACCESS;
    features.Readback = caps & BGFX_CAPS_TEXTURE_READ_BACK;
    features.HIDPI = caps & BGFX_CAPS_HIDPI;
    features.Uint10Attribute = caps & BGFX_CAPS_VERTEX_ATTRIB_UINT10;
    features.HalfAttribute = caps & BGFX_CAPS_VERTEX_ATTRIB_HALF;
#endif
    return features;
}
#if !RVE_SERVER

SystemInfo::PCIDevice RavEngine::SystemInfo::GPUPCIData()
{
#if 0
    SystemInfo::PCIDevice dev{ bgfx::getCaps()->vendorId,bgfx::getCaps()->deviceId };
    return dev;
#endif
    return {};
}
#endif
