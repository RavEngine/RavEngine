#include "NetworkManager.hpp"
#include "NetworkIdentity.hpp"
#include "App.hpp"
#include "Debug.hpp"

using namespace RavEngine;
using namespace std;

void NetworkManager::Spawn(World* source, ctti_t id, entity_t ent_id, const uuids::uuid& uuid) {
	// Running on the server?
    if (IsServer()){
        server->SpawnEntity(source,id,ent_id,uuid);
    }
	else{
		Debug::Warning("Cannot replicate entity creation from client");
	}
	
	//ownership is client? do nothing, clients cannot spawn things
    //instead use an RPC to have the server construct it and then spawn it
}

void NetworkManager::Destroy(const uuids::uuid& entity_id) {
	// ownership is server and running in the server? need to RPC clients
	if (IsServer()){	//even if the server does not own this object, if it is destroyed here, it must be replicated
        server->DestroyEntity(entity_id);
	}
	else{
		Debug::Warning("Cannot replicate entity destruction from client");
	}
}

bool NetworkManager::IsClient() {
	return static_cast<bool>(GetApp()->networkManager.client);
}

bool NetworkManager::IsServer() {
	return static_cast<bool>(GetApp()->networkManager.server);
}
