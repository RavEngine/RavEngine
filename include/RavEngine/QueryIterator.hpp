#pragma once
#include "Ref.hpp"
#include "World.hpp"

namespace RavEngine {

	/**
	 This class is responsible for performing the entity queries
	 */

	template<typename T>
	class QueryIterator {
		World::entry_type* QueryResult = nullptr;

	protected:
		//templates to extract function parameter tyes
		template<class ...> struct types { using type = types; };
		template<class Sig> struct args;
		template<class R, class...Args> struct args<R(Args...)> :types<Args...> {};
		template<class Sig> using args_t = typename args<Sig>::type;
	public:
		inline std::pair<World::entry_type::iterator, World::entry_type::iterator> GetIterators() const{
			return std::make_pair(QueryResult->begin(), QueryResult->end());
		}

		inline void DoQuery(const Ref<World>& world) {
			//always query by first type name
			auto& results = world->GetAllComponentsOfType<T>();
			this->QueryResult = &results;
		}

		const decltype(QueryResult) GetQueryResult() const {
			return QueryResult;
		}
	};

	template<typename T, typename ... A>
	class QueryIteratorAND : public QueryIterator<T> {
	public:
		template<typename System>
		inline void TickEntity(Ref<Component> c, float fpsScale, Ref<System> system) const{
			// does the entity pass this iterator?

			//constexpr auto types = args_t<System::Tick>{};
		}
	};

	template<typename T, typename ... A>
	class QueryIteratorOR : public QueryIterator<T> {

	};
}
