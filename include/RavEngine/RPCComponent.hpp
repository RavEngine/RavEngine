#pragma once
#include "Queryable.hpp"
#include "RPCMsgUnpacker.hpp"
#include "Ref.hpp"
#include "DataStructures.hpp"
#include "SpinLock.hpp"
#include "NetworkBase.hpp"
#include "Debug.hpp"
#include "Entity.hpp"
#include "App.hpp"
#include <steam/isteamnetworkingsockets.h>

namespace RavEngine {
	class RPCComponent : public ComponentWithOwner, public Queryable<RPCComponent> {
	public:
		enum class Directionality {
			OnlyOwnerInvokes,			// the owner 
			Bidirectional
		};
		RPCComponent(entity_t owner) : ComponentWithOwner(owner){}
	private:
		struct rpc_entry {
			Function<void(RPCMsgUnpacker&, HSteamNetConnection)> func;
			Directionality mode;
		};

		typedef locked_hashmap<uint16_t, rpc_entry, SpinLock> rpc_store;

        struct enqueued_rpc {
			std::string msg;
			bool isOwner;
			HSteamNetConnection origin;
		};

        typedef ConcurrentQueue<enqueued_rpc> queue_t;
        struct Data{
            queue_t C_buffer_A, C_buffer_B, S_buffer_A, S_buffer_B;
            std::atomic<queue_t*> readingptr_c = &C_buffer_A, writingptr_c = &C_buffer_B,
                readingptr_s = &S_buffer_A, writingptr_s = &S_buffer_B;
            
            rpc_store ClientRPCs, ServerRPCs;

            void Swap(){
                queue_t* reading = readingptr_c.load(), * writing = writingptr_c.load();
                std::swap(reading, writing);
                readingptr_c.store(reading);
                writingptr_c.store(writing);

                reading = readingptr_s.load(), writing = writingptr_s.load();
                std::swap(reading, writing);
                readingptr_s.store(reading);
                writingptr_s.store(writing);
            }
        };
        
        std::shared_ptr<Data> data = std::make_shared<Data>();
	

		/**
		Shared RPC registration code
		@param id the numeric ID for the RPC. Use an enum
		@param func the method to register
		@param store which table to store it in
		@param type ownership control of the RPC
		*/
		template<typename Fn>
		inline void RegisterRPC_Impl(uint16_t id, const Fn& func, rpc_store& store, Directionality type) {
			store[id] = rpc_entry{ func, type };
		}

		/**
		Convert a type into bytes
		@param offset the position in the buffer
		@param buffer the buffer to write the data
		@param value the data to encode
		*/
		template<typename T>
        inline void serializeType(size_t& offset, char* buffer, const T& value) const{
            auto id = CTTI<T>();
			std::memcpy(buffer + offset, &id, sizeof(ctti_t));
			std::memcpy(buffer + offset + sizeof(ctti_t), &value, RPCMsgUnpacker::SerializedSize<T>());
			offset += RPCMsgUnpacker::TotalSerializedSize(value);
		}

		/**
		Create a serialized RPC invocation
		@param id the RPC id number
		@param args the varargs to encode
		*/
		template<typename ... A>
		inline std::string SerializeRPC(uint16_t id, A ... args) const{
			constexpr size_t totalsize = (RPCMsgUnpacker::TotalSerializedSize(args) + ...) + RPCMsgUnpacker::header_size;

			auto uuid_bytes = GetOwner().GetComponent<NetworkIdentity>().GetNetworkID().raw();
			char msg[totalsize];

			//write message header
			std::memset(msg, 0, totalsize);
			msg[0] = NetworkBase::CommandCode::RPC;							//command code
			std::memcpy(msg + 1, uuid_bytes.data(), uuid_bytes.size());	//entity uuid
			std::memcpy(msg + 1 + uuid_bytes.size(), &id, sizeof(id));	//RPC ID

			//write mesage body
			size_t offset = RPCMsgUnpacker::header_size;
			(serializeType(offset, msg, args), ...);		//fold expression on all variadics
			Debug::Assert(offset == totalsize, "Incorrect number of bytes written!");
			return std::string(msg, totalsize);
		}

