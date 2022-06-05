#include "RenderEngine.hpp"
#include "Debug.hpp"
#include "SystemInfo.hpp"

// WinARM requires this for some reason
#define NTDDI_VERSION NTDDI_WIN7
#define XR_USE_GRAPHICS_API_D3D12

#if XR_AVAILABLE
#define XR_USE_GRAPHICS_API_VULKAN
#include <bgfx/../../3rdparty/khronos/vulkan-local/vulkan_core.h>
#if _WIN32
#include <d3d12.h>
#include <Windows.Foundation.h>
#include <wrl\wrappers\corewrappers.h>
#include <wrl\client.h>
static const GUID IID_ID3D12CommandQueue = { 0x0ec870a6, 0x5d7e, 0x4c22, { 0x8c, 0xfc, 0x5b, 0xaa, 0xe0, 0x76, 0x16, 0xed } }; // TODO: this is defined in bgfx/src/dxgi.cpp - use that one? 
#endif
#include <openxr/openxr.h>
#include <openxr/openxr_platform.h>
#include <bgfx/bgfx.h>
#include <bgfx/platform.h>
#include <GetApp.hpp>
#include <App.hpp>
#endif

// some code adapted from: https://github.com/maluoi/OpenXRSamples/blob/master/SingleFileExample/main.cpp

using namespace RavEngine;
using namespace std;

#if XR_AVAILABLE
static XrInstance xr_instance{};
static PFN_xrCreateDebugUtilsMessengerEXT ext_xrCreateDebugUtilsMessengerEXT = nullptr;
static PFN_xrDestroyDebugUtilsMessengerEXT ext_xrDestroyDebugUtilsMessengerEXT = nullptr;
#ifdef _WIN32
static PFN_xrGetD3D12GraphicsRequirementsKHR ext_xrGetD3D12GraphicsRequirementsKHR = nullptr;
static Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> rtvHeap;
#endif
static PFN_xrGetVulkanGraphicsRequirementsKHR ext_xrGetVulkanGraphicsRequirementsKHR = nullptr;
static XrDebugUtilsMessengerEXT xr_debug{};	
static XrFormFactor app_config_form = XR_FORM_FACTOR_HEAD_MOUNTED_DISPLAY;		//TODO: make this configurable?
static XrSystemId xr_system_id = XR_NULL_SYSTEM_ID;
static XrViewConfigurationType app_config_view = XR_VIEW_CONFIGURATION_TYPE_PRIMARY_STEREO;
static XrEnvironmentBlendMode xr_blend{};
static XrSession xr_session{};
static constexpr XrPosef xr_pose_identity = { {0,0,0,1}, {0,0,0} };
static XrSpace xr_app_space{};

static std::vector<XrSwapchain> swapchains;
#endif
static std::vector<RenderEngine::VRFramebuffer> VRFramebuffers;

