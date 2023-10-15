#pragma once
#include <memory>
#include <unordered_map>
namespace RGL {

	struct BufferBindingInfo {
		bool isUAV = false;
	};
	using BufferBindingStore = std::shared_ptr<std::unordered_map<uint32_t, BufferBindingInfo>>;

}