		/**
		Consume enqueued RPCs
		@param ptr the queue to consume from
		@param table the datastructure to look up the results in
		*/
        inline void ProcessRPCs_impl(const std::atomic<queue_t*>& ptr, const rpc_store& table) {
			auto reading = ptr.load();
			enqueued_rpc cmd;
			while (reading->try_dequeue(cmd)) {
				//read out of the header which RPC to invoke
				uint16_t RPC;
				std::memcpy(&RPC, cmd.msg.data() + RPCMsgUnpacker::code_offset, sizeof(RPC));

				//invoke that RPC
				if (table.if_contains(RPC, [&](const rpc_entry& func) {
					if (cmd.isOwner || func.mode == Directionality::Bidirectional) {
						auto packer = RPCMsgUnpacker(cmd.msg);
						func.func(packer, cmd.origin);
					}
					})) {
				}
				else {
					Debug::Warning("No cmd code with ID {}", RPC);
				}
			}
		}

	public:

		/**
		Register a server RPC - this is run on the server when invoked from a client
		@param name the name for the RPC (must be unique)
		@param func the function to call
		*/
		template<typename Fn>
		inline void RegisterServerRPC(uint16_t name, const Fn& func, Directionality type = Directionality::OnlyOwnerInvokes) {
			RegisterRPC_Impl(name, func, data->ServerRPCs, type);
		}

		/**
		Register a Client RPC - this is run on a client when invoked from the server
		@param name the name for the RPC (must be unique)
		@param func the function to call
		*/
		template<typename Fn>
		inline void RegisterClientRPC(uint16_t name, const Fn& func, Directionality type = Directionality::OnlyOwnerInvokes) {
			RegisterRPC_Impl(name, func, data->ClientRPCs, type);
		}

		/**
		Invoke an RPC on the server
		@param id the name of the RPC
		@param mode importance that the message arrive at the destination
		@param args templated parameter list
		*/
		template<typename ... A>
        constexpr inline void InvokeServerRPC(uint16_t id, NetworkBase::Reliability mode, A ... args) const{
			if (data->ServerRPCs.contains(id)) {
				auto msg = SerializeRPC(id, args...);
				GetApp()->networkManager.client->SendMessageToServer(msg, mode);
			}
			else {
				Debug::Warning("Cannot send Server RPC with ID {}", id);
			}
		}

		template<typename ... A>
        constexpr inline void InvokeClientRPCDirected(uint16_t id, HSteamNetConnection target, NetworkBase::Reliability mode, A ... args) const{
			if (data->ClientRPCs.contains(id)) {
				auto msg = SerializeRPC(id, args...);
				GetApp()->networkManager.server->SendMessageToClient(msg, target, mode);
			}
			else {
				Debug::Warning("Cannot send Client RPC with ID {} to recipient {}", id, target);
			}
		}

		template<typename ... A>
        constexpr inline void InvokeClientRPCToAllExcept(uint16_t id, HSteamNetConnection doNotSend, NetworkBase::Reliability mode, A ... args) const{
			if (data->ClientRPCs.contains(id)) {
				auto msg = SerializeRPC(id, args...);
				GetApp()->networkManager.server->SendMessageToAllClientsExcept(msg, doNotSend, mode);
			}
			else {
				Debug::Warning("Cannot send Client RPC with ID {} to all except {}", id, doNotSend);
			}
		}

		/**
		Invoke an RPC on the client
		@param id the name of the RPC
		@param mode importance that the message arrive at the destination
		@param args templated parameter list
		*/
		template<typename ... A>
        constexpr inline void InvokeClientRPC(uint16_t id, NetworkBase::Reliability mode, A ... args) const{
			if (data->ClientRPCs.contains(id)) {
				auto msg = SerializeRPC(id, args...);
				GetApp()->networkManager.server->SendMessageToAllClients(msg, mode);
			}
			else {
				Debug::Warning("Cannot send Client RPC with ID {}", id);
			}
		}

		/**
		Invoked automatically. For internal use only.
		*/
        inline void CacheClientRPC(const std::string_view& cmd, bool isOwner, HSteamNetConnection origin) {
			data->writingptr_c.load()->enqueue({ std::string(cmd.data(),cmd.size()), isOwner, origin });
		}

		/**
		Invoked automatically. For internal use only.
		*/
        inline void CacheServerRPC(const std::string_view& cmd, bool isOwner, HSteamNetConnection origin) {
			data->writingptr_s.load()->enqueue({ std::string(cmd.data(),cmd.size()), isOwner, origin });
		}

		/**
		Invoked automatically. For internal use only.
		*/
        inline void ProcessClientRPCs() {
			ProcessRPCs_impl(data->readingptr_c, data->ClientRPCs);
		}

		/**
		Invoked automatically. For internal use only.
		*/
        inline void ProcessServerRPCs() {
			ProcessRPCs_impl(data->readingptr_s, data->ServerRPCs);
		}


		/**
		For internal use only. Switches which queue is being filled and which is being emptied
		*/
        inline void Swap() {
            data->Swap();
		}
	};
}
