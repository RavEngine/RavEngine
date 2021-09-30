#pragma once
#include <phmap.h>
#include "Component.hpp"
#include <functional>
#include "DataStructures.hpp"
#include "CTTI.hpp"
#include "Debug.hpp"
#include <optional>
#include "unordered_vector.hpp"

//macro for checking type in template
#define C_REF_CHECK static_assert(std::is_base_of<RavEngine::Component, T>::value, "Template parameter must be a Component subclass");

namespace RavEngine{
	template<class lock = phmap::NullMutex>
	class ComponentStore {
	public:
		typedef UnorderedContiguousSet<Ref<RavEngine::Component>> entry_type;
	protected:
		typedef locked_hashmap<ctti_t, entry_type,lock> ComponentStructure;
		
		WeakRef<ComponentStore> parent;	//in entities, this is the World
		
		ComponentStructure components;

		/**
		 Fast path for world ticking
		 */
		inline const entry_type& GetAllComponentsOfTypeIndexFastPath(const ctti_t index){
			return components[index];
		}
		
		/**
		 Invoked when a component is added to this component store.
		 @param comp the component that was added
		 */
		virtual void OnAddComponent(Ref<Component> comp){}
		
		/**
		 Invoked when a component is removed from this component store.
		 @param comp the component that was removed
		 */
		virtual void OnRemoveComponent(Ref<Component> comp){}
		
		virtual void CTTI_Add(Ref<Component> comp, ctti_t id){
			components[id].insert(comp);
            auto p = parent.lock();
			if (p){
				p->CTTI_Add(comp,id);
			}
		}
		
		virtual void CTTI_Remove(Ref<Component> comp, ctti_t id){
			components[id].erase(comp);
            auto p = parent.lock();
			if (p){
				p->CTTI_Remove(comp,id);
			}
		}
		
	public:
		/**
		 Fast path for world ticking
		 */
		template<typename T>
		constexpr inline entry_type& GetAllComponentsOfType(){
			return components[CTTI<T>()];
		}

        /**
         Construct a component in-place and add it to the store
         @param T template type to create
         @param args ... templated arguments to pass to constructor
         @return strong pointer to created component
         */
        template<typename T, typename ... A>
        inline Ref<T> EmplaceComponent(A ... args){
            return AddComponent<T>(std::make_shared<T>(args...));
        }
        
		/**
		 Add a component to this Entity
		 @param componentRef the component to add to this Entity. Must pass a Ref class to a Component
		 */
		template<typename T>
		inline Ref<T> AddComponent(Ref<T> componentRef) {
			C_REF_CHECK
			CTTI_Add(componentRef,CTTI<T>());
			
			//add redundant types
			for (const auto alt : T::GetQueryTypes()) {
				CTTI_Add(componentRef,alt);
			}
			OnAddComponent(componentRef);
			return componentRef;
		}

		/**
		 Get the first component of a type (including all querytypes) in this Entity. 
		 @returns an optional reference. If the component type was not found, the optional will be invalid.
		 @throws bad_cast if a cast fails
		 */
		template<typename T>
		inline std::optional<Ref<T>> GetComponent() const{
			C_REF_CHECK;
			std::optional<Ref<T>> returned;
			components.if_contains(CTTI<T>(), [&](const entry_type& vec) {
				if (!vec.empty()) {
					returned = std::static_pointer_cast<T>(*vec.begin());
				}
			});
			return returned;
		}

		/**
		 Determines if this Entity has a component of a type.
		 @return true if this entity has a component of type T
		 @note Do not use to determine if a GetComponent call will succeed, that is not threadsafe. Instead, use GetComponent directly and check if the resulting std::optional is valid.
		 */
		template<typename T>
        constexpr inline bool HasComponentOfType() const{
			C_REF_CHECK
			return components.contains(CTTI<T>());
		}


		/**
		Remove a component by value
		@param component the component to remove
		*/
		template<typename T>
		inline void RemoveComponent(Ref<T> component) {
			CTTI_Remove(component, CTTI<T>);

			//add redundant types
			for (const auto& alt : T::GetQueryTypes()) {
				CTTI_Remove(component, alt);
			}
			OnRemoveComponent(component);
		}

		/**
		Copy components from one componentstore into this one
		@param other the other component store to copy from
		*/
		inline void Merge(const ComponentStore& other) {
			UnorderedSet<Ref<Component>> invoked;
			Ref<ComponentStore> p = parent.lock();
			
			for (const auto& type_pair : other.components) {
				for(const auto& to_add : type_pair.second){
					CTTI_Add(to_add, type_pair.first);
					
					if (!invoked.contains(to_add)){
						OnAddComponent(to_add);
					}
					
					// reflect changes in parent
					if(p){
						if (!invoked.contains(to_add)){
							p->OnAddComponent(to_add);
						}
					}
					invoked.insert(to_add);
				}
			}
		}

		/**
		Remove components from this store if they are also present in another
		@param other the ComponentStore to compare
		*/
		inline void Unmerge(const ComponentStore& other) {
            UnorderedSet<Ref<Component>> invoked;
			Ref<ComponentStore> p = parent.lock();
			
			for (const auto& type_pair : other.components) {
				for(const auto& to_remove : type_pair.second){
					CTTI_Remove(to_remove, type_pair.first);
					
					if (!invoked.contains(to_remove)){
						OnRemoveComponent(to_remove);
					}
					
					// reflect changes in parent
					if(p){
						if (!invoked.contains(to_remove)){
							p->OnRemoveComponent(to_remove);
						}
					}
					invoked.insert(to_remove);
				}
			}
		}
	};

	struct range {
		ComponentStore<phmap::NullMutex>::entry_type::const_iterator begin, end;
	};
	typedef locked_node_hashmap<ctti_t, range, phmap::NullMutex> iter_map;
}
