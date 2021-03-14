#pragma once
#include <atomic>
#include <thread>
#include <string>
#include <uuids/uuid.h>
#include "CTTI.hpp"

namespace RavEngine{

class NetworkBase{
protected:
	std::thread worker;
	std::atomic<bool> workerIsRunning = false;
	std::atomic<bool> workerHasStopped = false;

	std::string CreateSpawnCommand(uuids::uuid& id, ctti_t type, std::string_view& worldID);

	std::string CreateDestroyCommand(uuids::uuid& id, std::string_view& worldID);

public:
	struct CommandCode {
		enum {
			Spawn = 1,
			Destroy,
			RPC
		};
	};
};

}
