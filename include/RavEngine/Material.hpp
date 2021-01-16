#pragma once
#include "SharedObject.hpp"
#include "RenderEngine.hpp"
#include "DataStructures.hpp"
#include <bgfx/bgfx.h>
#include "glm/gtc/type_ptr.hpp"
#include "Common3D.hpp"
#include "CTTI.hpp"
#include "Debug.hpp"

namespace RavEngine {

	/**
	Represents the interface to a shader. Subclass to create more types of material and expose more abilities.
	*/
	class Material : public SharedObject {
	public:
		/**
		Create the default material. Override this constructor in subclasses, and from that, invoke the protected constructor.
		*/

		virtual ~Material() {}

		const std::string& GetName() {
			return name;
		}
		
		/**
		Enqueue commands to execute on the GPU
		@param commands the command buffer to write to
		*/
		void Draw(const bgfx::VertexBufferHandle& vertexBuffer, const bgfx::IndexBufferHandle& indexBuffer, int view = 0);
		
		/**
		 Static singleton for managing materials
		 */
		class Manager {
			friend class Material;
		protected:
			//materials are keyed by their shader name
			typedef locked_hashmap<ctti_t, Ref<RavEngine::Material>,SpinLock> MaterialStore;
			static MaterialStore materials;
		
			static bool HasMaterialByTypeIndex(const ctti_t);
			
		public:
			
			/**
			 Add a material to the structure
			 @param mat the material to register
			 @throws if a material with the same type is already registered
			 */
			template<typename T>
			static void RegisterMaterial(Ref<T> mat)
			{
				//check if material is already registered
				if (HasMaterialByType<T>()) {
					Debug::Fatal("Material with type is already allocated! Use GetMaterialByType to get it.");
				}
				
				materials.insert(std::make_pair(CTTI<T>, mat));
			}
			/**
			 Gets a material with a given name, casted to a particular type.
			 @param name the name of the material to query
			 @return the material if it exists, or a null reference if there is none
			 */
			template<typename T>
			static Ref<T> GetMaterialByType() {
				Ref<T> mat;
				mat.setNull();
				auto t = CTTI<T>;
				try{
					mat = materials.at(t);
				}
				catch(std::exception&){}
				return mat;
			}
			
			template<typename T>
			static inline bool HasMaterialByType(){
				return HasMaterialByTypeIndex(CTTI<T>);
			}
			
			/**
			 Mark a material for deletion by name. The material will remain allocated until its last reference is released.
			 @param T the type to remove.
			 */
			template<typename T>
			static inline void UnregisterMaterialByType(){
				materials.erase(CTTI<T>);
			}
			
			
			/**
			 Mark a material for deletion by reference. The material will remain allocated until its last reference is released.
			 @param material the material to remove
			 */
			template<typename T>
			static inline void UnregisterMaterial(Ref<T> material){
				UnregisterMaterialByType<T>();
			}
			
			/**
			 Unregister ALL loaded materials
			 */
			static inline void RemoveAll(){
				materials.clear();
			}
			
			/**
			 Helper to get a material by type. If one is not allocated, it will be created. Supports constructors via parameter pack
			 @param args arguments to pass to material constructor if needed
			 */
			template<typename T, typename ... A>
			static Ref<T> AccessMaterialOfType(A ... args){
				auto t = CTTI<T>;
				if (materials.contains(t)){
					return std::static_pointer_cast<T>(materials.at(t));
				}
				else{
					Ref<T> mat(std::make_shared<T>(args...));
					materials.insert(std::make_pair(t,mat));
                    return mat;
				}
			}
		};

	protected:
		std::string name;

		//trying to create a material that already exists will throw an exception
		Material(const std::string& name);
		
		bgfx::ProgramHandle program;
	};

	//for type conversions, do not use directly
	class MaterialInstanceBase : public SharedObject {
	public:
		virtual void Draw(const bgfx::VertexBufferHandle& vertexBuffer, const bgfx::IndexBufferHandle& indexBuffer, const matrix4& worldmatrix, int view = 0) = 0;
	};

	/**
	* Represents the settings of a material. Subclass to expose more properties.
	*/
	template<typename T>
	class MaterialInstance : public MaterialInstanceBase {
	protected:
		/**
		* Allows you to perform work before Draw executes. Use this to bind uniforms.
		*/
		virtual void DrawHook() {};
	public:
		virtual ~MaterialInstance() {}
		inline void Draw(const bgfx::VertexBufferHandle& vertexBuffer, const bgfx::IndexBufferHandle& indexBuffer, const matrix4& worldmatrix, int view = 0) override{
			DrawHook();
			float transmat[16];
            copyMat4((const decimalType*)glm::value_ptr(worldmatrix), transmat);
			bgfx::setTransform(transmat);
			mat->Draw(vertexBuffer, indexBuffer, view);
		}
	protected:
		MaterialInstance(Ref<T> m) : mat(m) {}
		Ref<T> mat;
	};
}
