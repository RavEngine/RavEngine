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

namespace RavEngine {
	class RPCComponent : public Component, public Queryable<RPCComponent> {
		typedef locked_hashmap<std::string, std::function<void(RPCMsgUnpacker&)>, SpinLock> rpc_store;
		rpc_store ClientRPCs, ServerRPCs;

		
		template<typename U>
		inline void RegisterRPC_Impl(const std::string& name, Ref<U> thisptr, void(U::* f)(RPCMsgUnpacker&), rpc_store& store) {
			WeakRef<U> weak(thisptr);
			auto func = [=](RPCMsgUnpacker& u) {
				(weak.lock().get()->*f)(u);
			};
 			store[name] = func;
		}

		template<typename T>
		inline void serializeType(size_t& offset, char* buffer, const T& value) {
			std::memcpy(buffer + offset + sizeof(ctti_t), &value, sizeof(value));
            std::memcpy(buffer + offset, &CTTI<T>,sizeof(ctti_t));
			offset += RPCMsgUnpacker::total_serialized_size(value) + sizeof(ctti_t);
		}
        
		template<typename ... A>
		inline std::string SerializeRPC(const std::string& id, A ... args) {
			const size_t totalsize = (RPCMsgUnpacker::total_serialized_size(args) + ...) + RPCMsgUnpacker::header_size;

			auto uuid_bytes = getOwner().lock()->GetComponent<NetworkIdentity>()->GetNetworkID().raw();

			char msg[totalsize];
			std::memset(msg, 0, totalsize);
            msg[0] = NetworkBase::CommandCode::RPC;
			std::memcpy(msg + 1, uuid_bytes.c_str(), uuid_bytes.length());

			size_t offset = RPCMsgUnpacker::header_size;
			(serializeType(offset,msg,args),...);		//fold expression on all variadics
			return std::string(msg,totalsize);
		}

	public:
		/**
		Register a server RPC - this is run on the server when invoked from a client
		@param name the name for the RPC (must be unique)
		@param thisptr the object to invoke, should be a component on the same entity
		@param f the function to call
		*/
		template<typename U>
		inline void RegisterServerRPC(const std::string& name, Ref<U> thisptr, void(U::* f)(RPCMsgUnpacker&)) {
			RegisterRPC_Impl(name, thisptr, f, ServerRPCs);
		}

		/**
		Register a Client RPC - this is run on a client when invoked from the server
		@param name the name for the RPC (must be unique)
		@param thisptr the object to invoke, should be a component on the same entity
		@param f the function to call
		*/
		template<typename U>
		inline void RegisterClientRPC(const std::string& name, Ref<U> thisptr, void(U::* f)(RPCMsgUnpacker&)) {
			RegisterRPC_Impl(name, thisptr, f, ClientRPCs);
		}

		/**
		Invoke an RPC on the server
		@param id the name of the RPC
		@param args templated parameter list
		*/
		template<typename ... A>
		inline void InvokeServerRPC(const std::string& id, A ... args) {
			auto msg = SerializeRPC(id,args...);
		}

		/**
		Invoke an RPC on the client
		@param id the name of the RPC
		@param args templated parameter list
		*/
		template<typename ... A>
		inline void InvokeClientRPC(const std::string& id, A ... args) {
			auto msg = SerializeRPC(id, args...);
		}
	};
}
