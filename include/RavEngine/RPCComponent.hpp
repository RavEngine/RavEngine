#pragma once
#include "Component.hpp"
#include "Queryable.hpp"
#include "RPCMsgUnpacker.hpp"
#include "Ref.hpp"
#include <functional>
#include "DataStructures.hpp"
#include "SpinLock.hpp"

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

		template<typename ... A>
		inline std::string SerializeRPC(const std::string& id, A ... args) {
			return "";
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

		template<typename ... A>
		inline void InvokeServerRPC(const std::string& id, A ... args) {
			//auto msg = SerializeRPC(id,...args);
		}

		template<typename ... A>
		inline void InvokeClientRPC(const std::string& id, A ... args) {

		}
	};
}