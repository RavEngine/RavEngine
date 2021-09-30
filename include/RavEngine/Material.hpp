#pragma once
#include "Ref.hpp"
#include "WeakRef.hpp"
#include "DataStructures.hpp"
#include <bgfx/bgfx.h>
#include "glm/gtc/type_ptr.hpp"
#include "Common3D.hpp"
#include "CTTI.hpp"
#include "Debug.hpp"
#include "Manager.hpp"

namespace RavEngine {

	/**
	Represents the interface to a shader. Subclass to create more types of material and expose more abilities.
	*/
	class Material : public AutoCTTI {
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
		public:
			
			/**
			 Helper to get a material by type. If one is not allocated, it will be created. Supports constructors via parameter pack
			 @param args arguments to pass to material constructor if needed
			 */
			template<typename T, typename ... A>
			static inline Ref<T> Get(A ... args){
                return GenericWeakCache<ctti_t,T,false>::Get(CTTI<T>(),args...);
			}
            
            /**
             Shrink memory usage by removing expired entries
             */
            template<typename T>
            static inline void Compact(){
                GenericWeakCache<ctti_t,T,false>::Compact();
            }
		};

	protected:
		std::string name;

		//trying to create a material that already exists will throw an exception
		Material(const std::string& name);
		
		bgfx::ProgramHandle program;

		friend class RenderEngine;
	private:
		static bgfx::ProgramHandle loadComputeProgram(const std::string_view& full_path);
        static bgfx::ShaderHandle loadShaderHandle(const std::string_view& full_path);
	};

	//for type conversions, do not use directly
	class MaterialInstanceBase {
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
