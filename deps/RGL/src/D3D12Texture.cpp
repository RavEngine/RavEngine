#if RGL_DX12_AVAILABLE
#include "D3D12Texture.hpp"
#include <directx/d3dx12.h>
#include "D3D12Device.hpp"
#include "D3D12CommandQueue.hpp"
#include <D3D12MemAlloc.h>

using namespace Microsoft::WRL;

namespace RGL {

	DXGI_FORMAT typelessForDS(DXGI_FORMAT format) {
		switch (format) {
		case DXGI_FORMAT_D32_FLOAT:
			return DXGI_FORMAT_R32_TYPELESS;
		case DXGI_FORMAT_D24_UNORM_S8_UINT:
			return DXGI_FORMAT_R24G8_TYPELESS;
		default:
			return format;	// otherwise just passthrough
		}
	}

	DXGI_FORMAT typelessForSRV(DXGI_FORMAT format) {
		switch (format) {
		case DXGI_FORMAT_D32_FLOAT:
			return DXGI_FORMAT_R32_FLOAT;
		default:
			return format;	//otherwise just passthrough
		}
	}

	TextureD3D12::TextureD3D12(decltype(texture) image, const Dimension& size, decltype(rtvIDX) offset, decltype(owningDevice) device) : texture(image), ITexture(size), rtvIDX(offset), owningDevice(device)
	{

	}
	TextureD3D12::TextureD3D12(decltype(texture) image, const TextureConfig& config, std::shared_ptr<IDevice> indevice) : owningDevice(std::static_pointer_cast<DeviceD3D12>(indevice)), ITexture({config.width, config.height}), texture(image)
	{
		// make the heap and SRV 
		bool canBeshadervisible = config.usage.Sampled;
		auto format = rgl2dxgiformat_texture(config.format);
		PlaceInHeaps(owningDevice, format, config);
	}
	TextureD3D12::TextureD3D12(decltype(owningDevice) owningDevice, const TextureConfig& config, untyped_span bytes) : TextureD3D12(owningDevice, config)
	{
		auto commandList = owningDevice->internalQueue->CreateCommandList();

		// create the staging buffer
		D3D12MA::ALLOCATION_DESC textureUploadAllocDesc = {};
		textureUploadAllocDesc.HeapType = D3D12_HEAP_TYPE_UPLOAD;
		D3D12_RESOURCE_DESC textureUploadResourceDesc = {};
		textureUploadResourceDesc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
		textureUploadResourceDesc.Alignment = 0;
		textureUploadResourceDesc.Width = bytes.size();
		textureUploadResourceDesc.Height = 1;
		textureUploadResourceDesc.DepthOrArraySize = 1;
		textureUploadResourceDesc.MipLevels = 1;
		textureUploadResourceDesc.Format = DXGI_FORMAT_UNKNOWN;
		textureUploadResourceDesc.SampleDesc.Count = 1;
		textureUploadResourceDesc.SampleDesc.Quality = 0;
		textureUploadResourceDesc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
		textureUploadResourceDesc.Flags = D3D12_RESOURCE_FLAG_NONE;
		ComPtr<ID3D12Resource> textureUpload;
		D3D12MA::Allocation* textureUploadAllocation;
		DX_CHECK(owningDevice->allocator->CreateResource(
			&textureUploadAllocDesc,
			&textureUploadResourceDesc,
			D3D12_RESOURCE_STATE_GENERIC_READ,
			nullptr, // pOptimizedClearValue
			&textureUploadAllocation,
			IID_PPV_ARGS(&textureUpload)));
		textureUpload->SetName(L"textureUpload");

		auto bytesPerRow = bytes.size() / config.width;
		D3D12_SUBRESOURCE_DATA textureSubresourceData = {};
		textureSubresourceData.pData = bytes.data();
		textureSubresourceData.RowPitch = bytesPerRow;
		textureSubresourceData.SlicePitch = bytes.size();	// TODO: for 3D textures, this should be the size of one layer

		UpdateSubresources(commandList.Get(), texture.Get(), textureUpload.Get(), 0, 0, 1, &textureSubresourceData);


		D3D12_RESOURCE_BARRIER textureBarrier = {};
		textureBarrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
		textureBarrier.Transition.pResource = texture.Get();
		textureBarrier.Transition.StateBefore = D3D12_RESOURCE_STATE_COPY_DEST;
		textureBarrier.Transition.StateAfter = D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE | D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE;
		textureBarrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
		commandList->ResourceBarrier(1, &textureBarrier);


		commandList->Close();
		auto fenceValue = owningDevice->internalQueue->ExecuteCommandList(commandList);
		owningDevice->internalQueue->WaitForFenceValue(fenceValue);

		textureUploadAllocation->Release();	// no longer need this so get rid of it

	}
	TextureD3D12::TextureD3D12(decltype(owningDevice) owningDevice, const TextureConfig& config) : owningDevice(owningDevice), ITexture({ config.width,config.height })
	{
		auto format = rgl2dxgiformat_texture(config.format);
		auto mainResourceFormat = format;

		const bool isDS = (config.aspect.HasDepth || config.aspect.HasStencil);

		// we cannot sample depth textures directly
		// instead, we have to create this resource as Typeless, and then cast it to compatbile
		// formats for the SRVs
		if (isDS && config.usage.Sampled) {
			mainResourceFormat = typelessForDS(format);
		}

		D3D12_RESOURCE_DESC resourceDesc = {};
		resourceDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
		resourceDesc.Alignment = 0;
		resourceDesc.Width = config.width;
		resourceDesc.Height = config.height;
		resourceDesc.DepthOrArraySize = config.arrayLayers;
		resourceDesc.MipLevels = config.mipLevels;
		resourceDesc.Format = mainResourceFormat;
		resourceDesc.SampleDesc.Count = 1;
		resourceDesc.SampleDesc.Quality = 0;
		resourceDesc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
		resourceDesc.Flags = D3D12_RESOURCE_FLAG_NONE;


		D3D12_CLEAR_VALUE optimizedClearValue = {
			.Format = format,
		};

		D3D12_RESOURCE_STATES initialState = rgl2d3d12resourcestate(config.initialLayout);
		if (isDS) {
			resourceDesc.Flags |= D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL;
			optimizedClearValue.DepthStencil = { 1,0 };
			//initialState |= D3D12_RESOURCE_STATE_DEPTH_WRITE;
		}

		if (config.usage.ColorAttachment) {
			resourceDesc.Flags |= D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET;
			std::fill(optimizedClearValue.Color, optimizedClearValue.Color + std::size(optimizedClearValue.Color), 0);
			//initialState |= D3D12_RESOURCE_STATE_RENDER_TARGET;
		}


		D3D12MA::ALLOCATION_DESC allocDesc = {};
		allocDesc.HeapType = D3D12_HEAP_TYPE_DEFAULT;

		// allocate the resource

		HRESULT hr = owningDevice->allocator->CreateResource(
			&allocDesc, &resourceDesc,
			initialState, (resourceDesc.Flags & D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL || resourceDesc.Flags & D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET) ? &optimizedClearValue : nullptr,
			&allocation, IID_PPV_ARGS(&texture));

		std::wstring wide;
		wide.resize(config.debugName == nullptr? 0 : strlen(config.debugName));
		MultiByteToWideChar(CP_UTF8, 0, config.debugName, -1, wide.data(), wide.size());
		texture->SetName(wide.data());

		// add the resource to the appropriate heaps
		PlaceInHeaps(owningDevice, format, config);
	}
	void TextureD3D12::PlaceInHeaps(const std::shared_ptr<RGL::DeviceD3D12>& owningDevice, DXGI_FORMAT format, const RGL::TextureConfig& config)
	{
		const bool isDS = (config.aspect.HasDepth || config.aspect.HasStencil);

		if (isDS) {
			dsvIDX = owningDevice->DSVHeap->AllocateSingle();
			D3D12_DEPTH_STENCIL_VIEW_DESC desc{
				.Format = format,
				.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2D,
			};
			auto handle = owningDevice->DSVHeap->GetCpuHandle(dsvIDX);
			owningDevice->device->CreateDepthStencilView(texture.Get(), &desc, handle);
		}
		if (config.usage.ColorAttachment) {
			rtvIDX = owningDevice->RTVHeap->AllocateSingle();
			D3D12_RENDER_TARGET_VIEW_DESC desc{
				.Format = format,
				.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2D,
			};
			auto handle = owningDevice->RTVHeap->GetCpuHandle(rtvIDX);
			owningDevice->device->CreateRenderTargetView(texture.Get(), &desc, handle);
		}
		if (config.usage.Sampled) {
			if (isDS) {
				// we need to change the format again because depth formats are not allowed for use in SRVs
				format = typelessForSRV(format);
			}
			srvIDX = owningDevice->CBV_SRV_UAVHeap->AllocateSingle();
			D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
			srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
			srvDesc.Format = format;
			srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
			srvDesc.Texture2D.MipLevels = config.mipLevels;

			auto handle = owningDevice->CBV_SRV_UAVHeap->GetCpuHandle(srvIDX);
			owningDevice->device->CreateShaderResourceView(texture.Get(), &srvDesc, handle);
		}
	}
	Dimension TextureD3D12::GetSize() const
	{
		return size;
	}
	TextureD3D12::~TextureD3D12()
	{
		texture = nullptr;
		if (allocation) {
			allocation->Release();
		}
		// release descriptors
		if (rtvAllocated()) {
			owningDevice->RTVHeap->DeallocateSingle(rtvIDX);
			rtvIDX = unallocated;
		}
		if (srvAllocated()) {
			owningDevice->CBV_SRV_UAVHeap->DeallocateSingle(srvIDX);
			srvIDX = unallocated;
		}
		if (dsvAllocated()) {
			owningDevice->DSVHeap->DeallocateSingle(dsvIDX);
			dsvIDX = unallocated;
		}
	}
}
#endif