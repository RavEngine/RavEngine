#pragma once
#include <unordered_map>
#include <memory>
#include <cstdint>

namespace RGL {
	struct BufferBindingInfo {
		bool writable;
	};

	using BufferBindingStore = std::shared_ptr<std::unordered_map<uint32_t, BufferBindingInfo>>;
}