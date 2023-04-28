#pragma once
#include <cstdint>
#include "TextureFormat.hpp"

namespace RGL {

	struct Dimension {
		uint32_t width = 0, height = 0;
	};
	
	enum class TextureType : uint8_t {
		T1D,
		T2D,
		T3D,
	};

	enum class TilingMode : uint8_t {
		Optimal,
		Linear
	};

	struct TextureConfig {
		TextureUsage usage;
		TextureAspect aspect;
		uint32_t width = 0, height = 0, depth = 1, mipLevels = 1, arrayLayers = 1;
		TextureType imageType = decltype(imageType)::T2D;
		TextureFormat format;
		TilingMode mode = decltype(mode)::Optimal;
		ResourceLayout initialLayout = ResourceLayout::Undefined;
		const char* debugName = nullptr;
		bool readbackEnabled = false;
	};

	class ITexture {
	protected:
		Dimension size;
	public:
		ITexture(decltype(size) size) : size(size) {}
		virtual Dimension GetSize() const = 0;
	};
}
