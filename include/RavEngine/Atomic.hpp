#pragma once
#include <atomic>

template<typename T, typename atomic = std::atomic<T>>
class Atomic{
public:
	//default
	Atomic(){}
	
	//from value
	Atomic(T v){
		val.store(v);
	}
	
	//copy
	Atomic(const Atomic<T,atomic>& other) : Atomic(other){}
	
	//copy assignment
	inline Atomic& operator=(const Atomic<T,atomic> other){
		if (&other == this){
			return *this;
		}
		val.store(other,std::memory_order_relaxed);
		return *this;
	}
	
	//conversion operator
	inline operator T () const{
		return val.load(std::memory_order_relaxed);
	}
private:
	atomic val;
};
