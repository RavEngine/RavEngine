#include "NetworkManager.hpp"
#include "NetworkIdentity.hpp"
#include "App.hpp"

using namespace RavEngine;

void NetworkManager::Spawn(Ref<World> source, Ref<NetworkIdentity> comp) {
	// ownership is server? need to RPC clients

	//ownership is client? need to RPC server, which will then RPC the other clients
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