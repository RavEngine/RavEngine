#include <mutex>
#include <iostream>

namespace LockLogger {
	std::mutex mtx;

	void Log(const std::string& msg) {
		mtx.lock();
		std::cout << msg << std::endl;
		mtx.unlock();
	}

	void LogError(const std::string& msg) {
		mtx.lock();
		std::cerr << msg << std::endl;
		mtx.unlock();
	}

};