void RenderEngine::InitXR() {
#if XR_AVAILABLE
	// ask for extensions available on this system
	const char* ask_extensions[]{
		XR_EXT_DEBUG_UTILS_EXTENSION_NAME,	// extra debug utils
#ifdef _WIN32
		XR_KHR_D3D12_ENABLE_EXTENSION_NAME,
#endif
		XR_KHR_VULKAN_ENABLE_EXTENSION_NAME
	};
	uint32_t ext_count = 0;
	xrEnumerateInstanceExtensionProperties(nullptr, 0, &ext_count, nullptr);
	vector<XrExtensionProperties> xr_extensions(ext_count, { XR_TYPE_EXTENSION_PROPERTIES });
	xrEnumerateInstanceExtensionProperties(nullptr, ext_count, &ext_count, xr_extensions.data());

	vector<const char*>  use_extensions;
	for (auto ask_ext : ask_extensions) {
		for (const auto& ext : xr_extensions) {
			if (strcmp(ask_ext, ext.extensionName) == 0) {
				use_extensions.push_back(ask_ext);
			}
		}
	}
	if (use_extensions.size() != sizeof(ask_extensions) / sizeof(ask_extensions[0])) {
		Debug::Fatal("Cannot initialize XR: Required extension {} is not present", ask_extensions[0]);
	}

	XrInstanceCreateInfo createInfo{ XR_TYPE_INSTANCE_CREATE_INFO };
	createInfo.enabledExtensionCount = use_extensions.size();
	createInfo.enabledExtensionNames = use_extensions.data();
	createInfo.applicationInfo.apiVersion = XR_CURRENT_API_VERSION;
	strcpy(createInfo.applicationInfo.applicationName, "RavEngine XR Application");	// TODO: make this user-configurable
	{
		auto res = xrCreateInstance(&createInfo, &xr_instance);

		if (xr_instance == nullptr || res != XR_SUCCESS) {
			Debug::Fatal("XR Initialization failed because an OpenXR Runtime was not found.");
		}
	}
	

	// load extension methods to use
	xrGetInstanceProcAddr(xr_instance, "xrCreateDebugUtilsMessengerEXT", (PFN_xrVoidFunction*)(&ext_xrCreateDebugUtilsMessengerEXT));
	xrGetInstanceProcAddr(xr_instance, "xrDestroyDebugUtilsMessengerEXT", (PFN_xrVoidFunction*)(&ext_xrDestroyDebugUtilsMessengerEXT));
#ifdef _WIN32
	xrGetInstanceProcAddr(xr_instance, "xrGetD3D12GraphicsRequirementsKHR", (PFN_xrVoidFunction*)(&ext_xrGetD3D12GraphicsRequirementsKHR));
#endif
	xrGetInstanceProcAddr(xr_instance, "xrGetVulkanGraphicsRequirementsKHR", (PFN_xrVoidFunction*)(&ext_xrGetVulkanGraphicsRequirementsKHR));

	// create debug log
	XrDebugUtilsMessengerCreateInfoEXT debug_info = { XR_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT };

	debug_info.messageTypes =
		XR_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT |
		XR_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT |
		XR_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT |
		XR_DEBUG_UTILS_MESSAGE_TYPE_CONFORMANCE_BIT_EXT;
	debug_info.messageSeverities =
		XR_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT |
		XR_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT |
		XR_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT |
		XR_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
	debug_info.userCallback = [](XrDebugUtilsMessageSeverityFlagsEXT severity, XrDebugUtilsMessageTypeFlagsEXT types, const XrDebugUtilsMessengerCallbackDataEXT* msg, void* user_data) {
		
		Debug::Log("[OpenXR] {}: {}", msg->functionName, msg->message);

		// Returning XR_TRUE here will force the calling function to fail
		return (XrBool32)XR_FALSE;
	};
	// start debug utils
	if (ext_xrCreateDebugUtilsMessengerEXT)
		ext_xrCreateDebugUtilsMessengerEXT(xr_instance, &debug_info, &xr_debug);

	// get device form factor
	XrSystemGetInfo systemInfo = { XR_TYPE_SYSTEM_GET_INFO };
	systemInfo.formFactor = app_config_form;
	if (xrGetSystem(xr_instance, &systemInfo, &xr_system_id) != XR_SUCCESS) {
		Debug::Fatal("xrGetSystem Failed");
	}

	// check blend modes for this device, and take the first available one
	uint32_t blend_count = 0;
	xrEnumerateEnvironmentBlendModes(xr_instance, xr_system_id, app_config_view, 1, &blend_count, &xr_blend);

#ifdef _WIN32
	XrGraphicsRequirementsD3D12KHR reqdx = { XR_TYPE_GRAPHICS_REQUIREMENTS_D3D12_KHR };
	ext_xrGetD3D12GraphicsRequirementsKHR(xr_instance, xr_system_id, &reqdx);
#endif
	XrGraphicsRequirementsVulkanKHR reqvk{ XR_TYPE_GRAPHICS_REQUIREMENTS_VULKAN_KHR };
	ext_xrGetVulkanGraphicsRequirementsKHR(xr_instance, xr_system_id, &reqvk);
	//TODO: do something with these req structs (currently ignored, but openXR requires calling these extension methods before proceeding)

	// create the session
	XrSessionCreateInfo sessionInfo = { XR_TYPE_SESSION_CREATE_INFO };
#ifdef _WIN32
	XrGraphicsBindingD3D12KHR d3dbinding{ XR_TYPE_GRAPHICS_BINDING_D3D12_KHR };
#endif
	XrGraphicsBindingVulkanKHR vkbinding{ XR_TYPE_GRAPHICS_BINDING_VULKAN_KHR };
	switch (bgfx::getRendererType()) {
#ifdef _WIN32
	case bgfx::RendererType::Direct3D12:
	{
		const bgfx::InternalData* idata = bgfx::getInternalData();
		auto device = (ID3D12Device*)idata->context;
		d3dbinding.device = device;
		ID3D12CommandQueue* commandQueue = nullptr;
		UINT size = sizeof(commandQueue);
		device->GetPrivateData(IID_ID3D12CommandQueue, &size, &commandQueue);
		
		d3dbinding.queue = commandQueue;	// TODO: setup command queue
		d3dbinding.next = nullptr;
		sessionInfo.next = &d3dbinding;
	}
		break;
#endif
	case bgfx::RendererType::Vulkan:
	{
		const bgfx::InternalData* idata = bgfx::getInternalData();
		//vkbinding.instance =
		//vkbinding.physicaDevice = 
		vkbinding.device = (VkDevice)idata->context;
		//vkbinding.queueFamilyIndex = 
		//vkbinding.queueIndex = 
		sessionInfo.next = &vkbinding;
	}
		break;
	default:
		Debug::Fatal("Cannot use API {} with OpenXR",GetApp()->GetRenderEngine().GetCurrentBackendName());
	}

	sessionInfo.systemId = xr_system_id;
	{
		auto result = xrCreateSession(xr_instance, &sessionInfo, &xr_session);
		if (xr_session == nullptr || result != XR_SUCCESS) {
			Debug::Fatal("Could not create XR Session - Device may not be attached or ready");
		}
	}


	// select the reference frame 
	// STAGE is relative to guardian bounds, LOCAL is relative to device starting position
	XrReferenceSpaceCreateInfo ref_space = { XR_TYPE_REFERENCE_SPACE_CREATE_INFO };
	ref_space.poseInReferenceSpace = xr_pose_identity;
	ref_space.referenceSpaceType = XR_REFERENCE_SPACE_TYPE_LOCAL;
	xrCreateReferenceSpace(xr_session, &ref_space, &xr_app_space);

	// make swap chains
	uint32_t view_count = 0;
	xrEnumerateViewConfigurationViews(xr_instance, xr_system_id, app_config_view, 0, &view_count, nullptr);	// get count
	vector<XrViewConfigurationView> config_views(view_count, { XR_TYPE_VIEW_CONFIGURATION_VIEW });
	xrEnumerateViewConfigurationViews(xr_instance, xr_system_id, app_config_view, view_count, &view_count, config_views.data());	// populate data
	for (uint8_t i = 0; i < view_count; i++) {
		XrViewConfigurationView& view = config_views[i];
		XrSwapchainCreateInfo swapchain_info = { XR_TYPE_SWAPCHAIN_CREATE_INFO };
		XrSwapchain handle{};
		swapchain_info.arraySize = 1;
		swapchain_info.mipCount = 1;
		swapchain_info.faceCount = 1;
#if _WIN32
		if (bgfx::getRendererType() == bgfx::RendererType::Direct3D12) {
			swapchain_info.format = DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;
		}
		else 
#endif
		{
			swapchain_info.format = VK_FORMAT_R8G8B8A8_UNORM;
		}
		swapchain_info.width = view.recommendedImageRectWidth;
		swapchain_info.height = view.recommendedImageRectHeight;
		swapchain_info.sampleCount = view.recommendedSwapchainSampleCount;
		swapchain_info.usageFlags = XR_SWAPCHAIN_USAGE_COLOR_ATTACHMENT_BIT;

		{
			auto result = xrCreateSwapchain(xr_session, &swapchain_info, &handle);
			if (result != XR_SUCCESS) {
				Debug::Fatal("OpenXR Swapchain creation failed: {}", result);
			}
		}
		// get num textures generated from the swapchain
		uint32_t surface_count = 0;
		xrEnumerateSwapchainImages(handle, 0, &surface_count, nullptr);
		const auto genSwapchainData = [&](auto& surface_vec, const auto& surface_datafn) {
			// it creates a triple buffer (why all 3 textures have the same type) - use xrAcquireSwapchainImage each frame to know which one to use
			xrEnumerateSwapchainImages(handle, surface_count, &surface_count, (XrSwapchainImageBaseHeader*)surface_vec.data());
			for (uint32_t i = 0; i < surface_count; i++) {
				// bgfx limitation: no way around not allocating a texture
				auto txhandle = bgfx::createTexture2D(swapchain_info.width, swapchain_info.height,false,1, bgfx::TextureFormat::RGBA32F, BGFX_TEXTURE_RT,nullptr);		

				// call the appropriate function to override the bgfx texture's data with the openXR swapchain
				surface_datafn(txhandle, (XrBaseInStructure&)surface_vec[i]);

				// make a framebuffer and add it to the list
				VRFramebuffers.push_back({ bgfx::createFrameBuffer(1, &txhandle, true), {int(swapchain_info.width), int(swapchain_info.height)} });
			}
		};
#if _WIN32
		if (bgfx::getRendererType() == bgfx::RendererType::Direct3D12) {
			vector<XrSwapchainImageD3D12KHR> images(surface_count, { XR_TYPE_SWAPCHAIN_IMAGE_D3D12_KHR });
			genSwapchainData(images,
				[&](bgfx::TextureHandle& tx, XrBaseInStructure& base) {
					auto& img = (XrSwapchainImageD3D12KHR&)base;
					bgfx::overrideInternal(tx,reinterpret_cast<uintptr_t>(img.texture));
				}
			);
		}
		else
#endif
		{
			vector<XrSwapchainImageVulkanKHR> images(surface_count, { XR_TYPE_SWAPCHAIN_IMAGE_D3D12_KHR });
			genSwapchainData(images,
				[&](bgfx::TextureHandle& tx, XrBaseInStructure& base) {
					auto& img = (XrSwapchainImageVulkanKHR&)base;
					bgfx::overrideInternal(tx, reinterpret_cast<uintptr_t>(img.image));
				}
			);
		}
		swapchains.push_back(handle);
	}
#else
	Debug::Fatal("Cannot initialize XR: Not available on platform {}", SystemInfo::OperatingSystemNameString());
#endif
}

const RenderEngine::BufferedFramebuffer RenderEngine::GetVRFrameBuffers() const{
	BufferedFramebuffer ret;
#if XR_AVAILABLE
	uint32_t start_index = 0;
	uint32_t offset_index = 0;
	xrAcquireSwapchainImage(swapchains[0], nullptr, &offset_index);
	ret.l_eye = VRFramebuffers[start_index + offset_index];
	start_index += 3;
	xrAcquireSwapchainImage(swapchains[1], nullptr, &offset_index);
	ret.r_eye = VRFramebuffers[start_index + offset_index];
#endif
	return ret;
}
