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
            std::memcpy(&val, message.data() + offset+sizeof(ctti_t), SerializedSize<T>());
            return val;
        }
        
	public:
        static constexpr size_t code_offset = 16 + 1;
        static constexpr size_t header_size = code_offset + sizeof(uint16_t);    //uuid, command code, method ID
        
		RPCMsgUnpacker(const std::string& msg) : message(msg) {}

		template<typename T>
        constexpr inline std::optional<T> Get() {
			std::optional<T> result;
			//is the current parameter the same type as T?
            ctti_t enc_type = 0;
            std::memcpy(&enc_type,message.data()+offset,sizeof(enc_type));
            if (enc_type == CTTI<T>()){
                //deserialize the type using template specialization
                auto val = Deserialize<T>();

                //advance the offset pointer
                offset += TotalSerializedSize<decltype(val)>();

                //emplace into the optional
                result.emplace(std::move(val));
            }
				
			return result;
		}
        
        template<typename T>
        static inline constexpr size_t SerializedSize(){
            return sizeof(T);
        }
        
        template<typename T>
        static inline constexpr size_t TotalSerializedSize(){
            return sizeof(CTTI<T>()) + SerializedSize<T>(); //encode what type it is + its size
        }
	};
}
