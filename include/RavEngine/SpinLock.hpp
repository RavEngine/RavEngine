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
    inline void lock(){
		while(flag.test_and_set());
	}
	
    inline void unlock(){
		flag.clear();
	}
    
    // constructors and operators
    SpinLock(){};
    //SpinLock(SpinLock&& other){}
    
    // copy-assign or copy-construct a lock does NOT
    // copy its state. These exist as conveniences for other things. 
    SpinLock(const SpinLock& other){}
    inline void operator=(const SpinLock& other){}
};

// do not dynamic allocate
template<typename T>
struct RAIILock {
    T& lock;

    RAIILock(T& lock) : lock(lock) {
        lock.lock();
    }
    ~RAIILock() {
        lock.unlock();
    }
};

}
