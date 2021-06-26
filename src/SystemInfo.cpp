#include "SystemInfo.hpp"
#include <thread>
#include "Debug.hpp"
#include <bgfx/bgfx.h>
#ifdef _WIN32
#include <PortableDeviceApi.h>
#include <psapi.h>
#include <tchar.h>
#endif

using namespace RavEngine::SystemInfo;
using namespace std;

uint16_t RavEngine::SystemInfo::NumLogicalProcessors()
{
	return thread::hardware_concurrency();
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
