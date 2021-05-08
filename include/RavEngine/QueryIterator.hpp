#pragma once
#include "Ref.hpp"
#include "World.hpp"

namespace RavEngine{

class Entity;

/**
 This class is responsible for performing the entity queries
 */

template<typename T>
class QueryIterator{
	World::entry_type* QueryResult = nullptr;
public:
	inline void DoQuery(const Ref<World>& world){
		//always query by first type name
		auto& results = world->GetAllComponentsOfType<T>();
		this->QueryResult = &results;
	}
};

template<typename T, typename ... A>
class QueryIteratorAND : public QueryIterator<T>{
public:
//	inline tf::Task Batch(const tf::TaskFlow& masterTasks){
//		auto func = [=](Ref<Component> c) {
//			system.Tick(getCurrentFPSScale(), c, query);
//		};
//
//		return masterTasks.for_each(std::ref(iterator_map[ID].begin), std::ref(iterator_map[ID].end), func);
//	}
//};
};
}
