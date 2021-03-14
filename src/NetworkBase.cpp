#include "NetworkBase.hpp"
#include <fmt/format.h>

using namespace std;

std::string RavEngine::NetworkBase::CreateSpawnCommand(uuids::uuid& id, ctti_t type, std::string_view& worldID)
{
	return "Hello!";
}

std::string RavEngine::NetworkBase::CreateDestroyCommand(uuids::uuid& id, std::string_view& worldID)
{
	return std::string();
}
