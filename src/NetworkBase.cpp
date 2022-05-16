#include "NetworkBase.hpp"
#include <fmt/format.h>
#include <cstdint>
#include "World.hpp"

using namespace std;
using namespace RavEngine;

std::string RavEngine::NetworkBase::CreateSpawnCommand(const uuids::uuid& id, ctti_t type, std::string_view& worldID)
{
	constexpr uint16_t size = 16 + sizeof(type) + World::id_size + 1;
	char message[size];
	memset(message, 0, size);

	//set command code
	message[0] = CommandCode::Spawn;

	char offset = 1;
	//set type
	memcpy(message + offset, &type, sizeof(type));

	offset += sizeof(type);

	//set uuid
	auto raw = id.raw();
	memcpy(message + offset, raw, 16);
	offset += 16;

	//set worldid
	memcpy(message + offset, worldID.data(), World::id_size);

	return string(message,size);
}

std::string RavEngine::NetworkBase::CreateDestroyCommand(const uuids::uuid& id)
{
	constexpr uint16_t size = 16 + 1;
	char message[size];
	
	//set command code
	message[0] = CommandCode::Destroy;
	
	//set uuid
	auto raw = id.raw();
	memcpy(message + 1, raw, 16);
	
	return string(message,size);
}

