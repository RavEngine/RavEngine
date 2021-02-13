#pragma once
#include "DataStructures.hpp"
#include "CTTI.hpp"
#include "SpinLock.hpp"
#include "System.hpp"
#include <chrono>

namespace RavEngine{
class SystemManager{
public:
	typedef std::chrono::high_resolution_clock clock_t;
	struct TimedSystem{
		Ref<System> system;
		std::chrono::duration<double, std::micro> interval;
		std::chrono::time_point<clock_t> last_timestamp = clock_t::now();
	};
	
protected:
	typedef locked_hashmap<ctti_t,Ref<System>,SpinLock> system_store;
	typedef locked_hashmap<ctti_t,TimedSystem,SpinLock> timed_system_store;
	
	system_store Systems;
	timed_system_store TimedSystems;
	
public:
	/**
	 Register a System to tick every frame
	 @param r_instance the system to tick every frame
	 */
	template<typename T>
	inline void RegisterSystem(Ref<T> r_instance) {
		Systems[CTTI<T>] = r_instance;
	}
	
	/**
	 Register a system that ticks on an interval
	 @param r_instance the system to register
	 @param interval the interval to tick the system. This is not obeyed exactly. The closest tick that satisfies >= interval will schedule this system.
	 */
	template<typename T, typename interval_t>
	inline void RegisterTimedSystem(Ref<T> r_instance, const interval_t& interval){
		TimedSystems[CTTI<T>] = {r_instance, std::chrono::duration_cast<decltype(TimedSystem::interval)>(interval)};
	}
	
	/**
	 Change a timed system's interval
	 @param interval the interval to tick the system. This is not obeyed exactly. The closest tick that satisfies >= interval will schedule this system.
	 */
	template<typename T, typename interval_t>
	inline void SetTimedSystemInterval(const interval_t& interval){
		TimedSystems[CTTI<T>].interval = std::chrono::duration_cast<decltype(TimedSystem::interval)>(interval);
	}
	
	/**
	 Unregister a timed system
	 */
	template<typename T>
	inline void UnregisterTimedSystem(){
		TimedSystems.erase(CTTI<T>);
	}
	
	/**
	 Unregister a System that ticks every frame.
	 */
	template<typename T>
	inline void UnregisterSystem() {
		Systems.erase(CTTI<T>);
	}
	
	/**
	 @return True if a system of the passed static type is contained
	 */
	template<typename T>
	inline bool HasSystem() const{
		return Systems.contains(CTTI<T>);
	}
	
	/**
	 @return the system of type
	 */
	template<typename T>
	inline Ref<T> GetSystemOfType() const{
		return Systems.at(CTTI<T>);
	}
	
	/**
	 @return systems that tick every frame. For internal use only
	 */
	inline const system_store& GetAlwaysTickSystems() const{
		return Systems;
	}
	
	/**
	 @return systems that tick on intervals. For internal use only
	 */
	inline timed_system_store& GetTimedTickSystems(){
		return TimedSystems;
	}
	
	/**
	 Unregister all systems of all types
	 */
	inline void Clear(){
		Systems.clear();
		TimedSystems.clear();
	}
};
}
