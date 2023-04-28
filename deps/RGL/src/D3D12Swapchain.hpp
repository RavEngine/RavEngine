#pragma once
#include <RGL/Types.hpp>
#include <RGL/Swapchain.hpp>
#include "D3D12DynamicDescriptorHeap.hpp"
#include "RGLD3D12.hpp"
#include <d3d12.h>
#include <dxgi1_5.h>

namespace RGL {
	struct DeviceD3D12;
	struct SurfaceD3D12;
	struct TextureD3D12;
	struct CommandQueueD3D12;
	
	struct SwapchainD3D12 : public ISwapchain {
		constexpr static uint8_t g_NumFrames = 3;

		ComPtr<IDXGISwapChain4> swapchain;
		const std::shared_ptr<DeviceD3D12> owningDevice;
		ComPtr<ID3D12Resource> backbuffers[g_NumFrames];
		std::vector<TextureD3D12> backbufferTextures;
		D3D12DynamicDescriptorHeap::index_t rtvIndices[g_NumFrames];

		bool tearingSupported = false;
		bool vsync = true;
		bool initialized = false;

		SwapchainD3D12(decltype(owningDevice), std::shared_ptr<SurfaceD3D12>, int width, int height, std::shared_ptr<CommandQueueD3D12> presentQueue);
		void UpdateRenderTargetViews(ComPtr<ID3D12Device2> device,
			ComPtr<IDXGISwapChain4> swapChain, D3D12DynamicDescriptorHeap& descriptorHeap);



		// ISwapchain
		void Resize(uint32_t width, uint32_t height) final;
		void GetNextImage(uint32_t* index) final;
		ITexture* ImageAtIndex(uint32_t index) final;
		void Present(const SwapchainPresentConfig&) final;
		virtual ~SwapchainD3D12();
	};
}