#pragma once
#include "DataStructures.hpp"
#include "CTTI.hpp"
#include "SpinLock.hpp"
#include "System.hpp"

namespace RavEngine{
class SystemManager{
protected:
	typedef locked_hashmap<ctti_t,Ref<System>,SpinLock> system_store;
	system_store Systems;
	
public:
	
	template<typename T>
	inline void RegisterSystem(Ref<T> r_instance) {
		Systems[CTTI<T>] = r_instance;
	}
	
	template<typename T>
	inline void UnregisterSystem() {
		Systems.erase(CTTI<T>);
	}
	
	template<typename T>
	inline bool HasSystem() const{
		return Systems.contains(CTTI<T>);
	}
	
	template<typename T>
	inline Ref<T> GetSystemOfType() const{
		return Systems.at(CTTI<T>);
	}
	
	const system_store& GetInternalStorage() const{
		return Systems;
	}
	
	void Clear(){
		Systems.clear();
	}
};
}
