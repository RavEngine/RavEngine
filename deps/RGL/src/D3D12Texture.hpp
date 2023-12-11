#pragma once
#define NOMINMAX
#include <RGL/Types.hpp>
#include <RGL/Texture.hpp>
#include "RGLD3D12.hpp"
#include "D3D12TrackedResource.hpp"
#include <d3d12.h>
#include <RGL/Span.hpp>
#undef max

namespace D3D12MA {
	struct Allocation;
}

namespace RGL {
	struct IDevice;
	struct DeviceD3D12;
	struct TextureD3D12 : public ITexture, public D3D12TrackedResource {
		friend class SwapchainD3D12;
		Microsoft::WRL::ComPtr<ID3D12Resource> texture;
		const std::shared_ptr<DeviceD3D12> owningDevice;
		D3D12MA::Allocation* allocation = nullptr;

		constexpr static UINT unallocated = std::numeric_limits<UINT>::max();

		UINT dsvIDX = unallocated, rtvIDX = unallocated, srvIDX = unallocated, uavIDX = unallocated;

		// also add a UAVallocated
		bool dsvAllocated() const{
			return dsvIDX != unallocated;
		}
		bool rtvAllocated() const{
			return rtvIDX != unallocated;
		}
		bool srvAllocated() const{
			return srvIDX != unallocated;
		}
		bool uavAllocated() const {
			return uavIDX != unallocated;
		}

		std::vector<UINT> mipHeapIndicesSRV, mipHeapIndicesUAV;
		uint32_t numMips = 1;

		TextureD3D12(decltype(texture) image, const Dimension& size, decltype(rtvIDX), decltype(owningDevice));
		TextureD3D12(decltype(texture) image, const TextureConfig& config, std::shared_ptr<IDevice> device, D3D12_RESOURCE_STATES nativeStateOverride = D3D12_RESOURCE_STATE_COMMON);	// for externally-managed rendertargets
		TextureD3D12(decltype(owningDevice), const TextureConfig&, untyped_span bytes);
		TextureD3D12(decltype(owningDevice), const TextureConfig&);

		void PlaceInHeaps(const std::shared_ptr<RGL::DeviceD3D12>& owningDevice, DXGI_FORMAT format, const RGL::TextureConfig& config);

		TextureView GetDefaultView() const final;
		TextureView GetViewForMip(uint32_t mip) const final;


		Dimension GetSize() const final;
		virtual ~TextureD3D12();

		ID3D12Resource* GetResource() const final {
			return texture.Get();
		}

		std::string debugName;
	};
}