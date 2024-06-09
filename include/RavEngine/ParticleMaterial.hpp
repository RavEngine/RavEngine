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


	struct ParticleMaterial {
		friend class RenderEngine;
	protected:
		ParticleMaterial(const std::string& initShaderName, const std::string& updateShaderName, const std::string& particleVS, const std::string& particleFS);

	public:
		/**
		* @return The number of bytes required for your material's per-particle user data. This data is placed immediately after the engine's required particle data for each particle
		*/
		virtual uint16_t ParticleUserDataSize() const {
			return 0;
		}

		const auto GetInitShader() const {
			return userInitPipeline;
		}

		const auto GetUpdateShader() const {
			return userUpdatePipeline;
		}

	private:
		RGLComputePipelinePtr userInitPipeline, userUpdatePipeline;
		RGLRenderPipelinePtr userRenderPipeline;
	};

	// Subclass this to make custom particle materials
	struct BillboardParticleMaterial : public ParticleMaterial {
	protected:
		BillboardParticleMaterial(const std::string& initShaderName, const std::string& updateShaderName, const std::string& particleVS, const std::string& particleFS) : ParticleMaterial(initShaderName, updateShaderName, particleVS, particleFS) {}

	public:
		Ref<Texture> spriteTex;
		struct spriteCount {
			uint16_t numSpritesWidth = 0;
			uint16_t numSpritesHeight;
		} spriteDim;

	};
	// Subclass this to make custom particle materials
	struct MeshParticleMaterial : public ParticleMaterial {
	protected:
		MeshParticleMaterial(const std::string& initShaderName, const std::string& updateShaderName, const std::string& particleVS, const std::string& particleFS) : ParticleMaterial(initShaderName, updateShaderName, particleVS, particleFS) {}
	};
}