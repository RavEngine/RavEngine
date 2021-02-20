#pragma once
#include "NetworkServer.hpp"
#include "NetworkClient.hpp"
#include "World.hpp"

namespace RavEngine {

	class World;
	class NetworkIdentity;

	class NetworkManager{
	public:
		static bool IsServer();
	
		static bool IsClient();
	
		std::unique_ptr<NetworkServer> server;
		std::unique_ptr<NetworkClient> client;

		/**
		* Spawn a networkidentity. For internal use only, called by the world
		*/
		void Spawn(Ref<World> source, Ref<NetworkIdentity> comp);

		/**
		* Spawn a networkidentity. For internal use only, called by the world
		*/
		void Destroy(Ref<World> source, Ref<NetworkIdentity> comp);
	};

}
