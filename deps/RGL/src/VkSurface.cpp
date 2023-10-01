#if RGL_VK_AVAILABLE
#include "VkSurface.hpp"
#include "RGLVk.hpp"

#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#include <vulkan/vulkan_win32.h>
#elif __linux__
#include <X11/Xlib.h>
#include <vulkan/vulkan_xlib.h>
#endif

using namespace RGL;

RGLSurfacePtr RGL::CreateVKSurfaceFromPlatformData(const CreateSurfaceConfig& config)
{
    VkSurfaceKHR surface;
#ifdef _WIN32
    auto hwnd = *static_cast<const HWND*>(config.pointer);
    VkWin32SurfaceCreateInfoKHR createInfo{
        .sType = VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR,
         .hinstance = GetModuleHandle(nullptr),
        .hwnd = hwnd,
    };
    VK_CHECK(vkCreateWin32SurfaceKHR(instance, &createInfo, nullptr, &surface));

   
#elif defined __linux__
    VkXlibSurfaceCreateInfoKHR createInfo{
           .sType = VK_STRUCTURE_TYPE_XLIB_SURFACE_CREATE_INFO_KHR,
           .pNext = nullptr,
           .flags = 0,
           .dpy = static_cast<Display*>(config.pointer),
           .window = static_cast<Window>(config.pointer2),
    };
    VK_CHECK(vkCreateXlibSurfaceKHR(instance, &createInfo, nullptr, &surface));
#endif

    return std::make_shared<SurfaceVk>(surface);
}

SurfaceVk::~SurfaceVk() {
    vkDestroySurfaceKHR(instance, surface, nullptr);
}
#endif
