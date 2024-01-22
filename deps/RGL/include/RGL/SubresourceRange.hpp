#pragma once
#include <cstdint>
#include <limits>
#include <cmath>

namespace RGL {

	// bitmasks representing the mip levels and array layers that the View represents.
	// these are combined. So if a texture covers mip 1 and array layers 2 and 3, then mip 1 is covered on both array 2 and 3. 
	// it is not possible to make a view that covers (mip 1, array 2) and (mip 0, array 3) at the same time, for instance
	using covered_mips_t = uint32_t;
	using covered_layers_t = uint64_t;

	constexpr covered_mips_t ALL_MIPS = std::numeric_limits<decltype(ALL_MIPS)>::max();
	constexpr covered_layers_t ALL_LAYERS = std::numeric_limits<decltype(ALL_LAYERS)>::max();

	inline covered_mips_t MakeMipMaskForIndex(covered_mips_t index) {
		return 1 << index;
	}

	inline covered_layers_t MakeLayerMaskForIndex(covered_layers_t layer) {
		return 1 << layer;
	}

	/**
	* mipMask must have one bit set
	*/
	inline uint32_t MaskToMipLevel(covered_mips_t mipMask) {
		return std::log2(mipMask);
	}

	inline uint32_t MaskToLayer(covered_layers_t layerMask) {
		return std::log2(layerMask);
	}
}