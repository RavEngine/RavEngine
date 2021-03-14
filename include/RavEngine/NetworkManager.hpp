#pragma once
#include "NetworkServer.hpp"
#include "NetworkClient.hpp"
#include "World.hpp"
#include "DataStructures.hpp"
#include "CTTI.hpp"
#include "Ref.hpp"

namespace RavEngine {

	class World;
	class NetworkIdentity;
	class Entity;

	class NetworkManager{
	private:
		locked_hashmap<ctti_t, std::function<Ref<Entity>(void)>> NetworkedObjects;

		void Spawn(const std::string_view& command);
	public:
		
		/**
		 Register an entity class as network spawnable.
		 @param id the type identifier for the entity
		 */
		template<typename T>
		inline void RegisterNetworkedEntity(){
			NetworkedObjects.insert(std::make_pair(CTTI<T>,[]() -> Ref<Entity>{
				//TODO: UUID
				return std::static_pointer_cast<Entity>(std::make_shared<T>());
			}));
		}
		
		/**
		 Unregister an entity class as network spawnable.
		 @param id the type identifier for the entity
		 */
		template<typename T>
		inline void UnregisterNetworkedEntity(){
			NetworkedObjects.erase(CTTI<T>);
		}
		
		/**
		 @param id the identifier for the entity class
		 @return true if a registration for that ID exists
		 */
		template<typename T>
		inline bool IsNetworkedIdentityRegistered(){
			return NetworkedObjects.contains(CTTI<T>);
		}

		/**
		Invoked by client / server when a message is received
		@param message the raw data received
		*/
		void OnMessageReceived(const std::string_view& message);
		
        /**
         @return true If there is an active Server running on this instance
         */
		static bool IsServer();
        /**
         @return true if there is an active Client running on this instance
         */
		static bool IsClient();
		
		/**
		 @return true if this game is networked
		 */
		static inline bool IsNetworked(){
			return IsServer() || IsClient();
		}
	
		std::unique_ptr<NetworkServer> server;
		std::unique_ptr<NetworkClient> client;

		/**
		Spawn a networkidentity. For internal use only, called by the world
		*/
		void Spawn(Ref<World> source, Ref<NetworkIdentity> comp);

		/**
		Spawn a networkidentity. For internal use only, called by the world
		*/
		void Destroy(Ref<World> source, Ref<NetworkIdentity> comp);
	};

}
