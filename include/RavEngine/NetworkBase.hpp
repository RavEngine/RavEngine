#pragma once
#include <atomic>
#include <thread>
#include <string>
#include "Uuid.hpp"
#include "CTTI.hpp"
#include "NetworkIdentity.hpp"
#include "Ref.hpp"
#include "SpinLock.hpp"
#include "DataStructures.hpp"
#include "ComponentHandle.hpp"

namespace RavEngine{

class NetworkBase{
    friend class NetworkManager;
protected:
	std::thread worker;
	std::atomic<bool> workerIsRunning = false;
	std::atomic<bool> workerHasStopped = true;

	std::string CreateSpawnCommand(const uuids::uuid& id, ctti_t type, std::string_view& worldID);

	std::string CreateDestroyCommand(const uuids::uuid& id);

    //Track all the networkidentities by their IDs
    locked_node_hashmap<uuids::uuid, Entity,SpinLock> NetworkIdentities;

public:
	
	enum Reliability{
		Unreliable = k_nSteamNetworkingSend_Unreliable,
		Reliable = k_nSteamNetworkingSend_Reliable
	};

	struct CommandCode {
		enum {
			Spawn = 1,			// receive on client
			Destroy,			// receive on client
			RPC,
            OwnershipToThis,	// receive on client
            OwnershipRevoked,	// receive on client
			ClientRequestingWorldSynchronization	// receive on server
		};
	};
};

}
