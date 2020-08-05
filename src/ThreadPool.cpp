#include "ThreadPool.h"

// the destructor joins all threads
ThreadPool::~ThreadPool() {
	{
		std::unique_lock<std::mutex> lock(queue_mutex);
		stop = true;
	}
	condition.notify_all();
	for (std::thread& worker : workers) {
		worker.join();
	}
}