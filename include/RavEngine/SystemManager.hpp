#pragma once
#include "DataStructures.hpp"
#include "CTTI.hpp"
#include "SpinLock.hpp"
#include "System.hpp"
#include <chrono>
#include <fmt/format.h>

namespace RavEngine{

	static const System::list_type SystemEntry_empty;

	//template existence detection
	template <typename T, typename = int>
	struct HasMustRunBefore : std::false_type { };

	template <typename T>
	struct HasMustRunBefore <T, decltype(&T::MustRunBefore, 0)> : std::true_type { };

	template<typename T, std::enable_if_t<HasMustRunBefore<T>::value, bool> = true>
	//if exists, call and return
	inline const System::list_type& MustRunBefore_impl(const Ref<T>& system)
	{
		return system->MustRunBefore();
	}

	template<typename T, std::enable_if_t<!HasMustRunBefore<T>::value, bool> = false>
	//if does not exist, return reference to empty container
	inline const System::list_type& MustRunBefore_impl(const Ref<T>&){
		return SystemEntry_empty;
	}

	template <typename T, typename = int>
	struct HasMustRunAfter : std::false_type { };

	template <typename T>
	struct HasMustRunAfter <T, decltype(&T::MustRunAfter, 0)> : std::true_type { };

	template<typename T, std::enable_if_t<HasMustRunAfter<T>::value, bool> = true>
	//if exists, call and return
	inline const System::list_type& MustRunAfter_impl(const Ref<T>& system)
	{
		return system->MustRunAfter();
	}

	template<typename T, std::enable_if_t<!HasMustRunAfter<T>::value, bool> = false>
	//if does not exist, return reference to empty container
	inline const System::list_type& MustRunAfter_impl(const Ref<T>&){
		return SystemEntry_empty;
	}

struct SystemEntry{
	const std::function<void(float,Ref<Component>,ctti_t)> Tick;
	const std::function<const System::list_type&()> QueryTypes;
	const std::function<const System::list_type&()> MustRunBefore;
	const std::function<const System::list_type&()> MustRunAfter;

	template<typename T>
	SystemEntry(Ref<T> system) :
		Tick([=](float f, Ref<Component> c, ctti_t id){
			system->Tick(f,c,id);
		}),
		QueryTypes([=]() -> const System::list_type& {
			return system->QueryTypes();
		}),
		MustRunBefore([=]() -> const System::list_type&{
			return MustRunBefore_impl<T>(system);
		}),
		MustRunAfter([=]() -> const System::list_type&{
			return MustRunAfter_impl(system);
		})
		{}
};

class SystemManager{
public:
	typedef std::chrono::high_resolution_clock clock_t;
	struct TimedSystem{
		SystemEntry system;
		std::chrono::duration<double, std::micro> interval;
		std::chrono::time_point<clock_t> last_timestamp = clock_t::now();
	};
	
protected:
	typedef locked_node_hashmap<ctti_t,SystemEntry,SpinLock> system_store;
	typedef locked_node_hashmap<ctti_t,TimedSystem,SpinLock> timed_system_store;
	
	system_store Systems;
	timed_system_store TimedSystems;
	
public:
	std::atomic<bool> graphNeedsRebuild = false;
	
	/**
	 Register a System to tick every frame
	 @param r_instance the system to tick every frame
	 */
	template<typename T>
	inline void RegisterSystem(Ref<T> r_instance) {
		Systems.insert(std::make_pair(CTTI<T>(),SystemEntry(r_instance)));
		graphNeedsRebuild = true;
	}
	
	/**
	 Register a system that ticks on an interval
	 @param r_instance the system to register
	 @param interval the interval to tick the system. This is not obeyed exactly. The closest tick that satisfies >= interval will schedule this system.
	 */
	template<typename T, typename interval_t>
	inline void RegisterTimedSystem(Ref<T> r_instance, const interval_t& interval){
		TimedSystems.insert(std::make_pair(CTTI<T>(),TimedSystem{r_instance, std::chrono::duration_cast<decltype(TimedSystem::interval)>(interval)}));
		graphNeedsRebuild = true;
	}
	
	/**
	 Change a timed system's interval
	 @param interval the interval to tick the system. This is not obeyed exactly. The closest tick that satisfies >= interval will schedule this system.
	 */
	template<typename T, typename interval_t>
	inline void SetTimedSystemInterval(const interval_t& interval){
		TimedSystems[CTTI<T>()].interval = std::chrono::duration_cast<decltype(TimedSystem::interval)>(interval);
	}
	
	/**
	 Unregister a timed system
	 */
	template<typename T>
	inline void UnregisterTimedSystem(){
		TimedSystems.erase(CTTI<T>());
		graphNeedsRebuild = true;
	}
	
	/**
	 Unregister a System that ticks every frame.
	 */
	template<typename T>
	inline void UnregisterSystem() {
		Systems.erase(CTTI<T>());
		graphNeedsRebuild = true;
	}
	
	/**
	 @return True if a system of the passed static type is contained
	 */
	template<typename T>
	inline bool HasSystem() const{
		return Systems.contains(CTTI<T>());
	}
	
	/**
	 @return the system of type
	 */
	template<typename T>
	inline Ref<T> GetSystemOfType() const{
		return Systems.at(CTTI<T>());
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
		graphNeedsRebuild = true;
	}
};
}
