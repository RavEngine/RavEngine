#pragma once
#include <phmap.h>
#include "SharedObject.hpp"
#include <typeindex>
#include "Component.hpp"
#include <functional>
#include "DataStructures.hpp"
#include "CTTI.hpp"
#include "Debug.hpp"

//macro for checking type in template
#define C_REF_CHECK static_assert(std::is_base_of<RavEngine::Component, T>::value, "Template parameter must be a Component subclass");

namespace RavEngine{
	template<class lock = phmap::NullMutex>
	class ComponentStore : public SharedObject {
	public:
		typedef locked_hashset<Ref<RavEngine::Component>,lock> entry_type;
	protected:
		typedef locked_hashmap<ctti_t, entry_type,lock> ComponentStructure;
		
		WeakRef<ComponentStore> parent;	//in entities, this is the World
		
		ComponentStructure components;
		
		/**
		 For internal use only.
		 @param type the compile-time type identifier to query for. Use RavEngine::CTTI<T> to get it.
		 @return all the components of a type index. Does NOT search base classes
		 */
		template<typename T>
		inline phmap::flat_hash_set<Ref<T>> GetAllComponentsOfTypeIndex(const ctti_t index) {
			C_REF_CHECK
			auto& comp = components[index];
			phmap::flat_hash_set<Ref<T>> cpy;
            for(const auto c : comp){
                cpy.insert(std::static_pointer_cast<T>(c));
            }
			return cpy;
		}

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
		
	public:
		/**
		 Fast path for world ticking
		 */
		template<typename T>
		inline const entry_type& GetAllComponentsOfTypeFastPath(){
			return components[CTTI<T>];
		}
		
		/**
		Remove all components from this store
		*/
		inline void clear() {
			components.clear();
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
			components[CTTI<T>].insert(componentRef);

			//add redundant types
			for (const auto alt : T::GetQueryTypes()) {
				components[alt].insert(componentRef);
			}
			OnAddComponent(componentRef);
			if(!parent.expired()){
				Ref<ComponentStore>(parent)->AddComponent(componentRef);
			}
			return componentRef;
		}

		/**
		 Get the first component of a type  in this Entity. Use as GetComponent<ComponentRef, Component>(). Investigates base classes.
		 @throws runtime_error if no component of type is found on this Entity. If you are not sure, call HasComponentOfType() first
		 @throws bad_cast if a cast fails
		 */
		template<typename T>
		inline Ref<T> GetComponent() {
			C_REF_CHECK
			auto& vec = components[CTTI<T>];
			if (vec.empty()) {
				Debug::Fatal("No component of type exists");
			}
			else {
				return std::static_pointer_cast<T>(*vec.begin());
			}
		}

		/**
		 Determines if this Entity has a component of a type. Pass in the ref type. Investigates only base classes the passed ref type.
		 @return true if this entity has a component of type ref
		 */
		template<typename T>
		inline bool HasComponentOfType() {
			C_REF_CHECK
			return components.contains(CTTI<T>);
		}


		/**
		 Get all of the components of a specific type. Does NOT search subclasses.
		 Pass the ref type and the type.
		 @returns the list of only the components of the high-level type. The list may be empty
		 */
		template<typename T>
		inline phmap::flat_hash_set<Ref<T>> GetAllComponentsOfType() {
			return GetAllComponentsOfTypeIndex<T>(CTTI<T>);
		}

		/**
		Remove a component by value
		@param component the component to remove
		*/
		template<typename T>
		inline void RemoveComponent(Ref<T> component) {
			components[CTTI<T>()].erase(component);

			//add redundant types
			for (const auto& alt : T::GetQueryTypes()) {
				components[alt].erase(component);
			}
			if(!parent.expired()){
				Ref<ComponentStore>(parent)->RemoveComponent(component);
			}
			OnRemoveComponent(component);
		}

		/**
		Copy components from one componentstore into this one
		@param other the other component store to copy from
		*/
		inline void Merge(const ComponentStore& other) {
			
			phmap::flat_hash_set<Ref<Component>> invoked;
			
			for (const auto& c : other.components) {
				//add the componets of the current type to the global list
				components[c.first].insert(c.second.begin(), c.second.end());
				Ref<ComponentStore> p = parent.lock();
				for(Ref<Component> cm : c.second){
					if (!invoked.contains(cm)){
						OnAddComponent(cm);
						if (p){
							p->OnAddComponent(cm);
						}
						invoked.insert(cm);
					}
					
				}
				// reflect changes in parent
				if(p){
					p->components[c.first].insert(c.second.begin(), c.second.end());
				}
			}
		}

		/**
		Remove components from this store if they are also present in another
		@param other the ComponentStore to compare
		*/
		inline void Unmerge(const ComponentStore& other) {
			phmap::flat_hash_set<Ref<Component>> invoked;
			
			for (const auto& type_pair : other.components) {
				for(const auto& to_remove : type_pair.second){
					components[type_pair.first].erase(to_remove);
					
					if (!invoked.contains(to_remove)){
						OnRemoveComponent(to_remove);
					}
					
					// reflect changes in parent
					Ref<ComponentStore> p = parent.lock();
					if(p){
						if (!invoked.contains(to_remove)){
							p->OnRemoveComponent(to_remove);
						}
						p->components[type_pair.first].erase(to_remove);
					}
					invoked.insert(to_remove);
				}
			}
		}
	};
}
