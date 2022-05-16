#pragma once
#include <boost/uuid/uuid.hpp>
#include <boost/uuid/random_generator.hpp>
#include <boost/uuid/nil_generator.hpp>
#include <boost/uuid/uuid_io.hpp>

namespace RavEngine {
	namespace uuids {
		struct uuid : public boost::uuids::uuid {
			// default uuids start with all 0s
			uuid() : boost::uuids::uuid(boost::uuids::nil_generator()()) {}
			
			// construct from data ptr
			uuid(const uint8_t* other_data) {
				std::memcpy(data, other_data, sizeof(data));
			}

			// uint8_t alias constructor for char*
			uuid(const char* other_data) : uuid(reinterpret_cast<const uint8_t*>(other_data)) {}

			uuid(const boost::uuids::uuid& other) : boost::uuids::uuid(other) {}

			static uuid create() {
				return boost::uuids::random_generator()();
			}

			inline auto raw() const{
				return data;
			}

			inline std::string to_string() const {
				return boost::uuids::to_string(*this);
			}

			inline static constexpr size_t size() {
				return sizeof(uuid::data);
			}
		};
	}
}