#pragma once
#include <atomic>

namespace RavEngine{

/**
  A spinlock implemented using std::atomic_flag, which uses test-and-set instead of kernel calls.
 */
class SpinLock{
	std::atomic_flag flag;
public:
#ifndef _WIN32
	SpinLock() : flag(ATOMIC_FLAG_INIT) {}	//required on macOS but incompatible with windows
#endif
	void lock(){
		while(flag.test_and_set());
	}
	
	void unlock(){
		flag.clear();
	}
};

}
