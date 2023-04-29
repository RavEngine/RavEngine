#pragma once
#include "Ref.hpp"
#include "WeakRef.hpp"
#include "DataStructures.hpp"
#include "glm/gtc/type_ptr.hpp"
#include "Common3D.hpp"
#include "CTTI.hpp"
#include "Debug.hpp"
#include "Manager.hpp"
#include <RGL/RGL.hpp>

namespace RavEngine {

	/**
	Represents the interface to a shader. Subclass to create more types of material and expose more abilities.
	*/
	class Material : public AutoCTTI {
	public:
		template<typename T>
		friend class MaterialInstance;
		/**
		Create the default material. Override this constructor in subclasses, and from that, invoke the protected constructor.
		*/

		virtual ~Material() {}

		const std::string& GetName() {
			return name;
		}
		
		/**
		 Static singleton for managing materials
		 */
		class Manager {
		public:
			
			/**
			 Helper to get a material by type. If one is not allocated, it will be created. Supports constructors via parameter pack
			 @param args arguments to pass to material constructor if needed
			 */
			template<typename T, typename ... A>
			static inline Ref<T> Get(A&& ... args){
                return GenericWeakReadThroughCache<ctti_t,T,false>::Get(CTTI<T>(),args...);
			}
            
            /**
             Shrink memory usage by removing expired entries
             */
            template<typename T>
            static inline void Compact(){
                GenericWeakReadThroughCache<ctti_t,T,false>::Compact();
            }
		};

	protected:
		std::string name;

		//trying to create a material that already exists will throw an exception
		Material(const std::string& name);
		
		friend class RenderEngine;
	};

	//for type conversions, do not use directly
	class MaterialInstanceBase {
	public:
        bool doubleSided = false;
	};

	/**
	* Represents the settings of a material. Subclass to expose more properties.
	*/
	template<typename T>
	class MaterialInstance : public MaterialInstanceBase {
	public:
		virtual ~MaterialInstance() {}
		
	protected:
		MaterialInstance(Ref<T> m) : mat(m) {}
		Ref<T> mat;
	};
}
