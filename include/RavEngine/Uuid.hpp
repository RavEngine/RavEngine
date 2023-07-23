#pragma once
#include <array>
#include <string>

namespace RavEngine {
	namespace uuids {
		struct uuid {
			constexpr static uint8_t nbytes = 16;

			// default uuids start with all 0s
			uuid() {}
			
			// construct from data ptr
			uuid(const uint8_t* other_data) {
				std::memcpy(data.data(), other_data, sizeof(data));
			}

			// uint8_t alias constructor for char*
			uuid(const char* other_data) : uuid(reinterpret_cast<const uint8_t*>(other_data)) {}

			static uuid create();

			inline auto raw() const{
				return data.data();
			}

			std::string to_string() const;

			inline constexpr static size_t size() {
				return nbytes;
			}

			inline bool operator==(const uuid& other) const{
				for (uint8_t i = 0; i < nbytes; i++) {
					if (data[i] != other.data[i]) {
						return false;
					}
				}
				return true;
			}

			std::array<uint8_t, nbytes> data{0};
		};
	}
}

namespace std {
	template<>
	struct hash<RavEngine::uuids::uuid> {
		inline size_t operator()(const RavEngine::uuids::uuid& id) const {
			uint64_t high = *reinterpret_cast<const uint64_t*>(id.data.data());
			uint64_t low = *(reinterpret_cast<const uint64_t*>(id.data.data())+1);
			return high ^ low;
		}
	};
}