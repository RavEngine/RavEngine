#pragma once

#if __has_include(<openxr/openxr.h>)
#define RVE_XR_AVAILABLE
#include "RenderTargetCollection.hpp"

#if RGL_VK_AVAILABLE
#define XR_USE_GRAPHICS_API_VULKAN
#include <vulkan/vulkan.h>
#include <RGL/../../src/VkTexture.hpp>
#endif
#if _WIN32
#define XR_USE_GRAPHICS_API_D3D12
#include <RGL/../../src/D3D12Texture.hpp>
#include <d3d12.h>
#endif

#include <openxr/openxr.h>
#include <openxr/openxr_platform.h>
#endif

namespace RavEngine {
#ifdef RVE_XR_AVAILABLE
	namespace OpenXRIntegration {
		struct {
			// required for all XR apps
			XrInstance instance;
			XrSession session;
			XrSystemId systemId;
			XrSessionState sessionState = XR_SESSION_STATE_UNKNOWN;

			// place space (usually local or stage)
			XrSpace space;

			// the configs for each display / eye
			std::vector<XrViewConfigurationView> viewConfigurationViews;
			std::vector <XrCompositionLayerProjectionView> projectionViews;
			std::vector<XrView> views;

			union XrSwapchainImage {
#if RGL_DX12_AVAILABLE
				XrSwapchainImageD3D12KHR d3d12Image;
#endif
#if RGL_VK_AVAILABLE
				XrSwapchainImageVulkanKHR vkImage;
#endif
			};

			// one list of images per eye
			std::vector<std::vector<XrSwapchainImage>> swapchainImages;
			std::vector<std::vector<std::unique_ptr<RGL::ITexture>>> rglSwapchainImages;
			std::vector<std::vector<XrSwapchainImage>> depthSwapchainImages;
			std::vector<std::vector<RGLTexturePtr>> rglDepthSwapchainImages;
			// one swapchain per eye, plus depth
			int64_t swapchain_format, depth_swapchain_format = -1;
			std::vector<XrSwapchain> swapchains;
			std::vector<XrSwapchain> depth_swapchains;

			// dpeth layer data
			struct
			{
				std::vector<XrCompositionLayerDepthInfoKHR> infos;
				bool supported;
			} depth;

			struct
			{
				std::vector<XrSwapchainImage> images;
				int64_t format;
				uint32_t swapchain_width, swapchain_height;
				uint32_t swapchain_length;
				XrSwapchain swapchain;
				bool supported;
			} cylinder;

			union {
#if RGL_DX12_AVAILABLE
				XrGraphicsBindingD3D12KHR d3d12Binding;
#endif
#if RGL_VK_AVAILABLE
				XrGraphicsBindingVulkan2KHR vkBinding;
#endif
			} graphicsBinding;

			XrSessionState state = XR_SESSION_STATE_UNKNOWN;

			XrDebugUtilsMessengerEXT debugMessenger;
			XrViewConfigurationType viewConfigurationType;

			inline static PFN_xrCreateDebugUtilsMessengerEXT ext_xrCreateDebugUtilsMessengerEXT = nullptr;
			inline static PFN_xrDestroyDebugUtilsMessengerEXT ext_xrDestroyDebugUtilsMessengerEXT = nullptr;

			static constexpr XrPosef identity_pose{
				.orientation = { .x = 0, .y = 0, .z = 0, .w = 1.0 },
					.position = { .x = 0, .y = 0, .z = 0 }
			};

		} xr;

		struct OpenXRInitInfo {
			RGLDevicePtr device;
			RGLCommandQueuePtr commandQueue;
		};
		void init_openxr(const OpenXRInitInfo& initInfo);

		std::vector<RenderViewCollection> CreateRenderTargetCollections();

		void UpdateXRTargetCollections(std::vector<RenderViewCollection>& collections, const std::vector<XrView>& views);
		std::pair<std::vector<XrView>, XrFrameState> BeginXRFrame();
		void EndXRFrame(XrFrameState& frameState);
	};
#endif
}