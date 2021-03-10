#include "NetworkManager.hpp"
#include "NetworkIdentity.hpp"
#include "App.hpp"
#include "NetworkReplicable.hpp"

using namespace RavEngine;
using namespace std;

void NetworkManager::Spawn(Ref<World> source, Ref<NetworkIdentity> comp) {
	// Running on the server?
    if (IsServer()){
        auto entity = comp->getOwner().lock();
        if (entity){
			auto casted = dynamic_pointer_cast<NetworkReplicable>(entity);
			if (casted){
				auto id = casted->NetTypeID();
				auto netID = entity->GetComponent<NetworkIdentity>()->GetNetworkID();
				//TODO: send highest-priority safe message with this info to clients
				
			}
        }
    }

	//ownership is client? do nothing, clients cannot spawn things
    //instead use an RPC to have the server construct it and then spawn it
}

void NetworkManager::Destroy(Ref<World> source, Ref<NetworkIdentity> comp) {
	// ownership is server? need to RPC clients

	//ownership is client? need to RPC server, which will then RPC the other clients
}

bool NetworkManager::IsClient() {
	return static_cast<bool>(App::networkManager.client);
}

bool NetworkManager::IsServer() {
	return static_cast<bool>(App::networkManager.server);
}
