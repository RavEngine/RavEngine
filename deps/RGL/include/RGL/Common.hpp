#pragma once

namespace RGL {
	enum class DepthCompareFunction : uint8_t {
		Never = 0,			// matches the values of VkCompareOp
		Less = 1,
		Equal = 2,
		LessOrEqual = 3,
		Greater = 4,
		NotEqual = 5,
		GreaterOrEqual = 6,
		Always = 7,
	};
}