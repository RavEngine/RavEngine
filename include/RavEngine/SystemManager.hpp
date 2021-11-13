#pragma once
#include "DataStructures.hpp"
#include "CTTI.hpp"
#include "SpinLock.hpp"
#include "System.hpp"
#include <chrono>
#include "Ref.hpp"
#include "Function.hpp"
#include <taskflow/taskflow.hpp>
#include "ComponentStore.hpp"

#include <boost/function_types/function_type.hpp>
#include <boost/function_types/parameter_types.hpp>
#include <boost/function_types/result_type.hpp>
#include <boost/function_types/function_arity.hpp>
#include <boost/hana/ext/std/integer_sequence.hpp>
#include <boost/hana/for_each.hpp>
#include <utility>

namespace RavEngine{

	static const System::list_type SystemEntry_empty;
	typedef std::chrono::high_resolution_clock clock_t;

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
	inline const System::list_type& MustRunBefore_impl(const Ref<T>&) {
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
	inline const System::list_type& MustRunAfter_impl(const Ref<T>&) {
		return SystemEntry_empty;
	}

template<typename World>
struct SystemEntry{

	// function type information extraction
	template <typename FuncType>
	using Arity = boost::function_types::function_arity<FuncType>;
	template <typename FuncType>
	using ResultType = typename boost::function_types::result_type<FuncType>::type;
	template <typename FuncType, size_t ArgIndex>
	using ArgType = typename boost::mpl::at_c<boost::function_types::parameter_types<FuncType>, ArgIndex>::type;
	template <typename Func, typename IndexSeq>
	struct ArgExtractor;
	template <typename Func, size_t... Inds>
	struct ArgExtractor<Func, std::integer_sequence<size_t, Inds...> >
	{
		// this wrapper enables expanding into the function
//		template<typename System, size_t n, auto ... Is>
//		inline constexpr void TickWrapper(float fpsScale, const Ref<System>& system, const Array<const Entity::entry_type*, n>& query_results, std::integer_sequence<int, Is...>) const {
//			system->Tick(fpsScale, std::static_pointer_cast<typename ArgType<Func, Is + 2>::element_type>(*query_results[Is]->begin())...);
//		}

		// evaluates an entity to determine if it passes the query test, and calls tick if it does
		template<typename System>
		inline const void TickEntity(const Ref<Component>& c, World* world, const Ref<System>& system) const {
//			Ref<Entity> e = c->GetOwner().lock();
//			if (e) {
//				constexpr size_t n_args = sizeof ... (Inds) - 2;	// number of types in variadic
//				static_assert(n_args > 0, "System must take at least one component parameter");
//                Array<const Entity::entry_type*, n_args> query_results;
//
//				constexpr auto indseq = std::make_integer_sequence<int, n_args>();
//				boost::hana::for_each(indseq, [&](const auto i){
//                    if (auto& coms = e->GetAllComponentsOfType<typename ArgType<Func, i+2>::element_type>()){
//                        query_results[i] = &coms.value();
//                    }
//                    else{
//                        query_results[i] = nullptr;
//                    }
//				});
//
//				// does the check pass?
//				bool passesCheck = true;
//				for (const auto& queryres : query_results) {
//					if (queryres->size() == 0) {
//						passesCheck = false;
//						break;
//					}
//				}
//
//				if (passesCheck) {
//					auto fpsScale = world->GetCurrentFPSScale();
//					TickWrapper(fpsScale, system, query_results, indseq);
//				}
//			}
		}
		
		// update iterators
		inline constexpr auto UpdateQuery(World* world) const
		{
            //TODO: FIX
			// do query
//            if(auto& query = world->template GetAllComponentsOfType<typename ArgType<Func,2>::element_type>()){
//                // return updated iterators
//                return std::make_pair(query.value().begin(), query.value().end());
//            }
//            else{
//                return std::make_pair(world->emptyContainer.begin(), world->emptyContainer.end());
//            }
		}
	};

	const Function<std::pair<tf::Task, tf::Task>(ctti_t, iter_map&, tf::Taskflow&, World*)> QueryTypes;
	const Function<const System::list_type&()> MustRunBefore;
	const Function<const System::list_type&()> MustRunAfter;

	template<typename T>
	SystemEntry(Ref<T> system) :
		QueryTypes([system](ctti_t sys_ID, iter_map& iterator_map, tf::Taskflow& masterTasks, World* world) -> std::pair<tf::Task, tf::Task> {
            //TODO: FIX
//			// get the function parameter types
//
//			auto MakeArgExtractor = [&](auto f) {
//				return ArgExtractor<decltype(f), std::make_index_sequence<Arity<decltype(f)>::value>>();
//			};
//			auto argex = MakeArgExtractor(&T::Tick);
//			auto begin_end = argex.UpdateQuery(world);
//
//			// iterator baseline set
//			iterator_map[sys_ID] = { begin_end.first, begin_end.second };
//
//			// iterator updates
//			auto update = masterTasks.emplace([&iterator_map, argex, sys_ID,world]() mutable -> void {
//				auto begin_end = argex.UpdateQuery(world);
//				iterator_map[sys_ID] = { begin_end.first,begin_end.second };
//			});

			// the actual tick
//			auto mainTick = masterTasks.for_each(std::ref(iterator_map[sys_ID].begin), std::ref(iterator_map[sys_ID].end), [=](const Ref<Component>& c) {
//				argex.TickEntity(c, world, system);	//TODO: currentFPSScale?
//			});
//			update.precede(mainTick);
//			return std::make_pair(mainTick,update);
			return std::pair<tf::Task, tf::Task>();
		}),
		MustRunBefore([system]() -> const System::list_type&{
			return MustRunBefore_impl<T>(system);
		}),
		MustRunAfter([system]() -> const System::list_type&{
			return MustRunAfter_impl(system);
		})
		{}
};

template<typename World>
class SystemManager{
public:
	struct TimedSystem{
		SystemEntry<World> system;
		std::chrono::duration<double, std::micro> interval;
		std::chrono::time_point<clock_t> last_timestamp = clock_t::now();
	};
	
protected:
	typedef locked_node_hashmap<ctti_t,SystemEntry<World>,SpinLock> system_store;
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
		Systems.insert(std::make_pair(CTTI<T>(),SystemEntry<World>(r_instance)));
		graphNeedsRebuild = true;
	}

	/**
	* Construct a system to tick every frame
	*/
	template<typename T, typename ... A>
	inline void EmplaceSystem(A ... args) {
		Systems.insert(std::make_pair(CTTI<T>(), SystemEntry<World>(std::make_shared<T>(args...))));
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
	* Construct a system to tick every frame
	*/
	template<typename T, typename interval_t, typename ... A>
	inline void EmplaceTimedSystem(const interval_t& interval, A ... args) {
		TimedSystems.insert(std::make_pair(CTTI<T>(), TimedSystem{ std::make_shared<T>(args...), std::chrono::duration_cast<decltype(TimedSystem::interval)>(interval) }));
		graphNeedsRebuild = true;
	}
	
	/**
	 Change a timed system's interval
	 @param interval the interval to tick the system. This is not obeyed exactly. The closest tick that satisfies >= interval will schedule this system.
	 */
	template<typename T, typename interval_t>
    constexpr inline void SetTimedSystemInterval(const interval_t& interval){
		TimedSystems[CTTI<T>()].interval = std::chrono::duration_cast<decltype(TimedSystem::interval)>(interval);
	}
	
	/**
	 Unregister a timed system
	 */
	template<typename T>
    constexpr inline void UnregisterTimedSystem(){
		TimedSystems.erase(CTTI<T>());
		graphNeedsRebuild = true;
	}
	
	/**
	 Unregister a System that ticks every frame.
	 */
	template<typename T>
    constexpr inline void UnregisterSystem() {
		Systems.erase(CTTI<T>());
		graphNeedsRebuild = true;
	}
	
	/**
	 @return True if a system of the passed static type is contained
	 */
	template<typename T>
    constexpr inline bool HasSystem() const{
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
    constexpr inline const system_store& GetAlwaysTickSystems() const{
		return Systems;
	}
	
	/**
	 @return systems that tick on intervals. For internal use only
	 */
    constexpr inline timed_system_store& GetTimedTickSystems(){
		return TimedSystems;
	}
	
	/**
	 Unregister all systems of all types
	 */
     constexpr inline void Clear(){
		Systems.clear();
		TimedSystems.clear();
		graphNeedsRebuild = true;
	}
};
}
