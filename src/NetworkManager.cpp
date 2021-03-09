#include "NetworkManager.hpp"
#include "NetworkIdentity.hpp"
#include "App.hpp"
#include "NetworkReplicable.hpp"

using namespace RavEngine;

void NetworkManager::Spawn(Ref<World> source, Ref<NetworkIdentity> comp) {
	// Running on the server?
    if (IsServer()){
        auto entity = comp->getOwner().lock();
        if (entity){
            //serialize all of the replicable components
            auto serialize = entity->GetAllComponentsOfTypeFastPath<NetworkReplicable>();
            
            phmap::flat_hash_map<std::string, std::string> masterDict;
            
            for(const auto& component : serialize){
                //TODO: convert the map into a string
                //TODO: add to master dictionary
            }
            //TODO: convert master dictionary into a string
            
            //TODO: send over the network to all the clients (except self if we are also a client!)
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
