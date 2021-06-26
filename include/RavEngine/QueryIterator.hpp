#pragma once
#include "Ref.hpp"
#include "World.hpp"
#include "AccessType.hpp"

namespace RavEngine {

	/**
	 This class is responsible for performing the entity queries
	 */

	template<typename T>
	class QueryIterator {
		World::entry_type* QueryResult = nullptr;

	protected:

		/**
		* Helper method used in fold expression for querying everything within a QueryIterator
		*/
		template<typename T_inst, size_t n>
		inline const void WriteOne(std::array<Entity::entry_type*, n>& arr, const Ref<Entity> e, int& i) const {
			arr[i++] = &(e->GetAllComponentsOfType<T_inst>());
		}

		template<typename T>
		inline const Ref<T> ConvertOne(Entity::entry_type* input) {
			return std::static_pointer_cast<T>(*input->begin());
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
		inline void TickWrapper(Ref<System> system, A ... args) {
			system->Tick(1.0, args...);
		}

		template<typename System>
		inline void unpack(Ref<System> system, Entity::entry_type** arr) {
			TickWrapper(system, (std::static_pointer_cast<A>(*++arr))...);
		}

		template<typename System>
		inline void TickEntity(Ref<RavEngine::Component> c, World* world, Ref<System> system) const{
			Ref<Entity> e = c->getOwner().lock();
			if (e) {
				constexpr size_t n_args = sizeof ... (A);	// number of types in variadic (query T separately)
				std::array<Entity::entry_type*, n_args + 1> query_results;
				query_results[0] = &e->GetAllComponentsOfType<T>();

				if constexpr (n_args > 0)
				{
					int i = 1;
					(this->template WriteOne<A>(query_results, e, i), ... );	//fold expression which does all queries for this type if there are multiple
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
					auto fpsScale = world->getCurrentFPSScale();
					unpack(system, query_results.data());

					// this is really dumb and cannot remain like this
					// 
					// TODO: some variadic template magic?
					//system->Tick(fpsScale,(ConvertOne<A>(query_results),...))
					/*if constexpr (n_args == 0) {
						system->Tick(fpsScale, 
							*(query_results[0]->begin())
						);
					}
					else if constexpr (n_args == 1) {
						system->Tick(fpsScale,
							*query_results[0]->begin(),
							*query_results[1]->begin()
						);
					}
					else if constexpr (n_args == 2) {
						system->Tick(fpsScale,
							*query_results[0]->begin(),
							*query_results[1]->begin(),
							*query_results[2]->begin()
						);
					}
					else if constexpr (n_args == 3) {
						system->Tick(fpsScale,
							*query_results[0]->begin(),
							*query_results[1]->begin(),
							*query_results[2]->begin(),
							*query_results[3]->begin()
						);
					}
					else {
						static_assert(n_args > 3, "Too many arguments to system!");
					}*/
				}

			}
		}
	};

	template<typename T, typename ... A>
	class QueryIteratorOR : public QueryIterator<T> {

	};
}
