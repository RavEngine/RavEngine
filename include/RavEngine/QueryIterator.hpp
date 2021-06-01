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

		inline void DoQuery(World* world) {
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
		inline void TickEntity(Ref<RavEngine::Component> c, float fpsScale, Ref<System> system) const{
			Ref<Entity> e = c->getOwner().lock();
			if (e) {
				constexpr size_t n_args = sizeof ... (A);	// number of types in variadic (query T separately)
				std::array<Entity::entry_type*, n_args + 1> query_results;
				query_results[0] = &e->GetAllComponentsOfType<T>();

				/*if constexpr (n_args > 0)
				{
					int i = 0;
					const auto doOneType = [&]() {
						query_results[i++] = 
					};
					(query_results[i++] = &e->GetAllComponentsOfType<A>(), ...);
				}*/

				// does the check pass?
				bool passesCheck = true;
				for (const auto& queryres : query_results) {
					if (queryres->size() == 0) {
						passesCheck = false;
						break;
					}
				}

				if (passesCheck) {
					//constexpr auto types = args_t<System::Tick>{};
					system->Tick(fpsScale, *query_results[0]->begin(), CTTI<T>());
				}

			}
		}
	};

	template<typename T, typename ... A>
	class QueryIteratorOR : public QueryIterator<T> {

	};
}
