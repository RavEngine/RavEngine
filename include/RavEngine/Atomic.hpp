#pragma once
#include <atomic>

template<typename T>
class Atomic{
public:
	//default
	Atomic(){}
	
	//from value
	Atomic(T v){
		val.store(v);
	}
	
	//copy
	Atomic(const Atomic<T>& other) : Atomic(other){}
	
	//copy assignment
	Atomic& operator=(const Atomic<T> other){
		if (&other == this){
			return *this;
		}
		val.store(other);
		return *this;
	}
	
	//conversion operator
	operator T () const{
		return val.load();
	}
private:
	std::atomic<T> val;
};
