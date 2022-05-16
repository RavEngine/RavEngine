#include "RenderEngine.hpp"
#include "Debug.hpp"
#include "SystemInfo.hpp"

#if XR_AVAILABLE
#if _WIN32
#define XR_USE_GRAPHICS_API_D3D12
#include <d3d12.h>
#endif
//#define XR_USE_GRAPHICS_API_VULKAN
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
static XrDebugUtilsMessengerEXT xr_debug{};	
static XrFormFactor app_config_form = XR_FORM_FACTOR_HEAD_MOUNTED_DISPLAY;		//TODO: make this configurable?
static XrSystemId xr_system_id = XR_NULL_SYSTEM_ID;
static XrViewConfigurationType app_config_view = XR_VIEW_CONFIGURATION_TYPE_PRIMARY_STEREO;
static XrEnvironmentBlendMode xr_blend{};
static XrSession xr_session{};
static constexpr XrPosef xr_pose_identity = { {0,0,0,1}, {0,0,0} };
static XrSpace xr_app_space{};
#endif

void RenderEngine::InitXR() {
#if XR_AVAILABLE
	// ask for extensions available on this system
	const char* ask_extensions[]{
		XR_EXT_DEBUG_UTILS_EXTENSION_NAME	// extra debug utils
	};
	uint32_t ext_count = 0;
	xrEnumerateInstanceExtensionProperties(nullptr, 0, &ext_count, nullptr);
	vector<XrExtensionProperties> xr_extensions(ext_count, { XR_TYPE_EXTENSION_PROPERTIES });
	xrEnumerateInstanceExtensionProperties(nullptr, ext_count, &ext_count, xr_extensions.data());

	vector<const char*>  use_extensions;
	for (auto ask_ext : ask_extensions) {
		for (auto ext : xr_extensions) {
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
	xrCreateInstance(&createInfo, &xr_instance);

	if (xr_instance == nullptr) {
		Debug::Fatal("XR Initialization failed because an OpenXR Runtime was not found.");
	}

	// load extension methods to use
	xrGetInstanceProcAddr(xr_instance, "xrCreateDebugUtilsMessengerEXT", (PFN_xrVoidFunction*)(&ext_xrCreateDebugUtilsMessengerEXT));
	xrGetInstanceProcAddr(xr_instance, "xrDestroyDebugUtilsMessengerEXT", (PFN_xrVoidFunction*)(&ext_xrDestroyDebugUtilsMessengerEXT));

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
	xrGetSystem(xr_instance, &systemInfo, &xr_system_id);

	// check blend modes for this device, and take the first available one
	uint32_t blend_count = 0;
	xrEnumerateEnvironmentBlendModes(xr_instance, xr_system_id, app_config_view, 1, &blend_count, &xr_blend);

	// TODO: force correct GPU? 
	// this sample has code for D3D11 https://github.com/maluoi/OpenXRSamples/blob/49bb33414261130a9ee575b3d5ae8a1d1e1b82de/SingleFileExample/main.cpp#L316

	// create the session
	void* bindingptr = nullptr;
	XrGraphicsBindingD3D12KHR d3dbinding{ XR_TYPE_GRAPHICS_BINDING_D3D12_KHR };
	//XrGraphicsBindingVulkanKHR vkbinding{ XR_TYPE_GRAPHICS_BINDING_VULKAN_KHR };
	switch (bgfx::getRendererType()) {
#ifdef _WIN32
	case bgfx::RendererType::Direct3D12:
	{
		d3dbinding.device = (ID3D12Device*)bgfx::getInternalData()->context;
		d3dbinding.queue = nullptr;	// TODO: setup command queue
		bindingptr = &d3dbinding;
	}
		break;
#endif
	//case bgfx::RendererType::Vulkan:
	{
		//vkbinding.instance =
		//vkbinding.physicaDevice = 
		//vkbinding.device = (VKPhysicalDevice)
		//vkbinding.queueFamilyIndex = 
		//vkbinding.queueIndex = 
		//bindingptr = &vkbinding;
	}
		break;
	default:
		Debug::Fatal("Cannot use API {} with OpenXR",GetApp()->GetRenderEngine().GetCurrentBackendName());
	}

	XrSessionCreateInfo sessionInfo = { XR_TYPE_SESSION_CREATE_INFO };
	sessionInfo.next = &bindingptr;
	sessionInfo.systemId = xr_system_id;
	xrCreateSession(xr_instance, &sessionInfo, &xr_session);
	if (xr_session == nullptr) {
		Debug::Fatal("Could not create XR Session - Device may not be attached or ready");
	}

	// select the reference frame 
	// STAGE is relative to guardian bounds, LOCAL is relative to device starting position
	XrReferenceSpaceCreateInfo ref_space = { XR_TYPE_REFERENCE_SPACE_CREATE_INFO };
	ref_space.poseInReferenceSpace = xr_pose_identity;
	ref_space.referenceSpaceType = XR_REFERENCE_SPACE_TYPE_LOCAL;
	xrCreateReferenceSpace(xr_session, &ref_space, &xr_app_space);

	//TODO: make swap chains
#else
	Debug::Fatal("Cannot initialize XR: Not available on platform {}", SystemInfo::OperatingSystemNameString());
#endif
}