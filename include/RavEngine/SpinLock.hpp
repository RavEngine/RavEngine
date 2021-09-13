#pragma once
#include <atomic>

namespace RavEngine{

/**
  A spinlock implemented using std::atomic_flag, which uses test-and-set instead of kernel calls.
 */
class SpinLock{
	std::atomic_flag flag
#ifndef _WIN32
		ATOMIC_FLAG_INIT
#endif
		;
public:
    constexpr inline void lock(){
		while(flag.test_and_set());
	}
	
    constexpr inline void unlock(){
		flag.clear();
	}
};

}
