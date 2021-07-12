#pragma once
#include "SpinLock.hpp"

template<typename T>
class LockFreeAtomic {
	T value;
	RavEngine::SpinLock mtx;
public:

	// default
	LockFreeAtomic() {}

	// from value
	LockFreeAtomic(const T& value) : value(value) {}

	//copy assignment
	inline LockFreeAtomic& operator=(const LockFreeAtomic<T>& other) {
		if (&other == this) {
			return *this;
		}
		
		store(other.value);

		return *this;
	}

	//conversion operator
	inline operator T () {
		return load();
	}

	inline T load() {
		T temp;
		mtx.lock();
		temp = value;
		mtx.unlock();
		return temp;
	}

	inline void store(const T& newvalue) {
		mtx.lock();
		value = newvalue;
		mtx.unlock();
	}	
};