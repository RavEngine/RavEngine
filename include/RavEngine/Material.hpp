#pragma once
#include "SharedObject.hpp"
#include "RenderEngine.hpp"
#include <unordered_map>
#include <mutex>
#include <bgfx/bgfx.h>

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
		void Draw(const bgfx::VertexBufferHandle& vertexBuffer, const bgfx::IndexBufferHandle& indexBuffer);
		
		/**
		 Static singleton for managing materials
		 */
		class Manager {
			friend class Material;
		protected:
			//materials are keyed by their shader name
			typedef std::unordered_map<std::string, Ref<RavEngine::Material>> MaterialStore;
			static MaterialStore materials;
			static std::mutex mtx;
			
			static matrix4 projectionMatrix;
			static matrix4 viewMatrix;
			
			//for internal use only
			static void RegisterMaterial(Ref<RavEngine::Material>);
		public:
			/**
			 Gets a material with a given name, casted to a particular type.
			 @param name the name of the material to query
			 @note Undefined Behavior occurs if the template parameter does not match the returned material or any of its base classes
			 */
			static Ref<Material> GetMaterialByName(const std::string& name) {
				mtx.lock();
				Ref<Material> mat;
				mat.setNull();
				try{
					mat = materials.at(name);
				}
				catch(std::exception&){}
				mtx.unlock();
				return mat;
			}
			static bool HasMaterialByName(const std::string&);
			static void UnregisterMaterialByName(const std::string&);
			
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
		virtual void Draw(const bgfx::VertexBufferHandle& vertexBuffer, const bgfx::IndexBufferHandle& indexBuffer, const matrix4& worldmatrix) = 0;
	};

	/**
	* Represents the settings of a material. Subclass to expose more properties.
	*/
	template<typename T>
	class MaterialInstance : public MaterialInstanceBase {
	public:
		virtual ~MaterialInstance() {}
		virtual void Draw(const bgfx::VertexBufferHandle& vertexBuffer, const bgfx::IndexBufferHandle& indexBuffer, const matrix4& worldmatrix){
			mat->SetTransformMatrix(worldmatrix);
			mat->Draw(vertexBuffer, indexBuffer);
		}
	protected:
		MaterialInstance(Ref<T> m) : mat(m) {}
		Ref<T> mat;
	};
}
