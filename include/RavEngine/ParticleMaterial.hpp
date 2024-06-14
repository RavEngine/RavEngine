#pragma once
#include <RGL/Types.hpp>
#include "mathtypes.hpp"
#include <string>
#include <variant>
#include "Ref.hpp"

namespace RavEngine {

	struct Texture;

	struct BilboardParticleEngineData {
		vector3 pos{ 0 };
		vector2 scale{ 0 };
		uint32_t animationFrame = 0;
	};


	struct ParticleUpdateUBO {
		float fpsScale;
	};

	struct ParticleBillboardUBO {
		vector2i spritesheetDim;
		vector2i numSprites;
	};

	struct ParticleQuadVert {
		vector2 pos;
	};

	struct ParticleUpdateMaterial {
		friend class RenderEngine;
	protected:
		ParticleUpdateMaterial(const std::string& initShaderName, const std::string& updateShaderName);
		
	public:
		const auto GetInitShader() const {
			return userInitPipeline;
		}

		const auto GetUpdateShader() const {
			return userUpdatePipeline;
		}

	private:
		RGLComputePipelinePtr userInitPipeline, userUpdatePipeline;
	};

	struct ParticleRenderMaterial {
		friend class RenderEngine;
	protected:
		ParticleRenderMaterial(const std::string& particleVS, const std::string& particleFS);

	public:
		/**
		* @return The number of bytes required for your material's per-particle user data. This data is placed immediately after the engine's required particle data for each particle
		*/
		virtual uint16_t ParticleUserDataSize() const {
			return 0;
		}
	private:
	
		RGLRenderPipelinePtr userRenderPipeline;
	};

	// Subclass this to make custom particle materials
	struct BillboardRenderParticleMaterial : public ParticleRenderMaterial {
	protected:
		BillboardRenderParticleMaterial(const std::string& particleVS, const std::string& particleFS) : ParticleRenderMaterial(particleVS, particleFS) {}

	public:
		Ref<Texture> spriteTex;
		struct spriteCount {
			uint16_t numSpritesWidth = 0;
			uint16_t numSpritesHeight = 0;
		} spriteDim;

	};

	// Subclass this to make custom particle materials
	struct MeshParticleRenderMaterial : public ParticleRenderMaterial {
	protected:
		MeshParticleRenderMaterial(const std::string& particleVS, const std::string& particleFS) : ParticleRenderMaterial(particleVS, particleFS) {}
	};

}