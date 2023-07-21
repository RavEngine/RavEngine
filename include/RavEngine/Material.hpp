#pragma once
#include "Ref.hpp"
#include "WeakRef.hpp"
#include "DataStructures.hpp"
#include "Common3D.hpp"
#include "CTTI.hpp"
#include "Manager.hpp"
#include <RGL/Types.hpp>
#include <RGL/Pipeline.hpp>
#include <RGL/Span.hpp>
#include <span>

namespace RavEngine {
	struct Texture;

	struct MaterialConfig {
		RGL::RenderPipelineDescriptor::VertexConfig vertConfig;
		RGL::RenderPipelineDescriptor::ColorBlendConfig colorBlendConfig;
		bool depthTestEnabled = true;
		bool depthWriteEnabled = true;
		RGL::DepthCompareFunction depthCompareFunction = RGL::DepthCompareFunction::Less;

		std::vector<RGL::PipelineLayoutDescriptor::LayoutBindingDesc> bindings;
		uint32_t pushConstantSize = 0;
	};

	/**
	Represents the interface to a shader. Subclass to create more types of material and expose more abilities.
	*/
	class Material : public AutoCTTI {
	public:
		friend class MaterialInstance;
		/**
		Create the default material. Override this constructor in subclasses, and from that, invoke the protected constructor.
		*/

		virtual ~Material();
		
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

			template<typename T, typename ... A>
			static inline Ref<T> GetWithKey(RavEngine::CacheBase::unique_key_t key, A&& ... args) {
				return GenericWeakReadThroughCache<ctti_t, T, false>::GetWithKey(CTTI<T>(), key, args...);
			}
            
            /**
             Shrink memory usage by removing expired entries
             */
            template<typename T>
            static inline void Compact(){
                GenericWeakReadThroughCache<ctti_t,T,false>::Compact();
            }
		};

		auto GetMainRenderPipeline() {
			return renderPipeline;
		}

		auto GetShadowRenderPipeline() {
			return shadowRenderPipeline;
		}

	protected:
		RGLRenderPipelinePtr renderPipeline, shadowRenderPipeline;
		RGLPipelineLayoutPtr pipelineLayout;

		Material(const std::string_view name, const MaterialConfig& config);
		Material(const std::string_view vsh_name, const std::string_view fsh_name, const MaterialConfig& config);
		
		friend class RenderEngine;
	};

	/**
	* Represents the settings of a material. Subclass to expose more properties.
	*/	
	struct MaterialInstance {
		constexpr static uint8_t maxBindingSlots = 8;
	protected:
		std::array<RGLBufferPtr, maxBindingSlots> bufferBindings;
		std::array<Ref<Texture>, maxBindingSlots> textureBindings;
	public:
        bool doubleSided = false;
		
		/**
		@return a byte view to the push constant data for the material. The data is appended after the viewProj mat4. 
		Returning a span of size 0, or with a nullptr pointer means that the material has no additional push constants.
		*/
		virtual const RGL::untyped_span GetPushConstantData() const{
			return { nullptr, 0 };
		}

		const auto& GetBufferBindings() const {
			return bufferBindings;
		}

		const auto& GetTextureBindings() const {
			return textureBindings;
		}

		virtual ~MaterialInstance() {}
		auto GetMat() {
			return mat;
		}

	protected:
		MaterialInstance(Ref<Material> m) : mat(m) {}
		Ref<Material> mat;
	};
}
