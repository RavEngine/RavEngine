#pragma once
#include "NetworkServer.hpp"
#include "NetworkClient.hpp"
#include "World.hpp"
#include "DataStructures.hpp"
#include "CTTI.hpp"
#include "Ref.hpp"
#include <uuids.h>
#include <optional>

namespace RavEngine {

	class World;
	struct NetworkIdentity;
	class Entity;

	class NetworkManager{
	private:
		typedef std::function<Ref<Entity>(const uuids::uuid&)> func_t;
		locked_hashmap<ctti_t, func_t> NetworkedObjects;
		
	public:
		
		void SyncVarUpdate(const std::string_view& data);
        
        std::optional<Ref<Entity>> CreateEntity(ctti_t id, uuids::uuid& uuid){
            std::optional<Ref<Entity>> value;
			NetworkedObjects.if_contains(id, [&](const func_t& fn) {
				value.emplace(fn(uuid));
			});
            return value;
        }
        
		
		/**
		 Register an entity class as network spawnable.
		 @param id the type identifier for the entity
		 */
		template<typename T>
        constexpr inline void RegisterNetworkedEntity(){
			NetworkedObjects.insert(std::make_pair(CTTI<T>(),[](const uuids::uuid& id) -> Ref<Entity>{
				return std::static_pointer_cast<Entity>(std::make_shared<T>(id));
			}));
		}
		
		/**
		 Unregister an entity class as network spawnable.
		 @param id the type identifier for the entity
		 */
		template<typename T>
        constexpr inline void UnregisterNetworkedEntity(){
			NetworkedObjects.erase(CTTI<T>);
		}
		
		/**
		 @param id the identifier for the entity class
		 @return true if a registration for that ID exists
		 */
		template<typename T>
        constexpr inline bool IsNetworkedIdentityRegistered(){
			return NetworkedObjects.contains(CTTI<T>);
		}
		
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
