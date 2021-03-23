#pragma once
#include <string>
#include <optional>
#include <cstring>

namespace RavEngine {
	class RPCMsgUnpacker {
		std::string message;
		uint32_t offset = header_size;   //advance past the RPC message header
        
        template<typename T>
        inline T Deserialize() const{
            T val;
            std::memcpy(&val, message.data() + offset+sizeof(ctti_t), type_serialized_size<T>());
            return val;
        }
        
	public:
        static constexpr size_t header_size = 16+1;
        
		RPCMsgUnpacker(const std::string& msg) : message(msg) {}

		template<typename T>
		inline std::optional<T> get() {
			std::optional<T> result;
			//is the current parameter the same type as T?
            ctti_t enc_type;
            std::memcpy(&enc_type,message.data()+offset,sizeof(enc_type));
            if (enc_type == CTTI<T>){
                //deserialize the type using template specialization
                auto val = Deserialize<T>();

                //advance the offset pointer
                offset += total_serialized_size(val);

                //emplace into the optional
                result.emplace(val);
            }
				
			return result;
		}
        
        template<typename T>
        static inline constexpr size_t type_serialized_size(){
            return sizeof(T);
        }
        
        template<typename T>
        static inline constexpr size_t total_serialized_size(const T& value){
            return sizeof(CTTI<T>) + type_serialized_size<T>(); //encode what type it is + its size
        }
	};
}
