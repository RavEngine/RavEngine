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
		Debug::Warning("Cannot replicate networked object from client");
	}
	
	//ownership is client? do nothing, clients cannot spawn things
    //instead use an RPC to have the server construct it and then spawn it
}

void NetworkManager::Destroy(Ref<World> source, Ref<NetworkIdentity> comp) {
	// ownership is server and running in the server? need to RPC clients
	if (IsServer() && comp->Owner == k_HSteamNetConnection_Invalid){
		auto entity = comp->getOwner().lock();
		if (entity){
			server->DestroyEntity(entity);
		}
	}
	//ownership is client and running on correct client? need to RPC a deletion request to the server, which will then RPC the other clients
    //if the deletion is honored
	else if (IsClient()){
		
	}
	else{
		Debug::Warning("Cannot replicate entity destruction without networking");
	}
}

void NetworkManager::NetSpawn(const string_view& command){
	//unpack the command
	ctti_t id;
	uint8_t offset = 1;
	
	//CTTI id
	std::memcpy(&id,command.data()+offset,sizeof(id));
	offset += sizeof(id);
	
	// uuid
	char uuid_bytes[16];
	std::memcpy(uuid_bytes, command.data()+offset, 16);
	uuids::uuid uuid(uuid_bytes);
	offset += 16;
	
	// world name
	char worldname[World::id_size];
    std::memcpy(worldname, command.data()+offset, World::id_size);
	
	//find the world and spawn
	if (auto e = CreateEntity(id, uuid)){
		if (auto world = App::GetWorldByName(std::string(worldname,World::id_size))){
			world.value()->Spawn(e.value());
			auto netid = e.value()->GetComponent<NetworkIdentity>();
			if (netid){
                Debug::Assert(netid->GetNetworkID() == uuid, "Created object does not have correct NetID! {} != {}",uuid.to_string(), netid->GetNetworkID().to_string());
                
                //NetSpawn always runs on the client
                client->NetworkIdentities[netid->GetNetworkID()] = netid;
			}
			else{
				Debug::Fatal("Cannot spawn networked entity without NetworkIdentity! Check uuid constructor.");
			}
		}
		else{
			Debug::Fatal("Cannot spawn networked entity in unloaded world: {}", worldname);
		}
	}
	else{
		Debug::Fatal("Cannot spawn entity with type ID {}",id);
	}
}

void NetworkManager::NetDestroy(const string_view& command){
	//unpack the command
	uint8_t offset = 1;
	
	//uuid
	char uuid_bytes[16];
	std::memcpy(uuid_bytes,command.data() + offset,16);
	uuids::uuid uuid(uuid_bytes);
    
    //this always runs on the client -- destructions are requests if coming from the client
    
	//lookup the entity and destroy it
	if (client->NetworkIdentities.contains(uuid)){
		auto id = client->NetworkIdentities.at(uuid);
		auto owner = id->getOwner().lock();
		if (owner){
			owner->Destroy();
		}
		client->NetworkIdentities.erase(uuid);
	}
	else{
		Debug::Warning("Cannot destroy entity with UUID {} because it does not exist",uuid.to_string());
	}
}

bool NetworkManager::IsClient() {
	return static_cast<bool>(App::networkManager.client);
}

void RavEngine::NetworkManager::OnMessageReceived(const std::string_view& message)
{
	//get the command code (first byte in the message)
	uint8_t cmdcode = message[0];
	switch (cmdcode) {
	case NetworkBase::CommandCode::Spawn:
		NetSpawn(message);
		break;
	case NetworkBase::CommandCode::Destroy:
		NetDestroy(message);
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
