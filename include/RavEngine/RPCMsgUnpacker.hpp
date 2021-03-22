#pragma once
#include <string>
#include <optional>

namespace RavEngine {
	class RPCMsgUnpacker {
		std::string message;
		uint32_t offset = 0;
	public:
		RPCMsgUnpacker(const std::string& msg) : message(msg) {}

		template<typename T>
		inline std::optional<T> get() {
			std::optional<T> result;
			//is the current parameter the same type as T?

				//deserialize the type using template specialization

				//advance the offset pointer

				//emplace into the optional

			return result;
		}
	};
}