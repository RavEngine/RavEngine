#pragma once
#define NOMINMAX
#include <RGL/Types.hpp>
#include <RGL/Device.hpp>
#include <RGL/Pipeline.hpp>
#include "D3D12DynamicDescriptorHeap.hpp"
#include <d3d12.h>
#include <dxgi1_6.h>
#include <wrl.h>

#undef CreateSemaphore

namespace D3D12MA {
	struct Allocator;
}


namespace RGL {

	struct CommandQueueD3D12;

	struct DeviceD3D12 : public IDevice, public std::enable_shared_from_this<DeviceD3D12>{

		Microsoft::WRL::ComPtr<ID3D12Device2> device;
		Microsoft::WRL::ComPtr<IDXGIAdapter4> adapter;
		std::shared_ptr<CommandQueueD3D12> internalQueue;
		Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList2> internalCommandList;
		Microsoft::WRL::ComPtr<D3D12MA::Allocator> allocator;
		UINT g_RTVDescriptorHeapSize = 0;

		Microsoft::WRL::ComPtr<ID3D12Fence> deviceRemovedFence;

		Microsoft::WRL::ComPtr<ID3D12CommandSignature> multidrawSignature, multidrawIndexedSignature, dispatchIndirectSignature;

		std::optional<D3D12DynamicDescriptorHeap> RTVHeap, DSVHeap, CBV_SRV_UAVHeap, SamplerHeap;

		DeviceD3D12(decltype(adapter) adapter);
		virtual ~DeviceD3D12();

		void Flush();

		// IDevice
		std::string GetBrandString();
		RGLSwapchainPtr CreateSwapchain(RGLSurfacePtr, RGLCommandQueuePtr, int, int) final;
		RGLPipelineLayoutPtr CreatePipelineLayout(const PipelineLayoutDescriptor&) final;
		RGLRenderPipelinePtr CreateRenderPipeline(const RenderPipelineDescriptor&) final;
		RGLComputePipelinePtr CreateComputePipeline(const ComputePipelineDescriptor&) final;

		RGLShaderLibraryPtr CreateShaderLibraryFromName(const std::string_view& name) final;
		RGLShaderLibraryPtr CreateDefaultShaderLibrary() final;
		RGLShaderLibraryPtr CreateShaderLibraryFromBytes(const std::span<const uint8_t>) final;
		RGLShaderLibraryPtr CreateShaderLibrarySourceCode(const std::string_view, const FromSourceConfig& config) final;
		RGLShaderLibraryPtr CreateShaderLibraryFromPath(const std::filesystem::path&) final;

		RGLBufferPtr CreateBuffer(const BufferConfig&) final;
		RGLTexturePtr CreateTextureWithData(const TextureConfig&, untyped_span) final;
		RGLTexturePtr CreateTexture(const TextureConfig&) final;

		RGLSamplerPtr CreateSampler(const SamplerConfig&) final;

		DeviceData GetDeviceData() final;

		TextureView GetGlobalBindlessTextureHeap() const final;

		RGLCommandQueuePtr CreateCommandQueue(QueueType type) final;
		RGLFencePtr CreateFence(bool preSignaled) final;
		void BlockUntilIdle() final;

		size_t GetTotalVRAM() const final;
		size_t GetCurrentVRAMInUse() const final;
	};

	RGLDevicePtr CreateDefaultDeviceD3D12();
}