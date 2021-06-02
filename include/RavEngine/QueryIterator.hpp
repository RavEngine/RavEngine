#pragma once
#include "Ref.hpp"
#include "World.hpp"
#include "AccessType.hpp"
#include <cstddef>
#include <utility>

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

		/**
		* Helper method used in fold expression for querying everything within a QueryIterator
		*/
		template<typename T_inst, size_t n>
		inline constexpr void WriteOne(std::array<Entity::entry_type*, n>& arr, int& i) const {
			arr[i++] = &e->GetAllComponentsOfType<T_inst>();
		}

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

				if constexpr (n_args > 0)
				{
					int i = 0;
					(WriteOne<A...>(query_results,i));	//fold expression which does all queries for this type if there are multiple
				}

				// does the check pass?
				bool passesCheck = true;
				for (const auto& queryres : query_results) {
					if (queryres->size() == 0) {
						passesCheck = false;
						break;
					}
				}

				if (passesCheck) {

					// simple case for single template argument
					if constexpr (n_args == 0) {
						auto ptr = *(query_results[0]->begin());
						system->Tick(fpsScale, std::static_pointer_cast<T>(ptr));
					}
					else {
						//TODO: implement variadic
						//constexpr auto types = args_t<&System::Tick>{};
					}
					static_assert(n_args == 0, "variadic query iterator not fully implemented");
				}

			}
		}
	};

	template<typename T, typename ... A>
	class QueryIteratorOR : public QueryIterator<T> {

	};
}
