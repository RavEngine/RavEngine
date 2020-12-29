#pragma once
#include <phmap.h>
#include "SharedObject.hpp"
#include <typeindex>
#include "Component.hpp"
#include <functional>
#include "Locked_Hashmap.hpp"

//macro for checking type in template
#define C_REF_CHECK static_assert(std::is_base_of<RavEngine::Component, T>::value, "Template parameter must be a Component subclass");

namespace RavEngine{
	template<typename lock>
	class ComponentStore : public SharedObject {
	protected:
		typedef locked_hashset<Ref<RavEngine::Component>,lock> entry_type;
		typedef locked_hashmap<std::type_index, entry_type,lock> ComponentStructure;
		
		WeakRef<ComponentStore> parent;	//in entities, this is the World
		
		ComponentStructure components;
		ComponentStructure componentsRedundant;
		
		/**
		For internal use only.
		@param type the type_index to query for
		@return all the components of a class or its base classes to a type index
		*/
		template<typename T>
		inline phmap::flat_hash_set<Ref<T>> GetAllComponentsOfSubclassTypeIndex(const std::type_index& type) {
			C_REF_CHECK
			//query both types
			auto& toplevel = components[type];
			auto& comp = componentsRedundant[type];
			
			//insert into
			phmap::flat_hash_set<Ref<T>> cpy;
			cpy.insert(toplevel.begin(), toplevel.end());
			cpy.insert(comp.begin(), comp.end());
			return cpy;
			
		}
		
		/**
		 For internal use only.
		 @param type the type_index to query for.
		 @return all the components of a type index. Does NOT search base classes
		 */
		template<typename T>
		inline phmap::flat_hash_set<Ref<T>> GetAllComponentsOfTypeIndex(const std::type_index& index) {
			C_REF_CHECK
			auto& comp = components[index];
			phmap::flat_hash_set<Ref<T>> cpy;
			cpy.insert(comp.begin(),comp.end());
			return cpy;
		}

		/**
		 Fast path for world ticking
		 */
		inline const entry_type& GetAllComponentsOfTypeIndexFastPath(const std::type_index& index){
			return components[index];
		}
		
		/**
		Fast path for world ticking
		 */
		inline const entry_type& GetAllComponentsOfTypeIndexSubclassFastPath(const std::type_index& index){
			return componentsRedundant[index];
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
			return components[std::type_index(typeid(T))];
		}
		
		/**
		 Fast path for world ticking
		 */
		template<typename T>
		inline const entry_type& GetAllComponentsOfTypeSubclassFastPath(){
			return componentsRedundant[std::type_index(typeid(T))];
		}
		
		/**
		Remove all components from this store
		*/
		inline void clear() {
			components.clear();
			componentsRedundant.clear();
		}

		/**
		 Add a component to this Entity
		 @param componentRef the component to add to this Entity. Must pass a Ref class to a Component
		 */
		template<typename T>
		inline Ref<T> AddComponent(Ref<T> componentRef) {
			C_REF_CHECK
				components[std::type_index(typeid(T))].insert(componentRef);

			//add redundant types
			for (const auto& alt : T::GetQueryTypes()) {
				componentsRedundant[alt].insert(componentRef);
			}
			OnAddComponent(componentRef);
			if(!parent.isNull()){
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
				auto& vec = components[std::type_index(typeid(T))];
			if (vec.empty()) {
				//try base classes
				return GetComponentOfSubclass<T>();
			}
			else {
				return *vec.begin();
			}
		}

		/**
		 Determines if this Entity has a component of a type. Pass in the ref type. Investigates only base classes the passed ref type.
		 @return true if this entity has a component of type ref
		 */
		template<typename T>
		inline bool HasComponentOfType() {
			C_REF_CHECK
			return (!components[std::type_index(typeid(T))].empty()) || (HasComponentOfSubclass<T>());
		}

		/**
		 Determines if this Entity has a component of a base type. Only considers base classes of ref. Pass in the ref type.
		 @return true if this entity has a component of type ref
		 */
		template<typename T>
		inline bool HasComponentOfSubclass() {
			C_REF_CHECK
			return !componentsRedundant[std::type_index(typeid(T))].empty();
		}

		/**
		 * Gets the first stored reference of a subclass type. Only use if you know the component you're querying is unique
		 @return the component of the base class if found
		 @throws runtime_error if no component of type is found on this Entity. If you are not sure, call HasComponentOfType() first
		 @throws bad_cast if a cast fails
		 @note If you have multiple entries that can satisfy this type, which one you recieve is not guarenteed!
		 */
		template<typename T>
		inline Ref<T> GetComponentOfSubclass() {
			C_REF_CHECK
				auto& vec = componentsRedundant[std::type_index(typeid(T))];
			if (vec.size() == 0) {
				throw std::runtime_error("No component of type");
			}
			else {
				return *vec.begin();
			}
		}

		/**
		 Gets all of the references of a base class type.
		 @return the list of all the refs of a base class type. The list may be empty.
		 */
		template<typename T>
		inline phmap::flat_hash_set<Ref<T>> GetAllComponentsOfSubclass() {
			return GetAllComponentsOfSubclassTypeIndex<T>(std::type_index(typeid(T)));
		}

		/**
		 Get all of the components of a specific type. Does NOT search subclasses.
		 Pass the ref type and the type.
		 @returns the list of only the components of the high-level type. The list may be empty
		 */
		template<typename T>
		inline phmap::flat_hash_set<Ref<T>> GetAllComponentsOfType() {
			return GetAllComponentsOfTypeIndex<T>(std::type_index(typeid(T)));
		}

		/**
		Remove a component by value
		@param component the component to remove
		*/
		template<typename T>
		inline void RemoveComponent(Ref<T> component) {
			components[std::type_index(typeid(T))].erase(component);

			//add redundant types
			for (const auto& alt : T::GetQueryTypes()) {
				componentsRedundant[alt].erase(component);
			}
			OnRemoveComponent(component);
		}

		/**
		Copy components from one componentstore into this one
		@param other the other component store to copy from
		*/
		inline void Merge(const ComponentStore& other) {
			for (const auto& c : other.components) {
				//add the componets of the current type to the global list
				components[c.first].insert(c.second.begin(), c.second.end());
				for(Ref<Component> cm : c.second){
					OnAddComponent(cm);
				}
			}
			//add to the redundant store
			for (const auto& c : other.componentsRedundant) {
				componentsRedundant[c.first].insert(c.second.begin(), c.second.end());
			}
		}

		/**
		Remove components from this store if they are also present in another
		@param other the ComponentStore to compare
		*/
		inline void Unmerge(const ComponentStore& other) {
			for (const auto& type_pair : other.components) {
				for(const auto& to_remove : type_pair.second){
					components[type_pair.first].erase(to_remove);
					OnRemoveComponent(to_remove);
				}
			}
			
			for (const auto& type_pair : other.componentsRedundant) {
				for(const auto& to_remove : type_pair.second){
					componentsRedundant[type_pair.first].erase(to_remove);
				}
			}
		}
	};
}
