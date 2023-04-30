#pragma once
#include "Ref.hpp"
#include "WeakRef.hpp"
#include "DataStructures.hpp"
#include "glm/gtc/type_ptr.hpp"
#include "Common3D.hpp"
#include "CTTI.hpp"
#include "Debug.hpp"
#include "Manager.hpp"
#include <RGL/Types.hpp>
#include <RGL/Pipeline.hpp>
#include <span>

namespace RavEngine {
	struct Texture;

	struct MaterialConfig {
		RGL::RenderPipelineDescriptor::VertexConfig vertConfig;
		RGL::RenderPipelineDescriptor::ColorBlendConfig colorBlendConfig;
		bool depthTestEnabled = true;
		bool depthWriteEnabled = true;
		RGL::DepthCompareFunction depthCompareFunction = RGL::DepthCompareFunction::Less;
	};

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

		virtual ~Material();

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
		RGLShaderLibraryPtr vertShader, fragShader;
		RGLRenderPipelinePtr renderPipeline;
		RGLPipelineLayoutPtr pipelineLayout;

		//trying to create a material that already exists will throw an exception
		Material(const std::string& name, const MaterialConfig& config);
		
		friend class RenderEngine;
		friend class MaterialInstanceBase;
	};

	//for type conversions, do not use directly
	class MaterialInstanceBase {
	private:
	protected:
		constexpr static uint8_t maxBindingSlots = 8;
		std::array<RGLBufferPtr, maxBindingSlots> bufferBindings;
		std::array<Ref<Texture>, maxBindingSlots> textureBindings;
	public:
        bool doubleSided = false;
		friend class RenderEngine;
		virtual std::span<std::byte> PushConstantData() {
			return std::span<std::byte, 0>{};	// a span of size 0 means that the material has no push constants
		}
	};

	/**
	* Represents the settings of a material. Subclass to expose more properties.
	*/
	template<typename T>
	class MaterialInstance : public MaterialInstanceBase {
	public:
		virtual ~MaterialInstance() {}
		friend class RenderEngine;
		auto GetMat() {
			return mat;
		}
	protected:
		MaterialInstance(Ref<T> m) : mat(m) {}
		Ref<T> mat;
	};
}
