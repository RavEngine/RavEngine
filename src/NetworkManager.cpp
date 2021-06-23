#include "NetworkManager.hpp"
#include "NetworkIdentity.hpp"
#include "App.hpp"
#include "NetworkReplicable.hpp"
#include "Debug.hpp"

using namespace RavEngine;
using namespace std;

void NetworkManager::Spawn(Ref<World> source, Ref<NetworkIdentity> comp) {
	// Running on the server?
    if (IsServer()){
        auto entity = comp->getOwner().lock();
        if (entity){
			server->SpawnEntity(entity);
        }
    }
	else{
		Debug::Warning("Cannot replicate entity creation from client");
	}
	
	//ownership is client? do nothing, clients cannot spawn things
    //instead use an RPC to have the server construct it and then spawn it
}

void NetworkManager::Destroy(Ref<World> source, Ref<NetworkIdentity> comp) {
	// ownership is server and running in the server? need to RPC clients
	if (IsServer()){	//even if the server does not own this object, if it is destroyed here, it must be replicated
		auto entity = comp->getOwner().lock();
		if (entity){
			server->DestroyEntity(entity);
		}
	}
	else{
		Debug::Warning("Cannot replicate entity destruction from client");
	}
}

bool NetworkManager::IsClient() {
	return static_cast<bool>(App::networkManager.client);
}

bool NetworkManager::IsServer() {
	return static_cast<bool>(App::networkManager.server);
}

void NetworkManager::SyncVarUpdate(const std::string_view &data){
	if (IsServer()){
		//local client (if there is one) already has the value, so just push to the other clients
		server->SendMessageToAllClients(data, NetworkBase::Reliability::Reliable);
	}
	else if (IsClient()){
		//update server, which will then update clients
		client->SendMessageToServer(data, NetworkBase::Reliability::Reliable);
	}
	else{
		// WAT
		Debug::Fatal("Cannot propagate syncvar, network is not active");
	}
}
