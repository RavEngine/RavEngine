#pragma once
#include <cstdint>
#include <string>

namespace RavEngine {
	namespace SystemInfo {
		/**
		* Get the number of logical cores on the device
		*/
		uint16_t NumLogicalProcessors();

		std::string GPUBrandString();
	}
}