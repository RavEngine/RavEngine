#include "NetworkManager.hpp"
#include "NetworkIdentity.hpp"
#include "App.hpp"
#include "NetworkReplicable.hpp"
#include <iostream>

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

void RavEngine::NetworkManager::Spawn(const std::string_view& command)
{
	//unpack the command
}

void RavEngine::NetworkManager::OnMessageReceived(const std::string_view& message)
{
	//get the command code (first byte in the message)
	uint8_t cmdcode = message[0];
	switch (cmdcode) {
	case NetworkBase::CommandCode::Spawn:
		Spawn(message);
		break;
	case NetworkBase::CommandCode::Destroy:
		break;
	case NetworkBase::CommandCode::RPC:
		break;
	default:
		Debug::Warning("Invalid command code: {}",cmdcode);
	}
}

bool NetworkManager::IsServer() {
	return static_cast<bool>(App::networkManager.server);
}
