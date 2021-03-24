#pragma once
#include "Component.hpp"
#include "Queryable.hpp"
#include "RPCMsgUnpacker.hpp"
#include "Ref.hpp"
#include <functional>
#include "DataStructures.hpp"
#include "SpinLock.hpp"
#include "NetworkBase.hpp"
#include "Debug.hpp"
#include "Entity.hpp"
#include "App.hpp"

namespace RavEngine {
	class RPCComponent : public Component, public Queryable<RPCComponent> {
		typedef locked_hashmap<uint16_t, std::function<void(RPCMsgUnpacker&)>, SpinLock> rpc_store;
		rpc_store ClientRPCs, ServerRPCs;

		typedef ConcurrentQueue<std::string_view> queue_t;
		queue_t C_buffer_A, C_buffer_B, S_buffer_A, S_buffer_B;
		std::atomic<queue_t*> readingptr_c = &C_buffer_A, writingptr_c = &C_buffer_B,
							  readingptr_s = &S_buffer_A, writingptr_s = &S_buffer_B;
		
		template<typename U>
		inline void RegisterRPC_Impl(uint16_t id, Ref<U> thisptr, void(U::* f)(RPCMsgUnpacker&), rpc_store& store) {
			WeakRef<U> weak(thisptr);
			auto func = [=](RPCMsgUnpacker& u) {
				auto ptr = weak.lock();
				if (ptr) {
					(ptr.get()->*f)(u);
				}
			};
 			store[id] = func;
		}

		template<typename T>
		inline void serializeType(size_t& offset, char* buffer, const T& value) {
			constexpr auto id = CTTI<T>();
			std::memcpy(buffer + offset, &id, sizeof(ctti_t));
			std::memcpy(buffer + offset + sizeof(ctti_t), &value, RPCMsgUnpacker::type_serialized_size<T>());
			offset += RPCMsgUnpacker::total_serialized_size(value);
		}
        
		template<typename ... A>
		inline std::string SerializeRPC(uint16_t id, A ... args) {
			constexpr size_t totalsize = (RPCMsgUnpacker::total_serialized_size(args) + ...) + RPCMsgUnpacker::header_size;

			auto uuid_bytes = getOwner().lock()->GetComponent<NetworkIdentity>()->GetNetworkID().raw();
			char msg[totalsize];

			//write message header
			std::memset(msg, 0, totalsize);
            msg[0] = NetworkBase::CommandCode::RPC;							//command code
			std::memcpy(msg + 1, uuid_bytes.c_str(), uuid_bytes.length());	//entity uuid
			std::memcpy(msg + 1 + uuid_bytes.length(), &id, sizeof(id));	//RPC ID

			//write mesage body
			size_t offset = RPCMsgUnpacker::header_size;
			(serializeType(offset,msg,args),...);		//fold expression on all variadics
			Debug::Assert(offset == totalsize, "Incorrect number of bytes written!");
			return std::string(msg,totalsize);
		}

		inline void ProcessRPCs_impl(const std::atomic<queue_t*>& ptr, const rpc_store& table) {
			auto reading = ptr.load();
			std::string cmd;
			while (reading->try_dequeue(cmd)) {
				//read out of the header which RPC to invoke
				uint16_t RPC;
				std::memcpy(&RPC, cmd.data() + RPCMsgUnpacker::code_offset, sizeof(RPC));

				//invoke that RPC
				if (table.contains(RPC)) {
					table.at(RPC)(RPCMsgUnpacker(cmd));
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
		@param thisptr the object to invoke, should be a component on the same entity
		@param f the function to call
		*/
		template<typename U>
		inline void RegisterServerRPC(uint16_t name, Ref<U> thisptr, void(U::* f)(RPCMsgUnpacker&)) {
			RegisterRPC_Impl(name, thisptr, f, ServerRPCs);
		}

		/**
		Register a Client RPC - this is run on a client when invoked from the server
		@param name the name for the RPC (must be unique)
		@param thisptr the object to invoke, should be a component on the same entity
		@param f the function to call
		*/
		template<typename U>
		inline void RegisterClientRPC(uint16_t name, Ref<U> thisptr, void(U::* f)(RPCMsgUnpacker&)) {
			RegisterRPC_Impl(name, thisptr, f, ClientRPCs);
		}

		/**
		Invoke an RPC on the server
		@param id the name of the RPC
		@param args templated parameter list
		*/
		template<typename ... A>
		inline void InvokeServerRPC(uint16_t id, A ... args) {
			if (ServerRPCs.contains(id)) {
				auto msg = SerializeRPC(id, args...);
				App::networkManager.client->SendMessageToServer(msg);
			}
			else {
				Debug::Warning("Cannot send Server RPC with ID {}",id);
			}
		}

		/**
		Invoke an RPC on the client
		@param id the name of the RPC
		@param args templated parameter list
		*/
		template<typename ... A>
		inline void InvokeClientRPC(uint16_t id, A ... args) {
			if (ClientRPCs.contains(id)) {
				auto msg = SerializeRPC(id, args...);
				App::networkManager.server->SendMessageToAllClients(msg);
			}
			else {
				Debug::Warning("Cannot send Client RPC with ID {}", id);
			}
		}

		/**
		This call is not for you.
		*/
		inline void CacheClientRPC(const std::string_view& cmd) {
			writingptr_c.load()->enqueue(std::string(cmd.data(),cmd.size()));
		}

		inline void CacheServerRPC(const std::string_view& cmd) {
			writingptr_s.load()->enqueue(std::string(cmd.data(),cmd.size()));
		}

		inline void ProcessClientRPCs() {
			ProcessRPCs_impl(readingptr_c,ClientRPCs);
		}


		inline void ProcessServerRPCs() {
			ProcessRPCs_impl(readingptr_s, ServerRPCs);
		}


		/**
		For internal use only. Switches which queue is being filled and which is being emptied
		*/
		inline void Swap() {
			queue_t *reading = readingptr_c.load(), *writing = writingptr_c.load();
			std::swap(reading, writing);
			readingptr_c.store(reading);
			writingptr_c.store(writing);

			reading = readingptr_s.load(), writing = writingptr_s.load();
			std::swap(reading, writing);
			readingptr_s.store(reading);
			writingptr_s.store(writing);
		}
	};
}
