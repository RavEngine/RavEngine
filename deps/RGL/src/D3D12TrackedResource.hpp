#pragma once
#include <directx/d3d12.h>

namespace RGL {

	struct D3D12TrackedResource {
		D3D12_RESOURCE_STATES nativeState = D3D12_RESOURCE_STATE_COMMON;
		virtual ID3D12Resource* GetResource() const = 0;
	};

}