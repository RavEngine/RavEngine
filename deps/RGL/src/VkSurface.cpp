#if RGL_VK_AVAILABLE
#if _WIN32
#define VK_USE_PLATFORM_WIN32_KHR
#elif __ANDROID__
#define VK_USE_PLATFORM_ANDROID_KHR
#elif __linux__ 
#define VK_USE_PLATFORM_XLIB_KHR
#define VK_USE_PLATFORM_WAYLAND_KHR
#endif

#include "VkSurface.hpp"
#include "RGLVk.hpp"

#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#include <vulkan/vulkan_win32.h>
#elif __linux__ && !__ANDROID__
#include <X11/Xlib.h>
#include <vulkan/vulkan_xlib.h>
#include <wayland-client-core.h>
#include <wayland-util.h>
#include <vulkan/vulkan_wayland.h>
#elif __ANDROID__
#include <vulkan/vulkan_android.h>
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


#elif defined __linux__ && !__ANDROID__
    if (config.isWayland) {
        VkWaylandSurfaceCreateInfoKHR createInfo{
            .sType = VK_STRUCTURE_TYPE_WAYLAND_SURFACE_CREATE_INFO_KHR,
            .pNext = nullptr,
            .flags = 0,
            .display = const_cast<wl_display*>(static_cast<const wl_display*>(config.pointer)),
            .surface = reinterpret_cast<wl_surface*>(config.pointer2)
        };
        VK_CHECK(vkCreateWaylandSurfaceKHR(instance, &createInfo, nullptr, &surface));
    }
    else{
        VkXlibSurfaceCreateInfoKHR createInfo{
               .sType = VK_STRUCTURE_TYPE_XLIB_SURFACE_CREATE_INFO_KHR,
               .pNext = nullptr,
               .flags = 0,
               .dpy = const_cast<Display*>(static_cast<const Display*>(config.pointer)),
               .window = static_cast<Window>(config.pointer2),
        };
        VK_CHECK(vkCreateXlibSurfaceKHR(instance, &createInfo, nullptr, &surface));
    }
#elif __ANDROID__
    VkAndroidSurfaceCreateInfoKHR createInfo{
        .sType = VK_STRUCTURE_TYPE_ANDROID_SURFACE_CREATE_INFO_KHR,
        .pNext = nullptr,
        .flags = 0,
        .window = const_cast<ANativeWindow*>(static_cast<const ANativeWindow*>(config.pointer))
    };
    VK_CHECK(vkCreateAndroidSurfaceKHR(instance, &createInfo, nullptr, &surface));
#endif

    return std::make_shared<SurfaceVk>(surface);
}

SurfaceVk::~SurfaceVk() {
    vkDestroySurfaceKHR(instance, surface, nullptr);
}
#endif
