#pragma once
#include <atomic>
#include <thread>

namespace RavEngine{

class NetworkBase{
protected:
	std::thread worker;
	std::atomic<bool> workerIsRunning = false;
	std::atomic<bool> workerHasStopped = false;
};

}
