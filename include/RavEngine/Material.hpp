#pragma once
#include "SharedObject.hpp"
#include "RenderEngine.hpp"
#include <unordered_map>
#include "Buffers.hpp"
#include "SpinLock.hpp"

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
		Set the world space matrix to use when rendering this material
		*/
		void SetTransformMatrix(const matrix4&);

		/**
		Enqueue commands to execute on the GPU
		@param commands the command buffer to write to
		*/
		void Draw(const VertexBuffer& vertexBuffer, const IndexBuffer& indexBuffer);
		
		/**
		 Static singleton for managing materials
		 */
		class Manager {
			friend class Material;
		protected:
			//materials are keyed by their shader name
			typedef std::unordered_map<std::type_index, Ref<RavEngine::Material>> MaterialStore;
			static MaterialStore materials;
			static SpinLock mtx;
			
			static matrix4 projectionMatrix;
			static matrix4 viewMatrix;
			
			//for internal use only
			static bool HasMaterialByTypeIndex(const std::type_index&);
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
					throw std::runtime_error("Material with type is already allocated! Use GetMaterialByType to get it.");
				}
				
				std::type_index t(typeid(T));
				mtx.lock();
				materials.insert(std::make_pair(t, mat));
				mtx.unlock();
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
				std::type_index t(typeid(T));
				mtx.lock();
				try{
					mat = materials.at(t);
				}
				catch(std::exception&){}
				mtx.unlock();
				return mat;
			}
			
			template<typename T>
			static bool HasMaterialByType(){
				return HasMaterialByTypeIndex(typeid(T));
			}
			
			/**
			 Mark a material for deletion by name. The material will remain allocated until its last reference is released.
			 @param T the type to remove.
			 */
			template<typename T>
			static void UnregisterMaterialByType(){
				mtx.lock();
				materials.erase(typeid(T));
				mtx.unlock();
			}
			
			
			/**
			 Mark a material for deletion by reference. The material will remain allocated until its last reference is released.
			 @param material the material to remove
			 */
			template<typename T>
			static void UnregisterMaterial(Ref<T> material){
				UnregisterMaterialByType<T>();
			}
			
			/**
			 Helper to get a material by type. If one is not allocated, it will be created. Supports constructors via parameter pack
			 @param args arguments to pass to material constructor if needed
			 */
			template<typename T, typename ... A>
			static Ref<T> AccessMaterialOfType(A ... args){
				Ref<T> mat;
				mtx.lock();
				std::type_index t(typeid(T));
				if (materials.find(t) != materials.end()){
					mat = materials.at(t);
				}
				else{
					mat = new T(args...);
					materials.insert(std::make_pair(t,mat));
				}
				mtx.unlock();
				return mat;
			}
			
			/**
			 Set the current projection matrix. For internal use only.
			 */
			static void SetProjectionMatrix(const matrix4& mat) {
				projectionMatrix = mat;
			}
			
			/**
			 @retuurn a const-reference to the current global projection matrix
			 For internal use only.
			 */
			static const matrix4& GetCurrentProjectionMatrix() {
				return projectionMatrix;
			}
			
			/**
			 Set the current view matrix. For interal use only.
			 */
			static void SetViewMatrix(const matrix4& mat) {
				viewMatrix = mat;
			}
			
			/**
			 @return a const-reference to the current global view matrix. For internal use only
			 */
			static const matrix4& GetCurrentViewMatrix() {
				return viewMatrix;
			}
		};

	protected:
		std::string name;

		//trying to create a material that already exists will throw an exception
		Material(const std::string& name);

		matrix4 transformMatrix = matrix4(1);

		struct Settings {
			float wvpMatrix[16];
		} settings;

		
		bgfx::ProgramHandle program;
	};

	//for type conversions, do not use directly
	class MaterialInstanceBase : public SharedObject {
	public:
		virtual void Draw(const VertexBuffer& vertexBuffer, const IndexBuffer& indexBuffer, const matrix4& worldmatrix) = 0;
	};

	/**
	* Represents the settings of a material. Subclass to expose more properties.
	*/
	template<typename T>
	class MaterialInstance : public MaterialInstanceBase {
	public:
		virtual ~MaterialInstance() {}
		virtual void Draw(const VertexBuffer& vertexBuffer, const IndexBuffer& indexBuffer, const matrix4& worldmatrix) override{
			mat->SetTransformMatrix(worldmatrix);
			mat->Draw(vertexBuffer, indexBuffer);
		}
	protected:
		MaterialInstance(Ref<T> m) : mat(m) {}
		Ref<T> mat;
	};
}
