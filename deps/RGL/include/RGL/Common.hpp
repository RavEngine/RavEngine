#pragma once

// grrr
#undef Never
#undef Less
#undef Equal
#undef LessOrEqual
#undef Greater
#undef NotEqual
#undef GreaterOrEqual
#undef Always

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
		None
	};
}
