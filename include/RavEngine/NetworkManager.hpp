#pragma once
#include "NetworkServer.hpp"
#include "NetworkClient.hpp"
#include "DataStructures.hpp"
#include "CTTI.hpp"
#include "Ref.hpp"
#include "Entity.hpp"
#include <optional>

namespace RavEngine {

	class World;
	struct NetworkIdentity;

	class NetworkManager{
	private:
		typedef Function<Entity(World*)> func_t;
		locked_hashmap<ctti_t, func_t,SpinLock> NetworkedObjects;

		template <typename T>
		class HasClientCreate
		{
		private:
			typedef char YesType[1];
			typedef char NoType[2];

			template <typename C> static YesType& test(decltype(&C::ClientCreate));
			template <typename C> static NoType& test(...);


		public:
			enum { value = sizeof(test<T>(0)) == sizeof(YesType) };
		};
		
	public:
		
        bool isNetworkEntity(ctti_t id) const{
            return NetworkedObjects.contains(id);
        }
                
        std::optional<Entity> CreateEntity(ctti_t id, World* world){
            std::optional<Entity> value;
			NetworkedObjects.if_contains(id, [&](const auto& fn) {
				Entity entity = fn(world);
				value.emplace(entity);
			});
            return value;
        }
        
		
		/**
		 Register an entity class as network spawnable.
		 @param id the type identifier for the entity
		 */
		template<typename T>
        constexpr inline void RegisterNetworkedEntity(){
			NetworkedObjects.insert(std::make_pair(CTTI<T>(),[](World* world) -> Entity{
                auto e = world->Instantiate<T>();
				if constexpr (HasClientCreate<T>::value) {
					e.ClientCreate();
				}
				return e;
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
		void Spawn(World* source, ctti_t type_id, Entity ent_id, const uuids::uuid& entity_id);

		/**
		Spawn a networkidentity. For internal use only, called by the world
		*/
		void Destroy(const uuids::uuid& entity_id);
	};

}
